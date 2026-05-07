/**
 * \file SAMAV2ISO7816Commands.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2ISO7816Commands commands.
 */


#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav2iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunitconfiguration.hpp>
#include <logicalaccess/plugins/cards/samav/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav/samkeyentry.hpp>
#include <logicalaccess/plugins/cards/samav/samkucentry.hpp>
#include <openssl/rand.h>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>

#include <cstring>

namespace logicalaccess
{
SAMAV2ISO7816Commands::SAMAV2ISO7816Commands()
    : SAMISO7816Commands<KeyEntryAV2Information, SETAV2>(CMD_SAMAV2ISO7816)
    , d_cmdCtr(0)
{
    d_lastMacIV.resize(16);
}

SAMAV2ISO7816Commands::SAMAV2ISO7816Commands(std::string ct)
    : SAMISO7816Commands<KeyEntryAV2Information, SETAV2>(ct)
    , d_cmdCtr(0)
{
    d_lastMacIV.resize(16);
}

SAMAV2ISO7816Commands::~SAMAV2ISO7816Commands() {}

void SAMAV2ISO7816Commands::generateSessionKey(ByteVector rnda, ByteVector rndb)
{
    constexpr size_t BLOCK_SIZE   = 16;
    constexpr size_t AES_128_SIZE = 16;
    constexpr size_t AES_192_SIZE = 24;
    constexpr size_t AES_256_SIZE = 32;

    ByteVector SV1a(BLOCK_SIZE), SV1b(BLOCK_SIZE), SV2a(BLOCK_SIZE), SV2b(BLOCK_SIZE),
        emptyIV(BLOCK_SIZE, 0x00);

    auto copy_block = [](ByteVector &dst, const ByteVector &src1, size_t off1,
                         const ByteVector &src2, size_t off2, const ByteVector &src3,
                         size_t off3)
    {
        std::copy(src1.begin() + off1, src1.begin() + off1 + 5, dst.begin());
        std::copy(src2.begin() + off2, src2.begin() + off2 + 5, dst.begin() + 5);
        std::copy(src3.begin() + off3, src3.begin() + off3 + 5, dst.begin() + 10);
    };

    copy_block(SV1a, rnda, 11, rndb, 11, rnda, 4);
    for (size_t x = 4; x <= 9; ++x)
        SV1a[x + 6] ^= rndb[x];

    copy_block(SV1b, rnda, 10, rndb, 10, rnda, 5);
    for (size_t x = 5; x <= 10; ++x)
        SV1b[x + 5] ^= rndb[x];

    copy_block(SV2a, rnda, 7, rndb, 7, rnda, 0);
    for (size_t x = 0; x <= 5; ++x)
        SV2a[x + 10] ^= rndb[x];

    copy_block(SV2b, rnda, 6, rndb, 6, rnda, 1);
    for (size_t x = 1; x <= 6; ++x)
        SV2b[x + 9] ^= rndb[x];

    size_t sessionKeySize = d_macSessionKey.size();
    if (sessionKeySize != AES_128_SIZE && sessionKeySize != AES_192_SIZE &&
        sessionKeySize != AES_256_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "generateSessionKey: Invalid session key size");

    static constexpr std::array<std::array<unsigned char, 4>, 3> SV_TAGS = {{
        {0x81, 0x00, 0x82, 0x00}, // AES 128
        {0x83, 0x84, 0x85, 0x86}, // AES 192
        {0x87, 0x88, 0x89, 0x8a}  // AES 256
    }};

    const size_t keyIndex = (sessionKeySize == AES_256_SIZE)   ? 2
                            : (sessionKeySize == AES_192_SIZE) ? 1
                                                               : 0;
    const auto &sv_tags   = SV_TAGS[keyIndex];

    SV1a[15] = sv_tags[0];
    SV1b[15] = sv_tags[1];
    SV2a[15] = sv_tags[2];
    SV2b[15] = sv_tags[3];

    auto symkey = openssl::AESSymmetricKey::createFromData(d_macSessionKey);
    auto iv     = openssl::AESInitializationVector::createFromData(emptyIV);
    openssl::AESCipher cipher;
    ByteVector Kea, Keb, Kma, Kmb;

    cipher.cipher(SV1a, Kea, symkey, iv, false);
    cipher.cipher(SV1b, Keb, symkey, iv, false);
    cipher.cipher(SV2a, Kma, symkey, iv, false);
    cipher.cipher(SV2b, Kmb, symkey, iv, false);

    d_sessionKey    = Kea;
    d_macSessionKey = Kma;
    if (sessionKeySize == 32) /* AES 256 */
    {
        d_sessionKey.insert(d_sessionKey.end(), Keb.begin(), Keb.end());
        d_macSessionKey.insert(d_macSessionKey.end(), Kmb.begin(), Kmb.end());
    }
    else if (sessionKeySize == 24) /* AES 192 */
    {
        for (unsigned char x = 0; x < 8; ++x)
        {
            d_sessionKey[x + 8] ^= Keb[x];
            d_macSessionKey[x + 8] ^= Kmb[x];
        }
        d_sessionKey.insert(d_sessionKey.end(), Keb.end() - 8, Keb.end());
        d_macSessionKey.insert(d_macSessionKey.end(), Kmb.end() - 8, Kmb.end());
    }
    OPENSSL_cleanse(SV1a.data(), SV1a.size());
    OPENSSL_cleanse(SV1b.data(), SV1b.size());
    OPENSSL_cleanse(SV2a.data(), SV2a.size());
    OPENSSL_cleanse(SV2b.data(), SV2b.size());
}

void SAMAV2ISO7816Commands::generateOfflineSessionKey(std::shared_ptr<DESFireKey> key,
                                                      unsigned short changecnt)
{
    if (key->getKeyType() != DF_KEY_AES)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "generateOfflineSessionKey Only AES Key allowed.");

    constexpr size_t BLOCK_SIZE   = 16;
    constexpr size_t AES_128_SIZE = 16;
    constexpr size_t AES_192_SIZE = 24;
    constexpr size_t AES_256_SIZE = 32;
    const ByteVector keydata      = key->getData();
    const size_t keysize          = keydata.size();

    if (keysize != AES_128_SIZE && keysize != AES_192_SIZE && keysize != AES_256_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "generateOfflineSessionKey Invalid AES key size.");

    d_sessionKey.clear();
    d_macSessionKey.clear();

    ByteVector SV1a, SV1b, SV2a, SV2b;
    ByteVector emptyIV(BLOCK_SIZE, 0x00);
    static constexpr std::array<std::array<unsigned char, 4>, 3> SV_TAGS = {{
        {0x71, 0x00, 0x72, 0x00}, // AES 128
        {0x73, 0x74, 0x75, 0x76}, // AES 192
        {0x77, 0x78, 0x79, 0x7a}  // AES 256
    }};

    const size_t idx = (keysize == AES_192_SIZE) ? 1 : (keysize == AES_256_SIZE) ? 2 : 0;
    const auto &t    = SV_TAGS[idx];

    SV1a = SV1b = SV2a = SV2b = ByteVector(BLOCK_SIZE);
    SV1a[0] = SV1b[0] = SV2a[0] = SV2b[0] =
        static_cast<unsigned char>((changecnt >> 8) & 0xff);
    SV1a[1] = SV1b[1] = SV2a[1] = SV2b[1] = static_cast<unsigned char>(changecnt & 0xff);
    std::fill(SV1a.begin() + 2, SV1a.end(), t[0]);
    std::fill(SV1b.begin() + 2, SV1b.end(), t[1]);
    std::fill(SV2a.begin() + 2, SV2a.end(), t[2]);
    std::fill(SV2b.begin() + 2, SV2b.end(), t[3]);

    auto symkey = openssl::AESSymmetricKey::createFromData(keydata);
    auto iv     = openssl::AESInitializationVector::createFromData(emptyIV);
    openssl::AESCipher cipher;
    ByteVector Kea, Keb, Kma, Kmb;

    cipher.cipher(SV1a, Kea, symkey, iv, false);
    cipher.cipher(SV1b, Keb, symkey, iv, false);
    cipher.cipher(SV2a, Kma, symkey, iv, false);
    cipher.cipher(SV2b, Kmb, symkey, iv, false);

    d_sessionKey    = Kea;
    d_macSessionKey = Kma;
    if (keysize == AES_256_SIZE) /* AES 256 */
    {
        d_sessionKey.insert(d_sessionKey.end(), Keb.begin(), Keb.end());
        d_macSessionKey.insert(d_macSessionKey.end(), Kmb.begin(), Kmb.end());
    }
    else if (keysize == AES_192_SIZE) /* AES 192 */
    {
        for (unsigned char x = 0; x < 8; ++x)
        {
            d_sessionKey[x + 8] ^= Keb[x];
            d_macSessionKey[x + 8] ^= Kmb[x];
        }
        d_sessionKey.insert(d_sessionKey.end(), Keb.end() - 8, Keb.end());
        d_macSessionKey.insert(d_macSessionKey.end(), Kmb.end() - 8, Kmb.end());
    }
    OPENSSL_cleanse(SV1a.data(), SV1a.size());
    OPENSSL_cleanse(SV1b.data(), SV1b.size());
    OPENSSL_cleanse(SV2a.data(), SV2a.size());
    OPENSSL_cleanse(SV2b.data(), SV2b.size());
}

void SAMAV2ISO7816Commands::authenticateHost(std::shared_ptr<DESFireKey> key,
                                             unsigned char keyno)
{
    authenticateHost(key, keyno, sam::HostMode::FullProtect); // Host Mode: Full Protection
}

void SAMAV2ISO7816Commands::authenticateHost(std::shared_ptr<DESFireKey> key,
                                             unsigned char keyno, sam::HostMode hostmode)
{

    if (key->getKeyType() != DF_KEY_AES)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost Only AES Key allowed.");

    constexpr unsigned char AES_BLOCK_SIZE = 16;
    constexpr unsigned char MAC_SIZE       = 8;
    constexpr unsigned char RND_SIZE       = 12;
    constexpr unsigned char RND_FULL_SIZE  = 16;
    auto mode                              = sam::toByte(hostmode);
    ByteVector emptyIV(AES_BLOCK_SIZE, 0x00);
    ByteVector data_p1 = {keyno, key->getKeyVersion(), mode};
    auto adapter       = getISO7816ReaderCardAdapter();

    /* emptyIV and Clear Key */
    d_lastMacIV     = emptyIV;
    d_LastSessionIV = emptyIV;
    d_sessionKey.clear();
    d_macSessionKey.clear();

    auto result = adapter->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, 0x03, data_p1, 0x00);
    if (result.getData().size() != 12 || result.getSW1() != 0x90 ||
        result.getSW2() != 0xAF)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost P1 Failed.");

    const ByteVector keycipher = key->getData();
    d_macSessionKey            = keycipher;
    auto cipher                = std::make_shared<openssl::AESCipher>();

    /* Create rnd2 for p3 - CMAC: rnd2 | Host Mode | ZeroPad */
    ByteVector rnd2 = result.getData();
    rnd2.push_back(mode);
    rnd2.resize(AES_BLOCK_SIZE, 0x00); // ZeroPad

    ByteVector macHost = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, rnd2,
                                                   d_lastMacIV, AES_BLOCK_SIZE);
    truncateMacBuffer(macHost);

    ByteVector rnd1(RND_SIZE);
    if (RAND_bytes(&rnd1[0], static_cast<int>(rnd1.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }

    ByteVector data_p2;
    data_p2.insert(data_p2.end(), macHost.begin(), macHost.begin() + MAC_SIZE);
    data_p2.insert(data_p2.end(), rnd1.begin(), rnd1.end());
    result = adapter->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, 0x14, data_p2, 0x00);
    if (result.getData().size() != 24 || result.getSW1() != 0x90 ||
        result.getSW2() != 0xAF)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost P2 Failed.");

    /* Check CMAC - Create rnd1 for p3 - CMAC: rnd1 | P1 | other data */
    rnd1.insert(rnd1.end(), rnd2.begin() + RND_SIZE, rnd2.end()); // p2 data without rnd2
    macHost = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, rnd1, d_lastMacIV,
                                        AES_BLOCK_SIZE);
    truncateMacBuffer(macHost);
    for (unsigned char x = 0; x < MAC_SIZE; ++x)
    {
        if (macHost[x] != result.getData()[x])
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "authenticateHost P2 CMAC from SAM is Wrong.");
    }

    /* Create kxe - d_authKey */
    generateAuthEncKey(keycipher, rnd1, rnd2);
    // create rndA
    ByteVector rndA(AES_BLOCK_SIZE);
    if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    // decipher rndB
    auto symkey = openssl::AESSymmetricKey::createFromData(d_authKey);
    auto iv     = openssl::AESInitializationVector::createFromData(d_lastMacIV);

    ByteVector encRndB(result.getData().begin() + MAC_SIZE, result.getData().end());
    ByteVector dencRndB;
    cipher->decipher(encRndB, dencRndB, symkey, iv, false);

    // create rndB'
    ByteVector rndB1;
    rndB1.insert(rndB1.begin(), dencRndB.begin() + 2, dencRndB.begin() + dencRndB.size());
    rndB1.push_back(dencRndB[0]);
    rndB1.push_back(dencRndB[1]);

    ByteVector dataHost;
    dataHost.insert(dataHost.end(), rndA.begin(), rndA.end());   // RndA
    dataHost.insert(dataHost.end(), rndB1.begin(), rndB1.end()); // RndB'

    iv = openssl::AESInitializationVector::createFromData(d_lastMacIV);
    ByteVector encHost;

    cipher->cipher(dataHost, encHost, symkey, iv, false);
    result = adapter->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, 0x20, encHost, 0x00);
    if (result.getData().size() != AES_BLOCK_SIZE || result.getSW1() != 0x90 ||
        result.getSW2() != 0x00)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost P3 Failed.");

    ByteVector SAMrndA;
    iv = openssl::AESInitializationVector::createFromData(d_lastMacIV);
    cipher->decipher(result.getData(), SAMrndA, symkey, iv, false);
    SAMrndA.insert(SAMrndA.begin(), SAMrndA.end() - 2, SAMrndA.end());

    if (!equal(SAMrndA.begin(), SAMrndA.begin() + AES_BLOCK_SIZE, rndA.begin()))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost P3 RndA from SAM is invalid.");
    generateSessionKey(rndA, dencRndB);
    d_cmdCtr = 0;

    OPENSSL_cleanse(rnd1.data(), rnd1.size());
    OPENSSL_cleanse(rnd2.data(), rnd2.size());
    OPENSSL_cleanse(rndA.data(), rndA.size());
    OPENSSL_cleanse(dencRndB.data(), dencRndB.size());
    OPENSSL_cleanse(rndB1.data(), rndB1.size());
    OPENSSL_cleanse(dataHost.data(), dataHost.size());
    OPENSSL_cleanse(encRndB.data(), encRndB.size());
    OPENSSL_cleanse(SAMrndA.data(), SAMrndA.size());
    OPENSSL_cleanse(macHost.data(), macHost.size());
}

std::pair<ByteVector, ByteVector>
SAMAV2ISO7816Commands::prepareSecureChainedPayload(const ByteVector &payload, bool le)
{
    if (payload.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "createfullProtectionCmd Invalid command size.");

    size_t lcvalue = payload.size();
    if (le)
        lcvalue--;
    ByteVector protectedCmd = payload;
    ByteVector encData;

    ByteVector data(payload.begin() + AV2_HEADER_LENGTH, payload.begin() + lcvalue);
    protectedCmd.erase(protectedCmd.begin() + AV2_HEADER_LENGTH,
                       protectedCmd.begin() + lcvalue);

    if (data.size() % 16 != 0)
    {
        data.push_back(0x80);
        if (data.size() % 16 != 0)
            data.resize((unsigned char)(data.size() / 16 + 1) * 16, 0x00);
    }

    /* generate IV because first encrypt */
    d_LastSessionIV    = generateEncIV(true);
    auto cipher        = std::make_shared<openssl::AESCipher>();
    auto symkeySession = openssl::AESSymmetricKey::createFromData(d_sessionKey);
    auto ivSession = openssl::AESInitializationVector::createFromData(d_LastSessionIV);
    cipher->cipher(data, encData, symkeySession, ivSession, false);
    protectedCmd.insert(protectedCmd.begin() + AV2_HEADER_LENGTH, encData.begin(),
                        encData.end());
    if (data.size() < 0xEF)
        protectedCmd[AV2_LC_POS] = static_cast<unsigned char>(encData.size() + 8);
    else
        protectedCmd[AV2_LC_POS] = static_cast<unsigned char>(0x00);
    ByteVector finalCmd = protectedCmd;

    /* Set counter*/
    ByteVector cmdCtrVector;
    BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
    std::reverse(cmdCtrVector.begin(), cmdCtrVector.end());
    protectedCmd.insert(protectedCmd.begin() + 2, cmdCtrVector.begin(),
                        cmdCtrVector.end());

    /* Creater our cipher buffer and keep last block */
    auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_macSessionKey);
    auto ivMac     = openssl::AESInitializationVector::createFromData(d_lastMacIV);
    const size_t blockReady = (protectedCmd.size() / 16) * 16;
    if (blockReady >= 16)
    {
        ByteVector encMac(protectedCmd.begin(), protectedCmd.begin() + blockReady);
        ByteVector tmp(encMac.size());
        protectedCmd.erase(protectedCmd.begin(), protectedCmd.begin() + blockReady);
        cipher->cipher(encMac, tmp, symkeyMac, ivMac, false);
        EXCEPTION_ASSERT_WITH_LOG(tmp.size() >= 16, LibLogicalAccessException,
                                  "Cipher output error.");
        d_lastMacIV.assign(tmp.end() - 16, tmp.end());
    }
    ByteVector macFull =
        openssl::CMACCrypto::cmac(d_macSessionKey, cipher, protectedCmd, d_lastMacIV, 16);
    truncateMacBuffer(macFull);
    return {encData, macFull};
}

std::vector<ByteVector> SAMAV2ISO7816Commands::createSecureChainedApduFrames(
    const ByteVector &cmd, const ByteVector &encData, const ByteVector &mac,
    unsigned char PxIndex, bool le)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid APDU : too short");

    constexpr size_t MAX_CHUNK = sam::MAX_APDU_DATA_SIZE;
    std::vector<ByteVector> frames;
    size_t offset = 0;
    bool isHead   = true;

    while (offset < encData.size())
    {
        const size_t chunkSize = (std::min)(isHead ? MAX_CHUNK - 8 : MAX_CHUNK, encData.size() - offset);
        const bool isLastChunk = (offset + chunkSize >= encData.size());

        ByteVector frame;
        frame.push_back(cmd[0]);
        frame.push_back(cmd[1]);
        frame.push_back(cmd[2]);
        // if (!isLastChunk) //Keep and correct later. Only first frame can have value != 0x00
        //     frame[2] = 0;
        frame.push_back(cmd[3]);
        unsigned char p = static_cast<unsigned char>(isHead ? 0x00 : 0xAF);
        frame[PxIndex] = p;
        frame.push_back(0x00);
        frame.insert(frame.end(), encData.begin() + offset,
                     encData.begin() + offset + chunkSize);
        if (isHead)
            frame.insert(frame.end(), mac.begin(), mac.begin() + 8);
        frame[AV2_LC_POS] = static_cast<unsigned char>(frame.size() - AV2_HEADER_LENGTH);
        isHead = false;
        if (le)
            frame.push_back(cmd.back());
        frames.push_back(std::move(frame));
        offset += chunkSize;
    }
    return frames;
}

ByteVector SAMAV2ISO7816Commands::createfullProtectionCmd(const ByteVector &cmd)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "createfullProtectionCmd Invalid command size.");

    bool lc = false;
    unsigned char lcvalue = 0;
    getLcLe(cmd, lc, lcvalue);
    auto cipher             = std::make_shared<openssl::AESCipher>();
    ByteVector protectedCmd = cmd;
    ByteVector encData;

    if (!lc)
    {
        protectedCmd.insert(protectedCmd.begin() + AV2_LC_POS, 0x08);
    }
    else
    {
        ByteVector data(cmd.begin() + AV2_HEADER_LENGTH,
                        cmd.begin() + AV2_HEADER_LENGTH + lcvalue);
        protectedCmd.erase(protectedCmd.begin() + AV2_HEADER_LENGTH,
                           protectedCmd.begin() + AV2_HEADER_LENGTH + lcvalue);

        if (data.size() % 16 != 0)
        {
            data.push_back(0x80);
            if (data.size() % 16 != 0)
                data.resize((unsigned char)(data.size() / 16 + 1) * 16, 0x00);
        }
        /* generate IV because first encrypt */
        d_LastSessionIV = generateEncIV(true);
        auto symkeySession = openssl::AESSymmetricKey::createFromData(d_sessionKey);
        auto ivSession = openssl::AESInitializationVector::createFromData(d_LastSessionIV);
        cipher->cipher(data, encData, symkeySession, ivSession, false);
        protectedCmd.insert(protectedCmd.begin() + AV2_HEADER_LENGTH, encData.begin(),
                            encData.end());
        protectedCmd[AV2_LC_POS] = static_cast<unsigned char>(encData.size() + 8);
    }
    ByteVector finalCmd = protectedCmd;

    /* Set counter*/
    ByteVector cmdCtrVector;
    BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
    std::reverse(cmdCtrVector.begin(), cmdCtrVector.end());
    protectedCmd.insert(protectedCmd.begin() + 2, cmdCtrVector.begin(),
                        cmdCtrVector.end());

    /* Creater our cipher buffer and keep last block */
    auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_macSessionKey);
    auto ivMac     = openssl::AESInitializationVector::createFromData(d_lastMacIV);
    const size_t blockReady = (protectedCmd.size() / 16) * 16;
    if (blockReady >= 16)
    {
        ByteVector encMac(protectedCmd.begin(), protectedCmd.begin() + blockReady);
        ByteVector tmp(encMac.size());
        protectedCmd.erase(protectedCmd.begin(), protectedCmd.begin() + blockReady);
        cipher->cipher(encMac, tmp, symkeyMac, ivMac, false);
        EXCEPTION_ASSERT_WITH_LOG(tmp.size() >= 16, LibLogicalAccessException,
                                  "Cipher output too small.");
        d_lastMacIV.assign(tmp.end() - 16, tmp.end());
    }
    ByteVector encProtectedCmd = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, protectedCmd, d_lastMacIV, 16);
    truncateMacBuffer(encProtectedCmd);
    finalCmd.insert(finalCmd.begin() + AV2_HEADER_LENGTH + encData.size(), encProtectedCmd.begin(), encProtectedCmd.begin() + 8);
    return finalCmd;
}

void SAMAV2ISO7816Commands::getLcLe(const ByteVector &cmd, bool &lc, unsigned char &lcvalue)
{
    const size_t cmdSize = cmd.size();

    if (cmdSize < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getLcLe cmd param is too small.");

    lc      = false;
    lcvalue = 0;

    // [CLA INS P1 P2] or [CLA INS P1 P2 LE]
    if (cmdSize <= AV2_HEADER_LENGTH)
        return;
    const unsigned char lcByte  = cmd[AV2_LC_POS];
    // [CLA INS P1 P2 LC DATA] or [CLA INS P1 P2 LC DATA LE]
    if (cmdSize == static_cast<size_t>(lcByte) + AV2_HEADER_LENGTH ||
        cmdSize == static_cast<size_t>(lcByte) + AV2_HEADER_LENGTH_WITH_LE)
    {
        lc      = true;
        lcvalue = lcByte;
        return;
    }
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "getLcLe invalid command structure (size mismatch).");
}

ByteVector SAMAV2ISO7816Commands::verifyAndDecryptResponse(const ByteVector &response)
{
    constexpr size_t MAC_SIZE    = 8;
    constexpr size_t STATUS_SIZE = 2;

    /* begin check mac */
    if (response.size() < MAC_SIZE + STATUS_SIZE)
        return response;

    auto cipher = std::make_shared<openssl::AESCipher>();

    ByteVector myMac, cmdCtrVector, myEncMac, data;
    ByteVector mac(response.end() - MAC_SIZE - STATUS_SIZE, response.end() - STATUS_SIZE);

    myMac.push_back(response[response.size() - 2]);
    myMac.push_back(response[response.size() - 1]);

    /* Set counter*/
    BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
    std::reverse(cmdCtrVector.begin(), cmdCtrVector.end());
    myMac.insert(myMac.end(), cmdCtrVector.begin(), cmdCtrVector.end());

    if (response.size() != MAC_SIZE + STATUS_SIZE)
    {
        /* We have data Creater our cipher buffer and keep last block */
        auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_macSessionKey);
        auto ivMac     = openssl::AESInitializationVector::createFromData(d_lastMacIV);
        myMac.insert(myMac.end(), response.begin(),
                     response.end() - MAC_SIZE - STATUS_SIZE);
        const size_t blockReady = (myMac.size() / 16) * 16;
        ByteVector lastBlock(myMac.begin() + blockReady, myMac.end());
        myMac.erase(myMac.begin() + blockReady, myMac.end());
        cipher->cipher(myMac, myEncMac, symkeyMac, ivMac, false);
        d_lastMacIV.assign(myEncMac.end() - 16, myEncMac.end());
        myMac = lastBlock;
    }

    myEncMac = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, myMac, d_lastMacIV, 16);
    truncateMacBuffer(myEncMac);

    if (!equal(myEncMac.begin(), myEncMac.begin() + MAC_SIZE, mac.begin()))
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "verifyAndDecryptResponse wasnt able to verify the answer of the sam");

    if (response.size() > MAC_SIZE + STATUS_SIZE)
    {
        /* begin decrypt */

        /* generate IV because first decrypt */
        auto symkeySession = openssl::AESSymmetricKey::createFromData(d_sessionKey);
        d_LastSessionIV    = generateEncIV(false);
        auto ivSession =
            openssl::AESInitializationVector::createFromData(d_LastSessionIV);
        ByteVector encData(response.begin(), response.end() - MAC_SIZE - STATUS_SIZE);
        cipher->decipher(encData, data, symkeySession, ivSession, false);

        int i = static_cast<int>(data.size()) - 1;
        while (i >= 0 && data[i] != 0x80 && data[i] == 0x00)
            --i;
        if (i >= 0)
            data.resize(i);
    }
    data.push_back(response[response.size() - 2]);
    data.push_back(response[response.size() - 1]);
    return data;
}

// TODO make more tests for this function after multi frames is stable
ByteVector SAMAV2ISO7816Commands::createMacProtectionCmd(const ByteVector &cmd)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "createMacProtectionCmd Invalid command size.");
    constexpr unsigned char MAC_SIZE = 8;
    bool lc = false;
    unsigned char lcvalue = 0;
    getLcLe(cmd, lc, lcvalue);
    auto cipher = std::make_shared<openssl::AESCipher>();
    ByteVector protectedCmd = cmd;

    if (!lc)
        protectedCmd.insert(protectedCmd.begin() + AV2_LC_POS, MAC_SIZE);
    else
        protectedCmd[AV2_LC_POS] = static_cast<unsigned char>(lcvalue + MAC_SIZE);
    ByteVector finalCmd = protectedCmd;

    /* Set counter */
    ByteVector cmdCtrVector;
    BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
    std::reverse(cmdCtrVector.begin(), cmdCtrVector.end());

    protectedCmd.insert(protectedCmd.begin() + 2, cmdCtrVector.begin(),
                        cmdCtrVector.end());

    /* Create our cipher buffer and keep last block */
    auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_macSessionKey);
    auto ivMac     = openssl::AESInitializationVector::createFromData(d_lastMacIV);

    const size_t blockReady = (protectedCmd.size() / 16) * 16;
    if (blockReady >= 16)
    {
        ByteVector encMac(protectedCmd.begin(), protectedCmd.begin() + blockReady);
        ByteVector tmp(encMac.size());
        protectedCmd.erase(protectedCmd.begin(), protectedCmd.begin() + blockReady);
        cipher->cipher(encMac, tmp, symkeyMac, ivMac, false);
        EXCEPTION_ASSERT_WITH_LOG(tmp.size() >= 16, LibLogicalAccessException,
                                  "Cipher output too small.");
        d_lastMacIV.assign(tmp.end() - 16, tmp.end());
    }

    ByteVector encProtectedCmd = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, protectedCmd, d_lastMacIV, 16);
    truncateMacBuffer(encProtectedCmd);
    const size_t dataSize = lc ? lcvalue : 0;
    finalCmd.insert(finalCmd.begin() + AV2_HEADER_LENGTH + dataSize,
                    encProtectedCmd.begin(), encProtectedCmd.begin() + MAC_SIZE);
    return finalCmd;
}

// TODO make more tests for this function after multi frames is stable
ByteVector SAMAV2ISO7816Commands::verifyAndDecryptMacResponse(const ByteVector &response)
{
    constexpr size_t MAC_SIZE    = 8;
    constexpr size_t STATUS_SIZE = 2;

    if (response.size() < MAC_SIZE + STATUS_SIZE)
        return response;

    auto cipher = std::make_shared<openssl::AESCipher>();

    ByteVector mac(response.end() - MAC_SIZE - STATUS_SIZE, response.end() - STATUS_SIZE);
    ByteVector myMac, cmdCtrVector, myEncMac;

    myMac.push_back(response[response.size() - 2]);
    myMac.push_back(response[response.size() - 1]);

    BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
    std::reverse(cmdCtrVector.begin(), cmdCtrVector.end());
    myMac.insert(myMac.end(), cmdCtrVector.begin(), cmdCtrVector.end());

    if (response.size() > MAC_SIZE + STATUS_SIZE)
    {
        myMac.insert(myMac.end(), response.begin(),
                     response.end() - MAC_SIZE - STATUS_SIZE);
        const size_t blockReady = (myMac.size() / 16) * 16;
        ByteVector lastBlock(myMac.begin() + blockReady, myMac.end());
        myMac.erase(myMac.begin() + blockReady, myMac.end());
        auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_macSessionKey);
        auto ivMac     = openssl::AESInitializationVector::createFromData(d_lastMacIV);
        cipher->cipher(myMac, myEncMac, symkeyMac, ivMac, false);
        d_lastMacIV.assign(myEncMac.end() - 16, myEncMac.end());
        myMac = lastBlock;
    }
    myEncMac = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, myMac, d_lastMacIV, 16);
    truncateMacBuffer(myEncMac);
    if (!equal(myEncMac.begin(), myEncMac.begin() + MAC_SIZE, mac.begin()))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "verifyAndDecryptMacResponse: MAC verification failed");
    ByteVector data(response.begin(), response.end() - MAC_SIZE - STATUS_SIZE);
    data.push_back(response[response.size() - 2]);
    data.push_back(response[response.size() - 1]);
    return data;
}

ByteVector SAMAV2ISO7816Commands::generateEncIV(bool encrypt) const
{
    constexpr size_t COUNTER_SIZE        = 4;
    constexpr size_t PREFIX_SIZE         = 4;
    constexpr size_t REPEAT_COUNT        = 3;
    constexpr unsigned char FILL_ENCRYPT = 0x01;
    constexpr unsigned char FILL_DECRYPT = 0x02;

    ByteVector myIV(PREFIX_SIZE + COUNTER_SIZE * REPEAT_COUNT);

    const unsigned char fill = encrypt ? FILL_ENCRYPT : FILL_DECRYPT;
    std::fill(myIV.begin(), myIV.begin() + PREFIX_SIZE, fill);

    ByteVector cmdCtrVector(COUNTER_SIZE);
    BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
    std::reverse(cmdCtrVector.begin(), cmdCtrVector.end());

    for (size_t i = 0; i < REPEAT_COUNT; ++i)
        std::copy(cmdCtrVector.begin(), cmdCtrVector.end(),
                  myIV.begin() + PREFIX_SIZE + (i * COUNTER_SIZE));

    auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_sessionKey);
    auto iv        = openssl::AESInitializationVector::createFromData(d_LastSessionIV);
    openssl::AESCipher cipher;

    ByteVector encIV;
    cipher.cipher(myIV, encIV, symkeyMac, iv, false);

    return encIV;
}

//TODO basic function (keep as is for now, will be reworked later), remove static when stable
static ByteVector parseSecureResponseStatus(const ByteVector &response)
{
    constexpr size_t STATUS_SIZE = 2;

    if (response.size() < STATUS_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid response size");

    return ByteVector(response.end() - STATUS_SIZE, response.end());
}

ByteVector SAMAV2ISO7816Commands::transmitSecureChained(const ByteVector &cmd,
                                                        unsigned char PxIndex, bool hasLe)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid APDU : too short");

    auto adapter = getISO7816ReaderCardAdapter();

    try
    {
        auto vec           = prepareSecureChainedPayload(cmd, hasLe);
        ByteVector encData = vec.first;
        ByteVector mac     = vec.second;
        auto frames = createSecureChainedApduFrames(cmd, encData, mac, PxIndex, hasLe);
        ByteVector response;
        std::reverse(frames.begin(), frames.end());
        for (auto &frame : frames) //TODO add 0x90AF check after version is stable
            response = adapter->sendCommand(frame); //TODO rework this after version is stable
        ++d_cmdCtr;
        resetIVs();
        response = verifyAndDecryptResponse(response); // TODO rework this with parseSecureResponseStatus
        return response;
    }
    catch (const std::exception &e)
    {
        secureZero(d_sessionKey);
        secureZero(d_macSessionKey);
        resetIVs();
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 std::string("SAM transmit failed : ") + e.what());
    }
}

ByteVector SAMAV2ISO7816Commands::transmit(ByteVector cmd, bool first, bool last, bool s_mode)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid APDU : too short");

    auto adapter = getISO7816ReaderCardAdapter();

    if (d_sessionKey.empty())
        return adapter->sendCommand(cmd);

    ByteVector result;

    try
    {
        /*if (protect == sam::HostMode::MAC) // TODO test with release version
        {
            protectedCmd = createMacProtectionCmd(cmd);
            ++d_cmdCtr;
        }*/
        if (first || s_mode)
            cmd = createfullProtectionCmd(cmd);
        result = adapter->sendCommand(cmd);
        if (first)
            resetIVs();
        if (first || s_mode)
            ++d_cmdCtr;
        if (last || !s_mode)
            result = verifyAndDecryptResponse(result);
        if (last)
            resetIVs();
        return result;
    }
    catch (const std::exception &e)
    {
        secureZero(d_sessionKey);
        secureZero(d_macSessionKey);
        resetIVs();
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 std::string("SAM transmit failed : ") + e.what());
    }
}

void SAMAV2ISO7816Commands::resetIVs()
{
    secureZero(d_LastSessionIV);
    secureZero(d_lastMacIV);
}

void SAMAV2ISO7816Commands::secureZero(ByteVector &vec)
{
    if (!vec.empty())
        OPENSSL_cleanse(vec.data(), vec.size());
}

std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2>>
SAMAV2ISO7816Commands::getKeyEntry(unsigned char keyno)
{
    constexpr size_t EXPECTED_SIZE_MIN         = 14;
    constexpr size_t EXPECTED_SIZE_MAX         = 15;
    constexpr unsigned char STATUS_SUCCESS_MSB = 0x90;
    constexpr unsigned char STATUS_SUCCESS_LSB = 0x00;

    unsigned char cmd[] = {d_cla, 0x64, keyno, 0x00, 0x00};
    ByteVector cmd_vector(cmd, cmd + 5);
    ByteVector result = transmit(cmd_vector, true, true);

    if ((result.size() != EXPECTED_SIZE_MIN && result.size() != EXPECTED_SIZE_MAX) ||
        result[result.size() - 2] != STATUS_SUCCESS_MSB ||
        result[result.size() - 1] != STATUS_SUCCESS_LSB)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getKeyEntry failed.");
    }

    const size_t resultSize = result.size();
    KeyEntryAV2Information keyentryinformation;
    keyentryinformation.ExtSET = result[resultSize - 3];
    memcpy(keyentryinformation.set, &result[resultSize - 5], 2);

    keyentryinformation.kuc          = result[resultSize - 6];
    keyentryinformation.cekv         = result[resultSize - 7];
    keyentryinformation.cekno        = result[resultSize - 8];
    keyentryinformation.desfirekeyno = result[resultSize - 9];

    memcpy(keyentryinformation.desfireAid, &result[resultSize - 12], 3);

    if (resultSize == EXPECTED_SIZE_MIN)
    {
        keyentryinformation.verb = result[resultSize - 13];
        keyentryinformation.vera = result[resultSize - 14];
    }
    else
    {
        keyentryinformation.verc = result[resultSize - 13];
        keyentryinformation.verb = result[resultSize - 14];
        keyentryinformation.vera = result[resultSize - 15];
    }

    auto keyentry = std::make_shared<SAMKeyEntry<KeyEntryAV2Information, SETAV2>>();
    keyentry->setSET(keyentryinformation.set);
    keyentry->setKeyEntryInformation(keyentryinformation);
    keyentry->setKeyTypeFromSET();
    keyentry->setUpdateMask(0);

    return keyentry;
}

std::shared_ptr<SAMKucEntry> SAMAV2ISO7816Commands::getKUCEntry(unsigned char kucno)
{
    std::shared_ptr<SAMKucEntry> kucentry(new SAMKucEntry);
    unsigned char cmd[] = {d_cla, 0x6c, kucno, 0x00, 0x00};
    ByteVector cmd_vector(cmd, cmd + 5);

    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() == 12 &&
        (result[result.size() - 2] == 0x90 || result[result.size() - 1] == 0x00))
    {
        SAMKUCEntryStruct kucentrys;
        memcpy(&kucentrys, &result[0], sizeof(SAMKUCEntryStruct));
        kucentry->setKucEntryStruct(kucentrys);
    }
    else
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getKUCEntry failed.");
    return kucentry;
}

void SAMAV2ISO7816Commands::changeKUCEntry(unsigned char kucno,
                                           std::shared_ptr<SAMKucEntry> kucentry,
                                           std::shared_ptr<DESFireKey> /*key*/)
{
    if (d_sessionKey.size() == 0)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "Failed: AuthentificationHost have to be done before use such command.");

    unsigned char cmd[] = {d_cla, 0xcc, kucno, kucentry->getUpdateMask(), 0x06};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(),
                      reinterpret_cast<char *>(&kucentry->getKucEntryStruct()),
                      reinterpret_cast<char *>(&kucentry->getKucEntryStruct()) + 6);
    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKUCEntry failed.");
}

void SAMAV2ISO7816Commands::changeKeyEntry(
    unsigned char keyno,
    std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2>> keyentry,
    std::shared_ptr<DESFireKey> /*key*/)
{
    if (d_sessionKey.size() == 0)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "Failed: AuthentificationHost have to be done before use such command.");

    unsigned char proMas = keyentry->getUpdateMask();

    size_t buffer_size  = SAM_KEY_BUFFER_SIZE + sizeof(KeyEntryAV2Information);
    unsigned char *data = new unsigned char[buffer_size]();

    memcpy(data, keyentry->getData(), SAM_KEY_BUFFER_SIZE);
    memcpy(data + SAM_KEY_BUFFER_SIZE, &keyentry->getKeyEntryInformation(),
           sizeof(KeyEntryAV2Information));
    ByteVector vectordata(data, data + buffer_size);
    delete[] data;

    unsigned char cmd[] = {d_cla, 0xc1, keyno, proMas, (unsigned char)vectordata.size()};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKeyEntry failed.");
}

void SAMAV2ISO7816Commands::changeKeyEntryOffline(
    unsigned char keyno, const KeyEntryUpdateSettings &updateSettings,
    unsigned short changecnt, const ByteVector &encke)
{
    unsigned char proMas = SAMBasicKeyEntry::getUpdateMask(updateSettings);

    ByteVector vectordata;
    vectordata.push_back(static_cast<unsigned char>((changecnt >> 8) & 0xff));
    vectordata.push_back(static_cast<unsigned char>(changecnt & 0xff));
    vectordata.insert(vectordata.end(), encke.begin(), encke.end());

    unsigned char cmd[] = {d_cla, 0xc1, keyno, proMas, (unsigned char)vectordata.size()};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "changeKeyEntryOffline failed.")
}

void SAMAV2ISO7816Commands::changeKUCEntryOffline(
    unsigned char kucno, const KucEntryUpdateSettings &updateSettings,
    unsigned short changecnt, const ByteVector &enckuc)
{
    unsigned char proMas = SAMKucEntry::getUpdateMask(updateSettings);

    ByteVector vectordata;
    vectordata.push_back(static_cast<unsigned char>((changecnt >> 8) & 0xff));
    vectordata.push_back(static_cast<unsigned char>(changecnt & 0xff));
    vectordata.insert(vectordata.end(), enckuc.begin(), enckuc.end());

    unsigned char cmd[] = {d_cla, 0xcc, kucno, proMas, (unsigned char)vectordata.size()};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "changeKUCEntryOffline failed.")
}

void SAMAV2ISO7816Commands::disableKeyEntryOffline(unsigned char keyno,
                                                   unsigned short changecnt,
                                                   const ByteVector &encuid)
{
    ByteVector vectordata;
    vectordata.push_back(static_cast<unsigned char>((changecnt >> 8) & 0xff));
    vectordata.push_back(static_cast<unsigned char>(changecnt & 0xff));
    vectordata.insert(vectordata.end(), encuid.begin(), encuid.end());

    unsigned char cmd[] = {d_cla, 0xd8, keyno, 0x00, (unsigned char)vectordata.size()};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "disableKeyEntryOffline failed.")
}

ByteVector SAMAV2ISO7816Commands::dumpSecretKey(unsigned char keyno,
                                                unsigned char keyversion,
                                                ByteVector divInpu)
{
    unsigned char p1 = 0x00;

    if (divInpu.size())
        p1 |= 0x02;

    unsigned char cmd[] = {
        d_cla, 0xd6,       p1,  0x00, static_cast<unsigned char>(divInpu.size() + 0x02),
        keyno, keyversion, 0x00};
    ByteVector cmd_vector(cmd, cmd + 8);
    cmd_vector.insert(cmd_vector.end() - 1, divInpu.begin(), divInpu.end());

    ByteVector result = transmit(cmd_vector);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "dumpSecretKey failed.");

    return ByteVector(result.begin(), result.end() - 2);
}

void SAMAV2ISO7816Commands::activateOfflineKey(unsigned char keyno,
                                               unsigned char keyversion,
                                               ByteVector divInpu)
{
    ByteVector activateOfflineKey = {0x80,
                                     0x01,
                                     static_cast<unsigned char>(divInpu.size() > 0x00),
                                     0x00,
                                     static_cast<unsigned char>(0x02 + divInpu.size()),
                                     keyno,
                                     keyversion};
    activateOfflineKey.insert(activateOfflineKey.end(), divInpu.begin(), divInpu.end());

    transmit(activateOfflineKey);
}

ByteVector SAMAV2ISO7816Commands::decipherOfflineData(ByteVector data)
{
    ByteVector decipherOfflineData = {
        0x80, 0x0d, 0x00, 0x00, static_cast<unsigned char>(data.size()), 0x00,
    };
    decipherOfflineData.insert(decipherOfflineData.end() - 1, data.begin(), data.end());

    auto result = transmit(decipherOfflineData);
    EXCEPTION_ASSERT_WITH_LOG(result.size() > 2, LibLogicalAccessException,
                              "Response is too short");
    result.resize(result.size() - 2);
    return result;
}

ByteVector SAMAV2ISO7816Commands::encipherOfflineData(ByteVector data)
{
    ByteVector encipherOfflineData = {
        0x80, 0x0e, 0x00, 0x00, static_cast<unsigned char>(data.size()), 0x00,
    };
    encipherOfflineData.insert(encipherOfflineData.end() - 1, data.begin(), data.end());

    auto result = transmit(encipherOfflineData);
    EXCEPTION_ASSERT_WITH_LOG(result.size() > 2, LibLogicalAccessException,
                              "Response is too short");
    result.resize(result.size() - 2);
    return result;
}

ByteVector SAMAV2ISO7816Commands::cmacOffline(const ByteVector &data)
{
    unsigned int block_size = 16;
    unsigned char Rb        = 0x87;

    ByteVector blankbuf;
    blankbuf.resize(block_size, 0x00);
    ByteVector L = encipherOfflineData(blankbuf);

    ByteVector K1;
    if ((L[0] & 0x80) == 0x00)
    {
        K1 = openssl::CMACCrypto::shift_string(L);
    }
    else
    {
        K1 = openssl::CMACCrypto::shift_string(L, Rb);
    }

    ByteVector K2;
    if ((K1[0] & 0x80) == 0x00)
    {
        K2 = openssl::CMACCrypto::shift_string(K1);
    }
    else
    {
        K2 = openssl::CMACCrypto::shift_string(K1, Rb);
    }

    int pad = (block_size - (data.size() % block_size)) % block_size;
    if (data.size() == 0)
        pad = block_size;

    ByteVector padded_data = data;
    if (pad > 0)
    {
        padded_data.push_back(0x80);
        if (pad > 1)
        {
            for (int i = 0; i < (pad - 1); ++i)
            {
                padded_data.push_back(0x00);
            }
        }
    }

    // XOR with K1
    if (pad == 0)
    {
        for (unsigned int i = 0; i < K1.size(); ++i)
        {
            padded_data[padded_data.size() - K1.size() + i] = static_cast<unsigned char>(
                padded_data[padded_data.size() - K1.size() + i] ^ K1[i]);
        }
    }
    // XOR with K2
    else
    {
        for (unsigned int i = 0; i < K2.size(); ++i)
        {
            padded_data[padded_data.size() - K2.size() + i] = static_cast<unsigned char>(
                padded_data[padded_data.size() - K2.size() + i] ^ K2[i]);
        }
    }

    ByteVector cmac = encipherOfflineData(padded_data);
    if (cmac.size() > block_size)
    {
        cmac = ByteVector(cmac.end() - block_size, cmac.end());
    }

    return cmac;
}


void SAMAV2ISO7816Commands::PKI_GenerateKeyPair(
    unsigned char keyNo, unsigned short configSettings, unsigned char keyNoCEK,
    unsigned char keyNoVCEK, unsigned char keyNoRef, unsigned char *keyNoAEK,
    unsigned char *keyVAEK, unsigned short nLen, const ByteVector &pki_e,
    bool includeAccess)
{
    if (keyNo > 0x01)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateKeyPair : invalid keyNo");
    if (nLen < 0x40 || nLen > 0x100 || (nLen % 8) != 0)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_GenerateKeyPair : invalid RSA modulus length (nLen).");

    const bool provideE = !pki_e.empty();
    const unsigned short eLen =
        provideE ? static_cast<unsigned short>(pki_e.size()) : 0x0004;

    if (provideE)
    {
        if (eLen < 0x04 || eLen > 0x100 || (eLen % 4) != 0 || eLen > nLen)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_GenerateKeyPair : invalid exponent length (PKI_eLen).");
        if ((pki_e.back() & 0x01) == 0)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_GenerateKeyPair : PKI_e must be odd.");
    }
    unsigned short effectiveConfig = configSettings;
    if (includeAccess)
    {
        effectiveConfig &= ~0x0004;
        if (!keyNoAEK || !keyVAEK)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "AEK/VAEK required but null");
    }

    unsigned char p1 = 0x00;
    if (provideE)
        p1 |= 0x01;
    if (includeAccess)
        p1 |= 0x02;
    p1 &= 0x03;
    ByteVector payload;

    payload.push_back(keyNo);
    payload.push_back(static_cast<unsigned char>((effectiveConfig >> 8) & 0xFF));
    payload.push_back(static_cast<unsigned char>(effectiveConfig & 0xFF));
    payload.push_back(keyNoCEK);
    payload.push_back(keyNoVCEK);
    payload.push_back(keyNoRef);
    if (includeAccess)
    {
        payload.push_back(*keyNoAEK);
        payload.push_back(*keyVAEK);
    }
    payload.push_back(static_cast<unsigned char>((nLen >> 8) & 0xFF));
    payload.push_back(static_cast<unsigned char>(nLen & 0xFF));
    payload.push_back(static_cast<unsigned char>((eLen >> 8) & 0xFF));
    payload.push_back(static_cast<unsigned char>(eLen & 0xFF));
    if (provideE)
        payload.insert(payload.end(), pki_e.begin(), pki_e.end());

    const size_t size = payload.size();

    auto buildAPDU = [&](const ByteVector &data) -> ByteVector
    {
        ByteVector apdu;
        apdu.push_back(d_cla);
        apdu.push_back(0x15);
        apdu.push_back(p1);
        apdu.push_back(0x00);
        if (data.size() > sam::MAX_APDU_DATA_SIZE)
            apdu.push_back(0x00);
        else
            apdu.push_back(static_cast<unsigned char>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        return apdu;
    };
    const ByteVector fullPayload = buildAPDU(payload);
    const ByteVector resp = transmitSecureChained(fullPayload); // TODO : keep for testing
    if (resp.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateKeyPair : invalid response length");
    const uint16_t sw = (resp[resp.size() - 2] << 8) | resp[resp.size() - 1];
    if (sw == 0x9000)
        return;
    if (sw == 0x6A80)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateKeyPair : RSA key error");
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_GenerateKeyPair : unexpected status word");
}

void SAMAV2ISO7816Commands::PKI_ImportKey(
    unsigned char keyNo, unsigned short configSettings, unsigned char keyNoCEK,
    unsigned char keyNoVCEK, unsigned char refNoKUC, const ByteVector &pki_n,
    const ByteVector &pki_e, const ByteVector &pki_p, const ByteVector &pki_q,
    const ByteVector &pki_dP, const ByteVector &pki_dQ, const ByteVector &pki_ipq,
    unsigned char *keyNoAEK, unsigned char *keyVAEK, bool includeAccess,
    bool updateSettingsOnly)
{
    const bool hasPrivate = !pki_p.empty();

    if (hasPrivate)
    {
        if (keyNo > 0x01)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_ImportKey : keyNo out of range for private key.");
    }
    else
    {
        if (keyNo > 0x02)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_ImportKey : keyNo out of range for public key.");
    }

    const size_t nLen = pki_n.size();
    size_t eLen;
    ByteVector normalizedE = pki_e;

    if (!updateSettingsOnly)
    {
        if (pki_n.empty() || pki_e.empty())
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : missing RSA components.");

        if (nLen < 0x40 || nLen > 0x100 || (nLen % 8) != 0)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : invalid modulus length.");

        if (nLen < 4 || (pki_n[0] == 0x00 && pki_n[1] == 0x00 && pki_n[2] == 0x00 &&
                         pki_n[3] == 0x00))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : modulus MSW must not be zero.");

        if (!normalizedE.empty() && (normalizedE.size() % 4) != 0)
        {
            const size_t pad = 4 - (normalizedE.size() % 4);
            normalizedE.insert(normalizedE.begin(), pad, 0x00);
        }

        eLen = normalizedE.size();

        if (eLen < 0x04 || eLen > 0x100 || (eLen % 4) != 0 || eLen > nLen)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : invalid exponent length.");

        if ((normalizedE.back() & 0x01) == 0)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : exponent must be odd.");

        if (pki_n[0] == 0x00)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : invalid modulus MSB.");
    }

    if (hasPrivate)
    {
        if (pki_q.empty() || pki_dP.empty() || pki_dQ.empty() || pki_ipq.empty())
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : incomplete CRT key.");

        if (pki_p[0] == 0x00 || pki_q[0] == 0x00)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : invalid CRT primes MSB.");
        const size_t nWords = (nLen + 3) / 4;
        const size_t pWords = (pki_p.size() + 3) / 4;
        const size_t qWords = (pki_q.size() + 3) / 4;
        if ((pWords + 2 > nWords) || (qWords + 2 > nWords))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : invalid CRT size relation.");
    }

    const bool disableRequested    = (configSettings & 0x0004) != 0;
    unsigned short effectiveConfig = configSettings;
    if (includeAccess)
    {
        if (disableRequested)
            LOG(LogLevel::WARNINGS)
                << "PKI_ImportKey : overriding PKI_SET disable bit due to AEK.";
        effectiveConfig &= ~0x0004;
    }

    if (includeAccess && (!keyNoAEK || !keyVAEK))
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportKey : AEK/VAEK required.");
    }

    unsigned char p1 = 0x00;

    if (updateSettingsOnly)
        p1 |= 0x01;
    if (includeAccess)
        p1 |= 0x02;
    p1 &= 0x03;

    ByteVector payload;
    payload.push_back(keyNo);
    payload.push_back(static_cast<unsigned char>((effectiveConfig >> 8) & 0xFF));
    payload.push_back(static_cast<unsigned char>(effectiveConfig & 0xFF));
    payload.push_back(keyNoCEK);
    payload.push_back(keyNoVCEK);
    payload.push_back(refNoKUC);

    if (includeAccess)
    {
        payload.push_back(*keyNoAEK);
        payload.push_back(*keyVAEK);
    }

    if (!updateSettingsOnly)
    {
        payload.push_back(static_cast<unsigned char>((nLen >> 8) & 0xFF));
        payload.push_back(static_cast<unsigned char>(nLen & 0xFF));
        payload.push_back(static_cast<unsigned char>((normalizedE.size() >> 8) & 0xFF));
        payload.push_back(static_cast<unsigned char>(normalizedE.size() & 0xFF));

        if (hasPrivate)
        {
            const auto pSize = pki_p.size();
            const auto qSize = pki_q.size();

            if (pSize > 0xFFFF || qSize > 0xFFFF)
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "PKI_ImportKey : invalid CRT length overflow.");

            const uint16_t pLen = static_cast<uint16_t>(pSize);
            const uint16_t qLen = static_cast<uint16_t>(qSize);

            payload.push_back(static_cast<unsigned char>((pLen >> 8) & 0xFF));
            payload.push_back(static_cast<unsigned char>(pLen & 0xFF));
            payload.push_back(static_cast<unsigned char>((qLen >> 8) & 0xFF));
            payload.push_back(static_cast<unsigned char>(qLen & 0xFF));
        }

        payload.insert(payload.end(), pki_n.begin(), pki_n.end());
        payload.insert(payload.end(), normalizedE.begin(), normalizedE.end());

        if (hasPrivate)
        {
            payload.insert(payload.end(), pki_p.begin(), pki_p.end());
            payload.insert(payload.end(), pki_q.begin(), pki_q.end());
            payload.insert(payload.end(), pki_dP.begin(), pki_dP.end());
            payload.insert(payload.end(), pki_dQ.begin(), pki_dQ.end());
            payload.insert(payload.end(), pki_ipq.begin(), pki_ipq.end());
        }
    }

    size_t offset = 0;
    bool success  = false;

    auto buildAPDU = [&](unsigned char p1, unsigned char p2, const ByteVector &data,
                         bool first, bool last)
    {
        ByteVector apdu;
        apdu.push_back(d_cla);
        apdu.push_back(0x19);
        apdu.push_back(p1);
        apdu.push_back(0x00);
        apdu.push_back(static_cast<unsigned char>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        return transmitSecureChained(apdu); // TODO : keep for testing
    };

    auto guardFailure = [&](const std::string &msg)
    {
        if (!success)
        {
            LOG(LogLevel::WARNINGS)
                << "PKI_ImportKey : failure → key likely disabled (keyNo = "
                << static_cast<int>(keyNo) << ")";
        }
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, msg);
    };


    while (offset < payload.size())
    {
        const size_t remaining  = payload.size() - offset;
        const size_t frameSize  = (std::min)(remaining, sam::MAX_APDU_DATA_SIZE);
        const bool isFirstFrame = (offset == 0);
        const bool isLastFrame  = (remaining <= sam::MAX_APDU_DATA_SIZE);

        ByteVector chunk(payload.begin() + offset, payload.begin() + offset + frameSize);

        const unsigned char p2 = isLastFrame ? 0x00 : 0xAF;
        offset += frameSize;

        const ByteVector resp = buildAPDU(p1, p2, chunk, isFirstFrame, isLastFrame);

        if (resp.size() < 2)
            guardFailure("PKI_ImportKey : invalid response.");

        uint16_t sw = (resp[resp.size() - 2] << 8) | resp.back();

        if (!isLastFrame)
        {
            if (sw != 0x90AF)
                guardFailure("PKI_ImportKey : chaining error.");
            continue;
        }

        if (sw == 0x9000)
        {
            success = true;
            return;
        }

        if (sw == 0x6A80)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : Key ref out of range.");

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportKey : unexpected status word.");
    }
}

ByteVector SAMAV2ISO7816Commands::PKI_ExportPrivateKey(unsigned char keyNo,
                                                       bool returnAEK)
{
    constexpr size_t MIN_RESPONSE_SIZE = 3;
    constexpr size_t MIN_KEY_DATA_SIZE = 13;

    if (keyNo > 0x01)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ExportPrivateKey : invalid key reference number.");

    ByteVector apdu;
    apdu.reserve(5);
    apdu.push_back(d_cla);
    apdu.push_back(0x1F);
    apdu.push_back(keyNo);
    apdu.push_back(returnAEK ? 0x80 : 0x00);
    apdu.push_back(0x00);
    ByteVector fullResponse;
    bool firstFrame = true;

    while (true)
    {
        ByteVector resp = transmit(apdu, firstFrame, !firstFrame, false);
        if (resp.size() < MIN_RESPONSE_SIZE)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ExportPrivateKey : invalid response length.");
        const uint16_t sw = (resp[resp.size() - 2] << 8) | resp[resp.size() - 1];
        resp.resize(resp.size() - 2);
        fullResponse.insert(fullResponse.end(), resp.begin(), resp.end());

        if (sw == 0x90AF)
        {
            firstFrame = false;
            continue;
        }
        if (sw == 0x9000)
            break;
        if (sw == 0x6986)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_ExportPrivateKey : private key export not allowed.");

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ExportPrivateKey : unexpected status word.");
    }
    return fullResponse;
}

ByteVector SAMAV2ISO7816Commands::PKI_ExportPublicKey(unsigned char keyNo, bool returnAEK)
{
    // TODO : add this in header
    constexpr uint16_t SW_MORE_DATA          = 0x90AF;
    constexpr uint16_t SW_SUCCESS            = 0x9000;
    constexpr size_t MIN_FIRST_RESPONSE_SIZE = 9;
    constexpr size_t MIN_RESPONSE_SIZE       = 3;

    if (keyNo > 0x02)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ExportPublicKey : keyNo out of range.");

    ByteVector apdu;
    apdu.reserve(5);
    apdu.push_back(d_cla);
    apdu.push_back(0x18);
    apdu.push_back(keyNo);
    apdu.push_back(returnAEK ? 0x80 : 0x00);
    apdu.push_back(0x00);

    ByteVector result;
    bool firstFrame = true;

    while (true)
    {
        ByteVector resp = transmit(apdu, firstFrame, !firstFrame, false);

        if (resp.size() < MIN_RESPONSE_SIZE)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ExportPublicKey : invalid response size.");

        if (firstFrame && resp.size() < MIN_FIRST_RESPONSE_SIZE)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ExportPublicKey : incomplete first response.");

        const uint16_t sw =
            static_cast<uint16_t>(resp[resp.size() - 2]) << 8 | resp.back();
        result.insert(result.end(), resp.begin(), resp.end() - 2);

        if (sw == SW_MORE_DATA)
        {
            firstFrame = false;
            continue;
        }
        if (sw == SW_SUCCESS)
            return result;
        if (sw == 0x6986)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ExportPublicKey : export not allowed.");

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ExportPublicKey : unexpected status word.");
    }
}

ByteVector SAMAV2ISO7816Commands::buildPlaintext(uint16_t changeCtr, const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries)
{
    ByteVector pt;

    pt.push_back((changeCtr >> 8) & 0xFF);
    pt.push_back(changeCtr & 0xFF);

    for (const auto &entry : entries)
    {

        if (!entry)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Null SAMBasicKeyEntry");
        }

        auto *avEntry =
            dynamic_cast<SAMKeyEntry<KeyEntryAV2Information, SETAV2> *>(entry.get());

        if (!avEntry)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Entry is not a SAMKeyEntry AV2 type");
        }

        uint8_t keyNo  = avEntry->getKeyEntryInformation().desfirekeyno;
        uint8_t proMas = entry->getUpdateMask();

        const unsigned char *data = entry->getData();
        size_t len                = entry->getLength();

        pt.push_back(keyNo);
        pt.push_back(proMas);
        pt.insert(pt.end(), data, data + len);
    }

    return pt;
}

ByteVector SAMAV2ISO7816Commands::rsa_oaep_encrypt(EVP_PKEY *pubKey, const ByteVector &plaintext, const EVP_MD *md)
{
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pubKey, nullptr);
    if (!ctx)
        throw std::runtime_error("CTX init failed");

    if (EVP_PKEY_encrypt_init(ctx) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("encrypt_init failed");
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0 ||
        EVP_PKEY_CTX_set_rsa_oaep_md(ctx, md) <= 0 ||
        EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, md) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("OAEP parameter setup failed");
    }

    size_t outLen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outLen, plaintext.data(), plaintext.size()) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("encrypt size calc failed");
    }

    ByteVector out(outLen);

    if (EVP_PKEY_encrypt(ctx, out.data(), &outLen, plaintext.data(), plaintext.size()) <=
        0)
    {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("OAEP encrypt failed");
    }
    out.resize(outLen);
    EVP_PKEY_CTX_free(ctx);
    return out;
}

ByteVector SAMAV2ISO7816Commands::rsa_pss_sign(EVP_PKEY *privKey, const ByteVector &data,
                                               const EVP_MD *md)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx)
        throw std::runtime_error("MD_CTX failed");

    EVP_PKEY_CTX *pkey_ctx = nullptr;

    if (EVP_DigestSignInit(ctx, &pkey_ctx, md, nullptr, privKey) <= 0)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("SignInit failed");
    }

    if (EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, RSA_PKCS1_PSS_PADDING) <= 0 ||
        EVP_PKEY_CTX_set_rsa_mgf1_md(pkey_ctx, md) <= 0 ||
        EVP_PKEY_CTX_set_rsa_pss_saltlen(pkey_ctx, EVP_MD_size(md)) <= 0)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("PSS config failed");
    }

    if (EVP_DigestSignUpdate(ctx, data.data(), data.size()) <= 0)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("SignUpdate failed");
    }

    size_t sigLen = 0;

    if (EVP_DigestSignFinal(ctx, nullptr, &sigLen) <= 0)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Sign size calculation failed");
    }

    ByteVector sig(sigLen);

    if (EVP_DigestSignFinal(ctx, sig.data(), &sigLen) <= 0)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Sign failed");
    }

    sig.resize(sigLen);
    EVP_MD_CTX_free(ctx);

    return sig;
}

const EVP_MD *SAMAV2ISO7816Commands::getHash(uint8_t hashAlgo)
{
    switch (hashAlgo & 0x03)
    {
    case 0x00: return EVP_sha1();
    case 0x01: return EVP_sha224();
    case 0x02: return EVP_sha256();
    default: throw std::runtime_error("Invalid hash");
    }
}

void SAMAV2ISO7816Commands::buildCryptogram(
    EVP_PKEY *encKey, EVP_PKEY *signKey, uint8_t keyNoEnc, uint8_t keyNoSign,
    uint16_t changeCtr, const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries,
    uint8_t hashAlgo, ByteVector &encFrame, ByteVector &signature)
{
    const EVP_MD *md = getHash(hashAlgo);

    if (!md)
        throw std::runtime_error("Invalid hash algorithm");

    // plaintext
    ByteVector pt = buildPlaintext(changeCtr, entries);

    // encryption
    encFrame = rsa_oaep_encrypt(encKey, pt, md);

    // signature
    ByteVector toSign;
    toSign.reserve(2 + encFrame.size());
    toSign.push_back(keyNoEnc);
    toSign.push_back(keyNoSign);
    toSign.insert(toSign.end(), encFrame.begin(), encFrame.end());
    signature = rsa_pss_sign(signKey, toSign, md);
}

ByteVector SAMAV2ISO7816Commands::PKI_UpdateKeyEntries(
    EVP_PKEY *encKey, EVP_PKEY *signKey, unsigned char keyNoEnc, unsigned char keyNoSign,
    unsigned char keyNoAck, unsigned char hashAlgo,
    const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries, uint16_t changeCounter)
{
    if (entries.empty() || entries.size() > 3)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : invalid number of entries");

    if (!encKey || !signKey)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid ENC or SIGN key");

    ByteVector encFrame;
    ByteVector signature;

    buildCryptogram(encKey, signKey, keyNoEnc, keyNoSign, changeCounter, entries,
                    hashAlgo, encFrame, signature);

    return PKI_UpdateKeyEntries(keyNoEnc, keyNoSign, keyNoAck, hashAlgo,
                                static_cast<unsigned char>(entries.size()), encFrame,
                                signature);
}

ByteVector SAMAV2ISO7816Commands::PKI_UpdateKeyEntries(
    unsigned char keyNoEnc, unsigned char keyNoSign, unsigned char keyNoAck,
    unsigned char hashAlgo, unsigned char nbKeyEntries, const ByteVector &encKeyFrame,
    const ByteVector &signature)
{
    if (hashAlgo > 0x03)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Invalid hashAlgo (must be 0..3)");

    if (nbKeyEntries == 0 || nbKeyEntries > 3)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Invalid nbKeys (must be 1..3)");

    if (encKeyFrame.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "EncKeyFrame cannot be empty");

    if (signature.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Signature cannot be empty");

    uint8_t p1 = (hashAlgo & 0x03) | ((nbKeyEntries & 0x03) << 2);

    ByteVector payload;

    payload.push_back(keyNoEnc);
    payload.push_back(keyNoSign);
    payload.insert(payload.end(), encKeyFrame.begin(), encKeyFrame.end());
    payload.insert(payload.end(), signature.begin(), signature.end());

    const size_t MAX_FRAME_SIZE = 0xFF;

    size_t offset = 0;
    bool success  = false;
    ByteVector result;

    auto buildAPDU = [&](unsigned char p2, const ByteVector &data, bool first,
                         bool last) -> ByteVector
    {
        ByteVector apdu;
        apdu.push_back(d_cla);
        apdu.push_back(0x1D);
        apdu.push_back(p1);
        apdu.push_back(p2);

        ByteVector frameData = data;
        if (last)
            frameData.insert(frameData.begin() + 2, keyNoAck);
        if (frameData.size() > 0xFF)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_UpdateKeyEntries : frame too large for Lc (max 255).");
        apdu.push_back(static_cast<unsigned char>(frameData.size()));
        apdu.insert(apdu.end(), frameData.begin(), frameData.end());
        if (last)
            apdu.push_back(0x00);
        return transmit(apdu, first, last);
    };

    auto guardFailure = [&](const std::string &msg)
    {
        if (!success)
        {
            LOG(LogLevel::WARNINGS)
                << "PKI_UpdateKeyEntries : failure, no successful update committed.";
        }
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, msg);
    };

    while (offset < payload.size())
    {
        size_t remaining  = payload.size() - offset;
        size_t frameSize  = (std::min)(remaining, MAX_FRAME_SIZE);
        bool isFirstFrame = (offset == 0);
        bool isLastFrame  = (remaining <= MAX_FRAME_SIZE);

        ByteVector chunk(payload.begin() + offset, payload.begin() + offset + frameSize);

        unsigned char p2 = isLastFrame ? 0x00 : 0xAF;
        offset += frameSize;

        ByteVector resp = buildAPDU(p2, chunk, isFirstFrame, isLastFrame);

        if (resp.size() < 2)
            guardFailure("PKI_UpdateKeyEntries : response too short");

        uint8_t sw1 = resp[resp.size() - 2];
        uint8_t sw2 = resp[resp.size() - 1];

        if (!isLastFrame)
        {
            if (!(sw1 == 0x90 && sw2 == 0xAF))
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "PKI_UpdateKeyEntries : chaining failed");
        }
        else
        {
            if (sw1 == 0x90 && sw2 == 0x00)
            {
                result  = resp;
                success = true;
            }
            else if (sw1 == 0x6A && sw2 == 0x80)
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "PKI_UpdateKeyEntries : incorrect ChangeCtr");
            }
            else
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "PKI_UpdateKeyEntries : unexpected status word");
            }
        }
    }
    if (result.size() > 2)
        return ByteVector(result.begin(), result.end() - 2);
    return result;
}

ByteVector SAMAV2ISO7816Commands::PKI_EncipherKeyEntries(
    unsigned char hashAlgo, unsigned char keyNoEnc, unsigned char keyNoSign,
    unsigned char keyNoDec, unsigned char keyNoVerif, unsigned short persoCtr,
    const std::vector<std::pair<unsigned char, unsigned char>> &keyEntries,
    const ByteVector &divInput)
{
    if (keyEntries.empty() || keyEntries.size() > 3)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : 1 to 3 key entries allowed");

    if (hashAlgo > 0x03)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_EncipherKeyEntries : invalid hash algorithm selector");

    if (!divInput.empty() && divInput.size() > 31)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : DivInput must be 1..31 bytes");

    if (keyNoEnc > 0x02 || keyNoVerif > 0x02 || keyNoSign > 0x01 || keyNoDec > 0x01)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : invalid key reference");

    for (const auto &k : keyEntries)
    {
        if (k.first > 0x7F || k.second > 0x7F)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_EncipherKeyEntries : invalid key entry pair");
    }

    unsigned char p1 = 0x00;

    p1 |= (hashAlgo & 0x03);
    p1 |= (static_cast<unsigned char>(keyEntries.size()) & 0x03) << 2;
    if (!divInput.empty())
        p1 |= 0x10;

    ByteVector payload;
    payload.push_back(keyNoEnc);
    payload.push_back(keyNoSign);
    payload.push_back(keyNoDec);
    payload.push_back(keyNoVerif);
    payload.push_back(static_cast<unsigned char>((persoCtr >> 8) & 0xFF));
    payload.push_back(static_cast<unsigned char>(persoCtr & 0xFF));
    for (const auto &entry : keyEntries)
    {
        payload.push_back(entry.first);
        payload.push_back(entry.second);
    }
    if (!divInput.empty())
        payload.insert(payload.end(), divInput.begin(), divInput.end());

    size_t offset = 0;
    bool success  = false;
    ByteVector fullResponse;

    auto buildAPDU = [&](unsigned char p2, const ByteVector &data, bool first,
                         bool last) -> ByteVector
    {
        ByteVector apdu;
        apdu.push_back(d_cla);
        apdu.push_back(0x12);
        apdu.push_back(p1);
        apdu.push_back(p2);

        if (data.size() > sam::MAX_APDU_DATA_SIZE)
        {
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_EncipherKeyEntries : frame too large for Lc (max 255).");
        }
        apdu.push_back(static_cast<unsigned char>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        apdu.push_back(0x00);
        return transmit(apdu, first, last);
    };

    auto guardFailure = [&](const std::string &msg)
    {
        if (!success)
        {
            LOG(LogLevel::WARNINGS)
                << "PKI_EncipherKeyEntries : failure, no successful update committed.";
        }
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, msg);
    };

    while (offset < payload.size())
    {
        const size_t remaining  = payload.size() - offset;
        const size_t frameSize  = (std::min)(remaining, sam::MAX_APDU_DATA_SIZE);
        const bool isFirstFrame = (offset == 0);
        const bool isLastFrame  = (remaining <= sam::MAX_APDU_DATA_SIZE);

        ByteVector chunk(payload.begin() + offset, payload.begin() + offset + frameSize);

        const unsigned char p2 = isLastFrame ? 0x00 : 0xAF;
        offset += frameSize;

        ByteVector resp = buildAPDU(p2, chunk, isFirstFrame, isLastFrame);
        fullResponse.insert(fullResponse.end(), resp.begin(), resp.end() - 2);

        if (resp.size() < 2)
            guardFailure("PKI_EncipherKeyEntries : response too short");

        const uint16_t sw = (resp[resp.size() - 2] << 8) | resp[resp.size() - 1];

        if (!isLastFrame)
        {
            if (sw != 0x90AF)
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "PKI_EncipherKeyEntries : chaining failed");
            continue;
        }
        if (sw == 0x9000)
        {
            success = true;
            return fullResponse;
        }
        if (sw == 0x6A80)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_EncipherKeyEntries : at least one selected key not reserved");

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : unexpected status word");
    }

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_EncipherKeyEntries : incomplete execution");
}

ByteVector SAMAV2ISO7816Commands::PKI_GenerateHash(unsigned char hashAlgo,
                                                   const ByteVector &message)
{
    if (message.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateHash : message cannot be empty.");

    const unsigned char p1 = (hashAlgo & 0x03);

    size_t offset   = 0;
    bool firstFrame = true;

    ByteVector response;

    auto buildAPDU = [&](unsigned char p2, const ByteVector &data, bool includeMLen,
                         bool isLastFrame) -> ByteVector
    {
        ByteVector apdu;

        apdu.push_back(d_cla);
        apdu.push_back(0x17);
        apdu.push_back(p1);
        apdu.push_back(p2);

        ByteVector frame = data;

        if (includeMLen)
        {
            ByteVector mlen(4, 0x00);
            BufferHelper::setUInt32(mlen, static_cast<uint32_t>(message.size()));
            frame.insert(frame.begin(), mlen.begin(), mlen.end());
        }

        if (frame.size() > 0xFF)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_GenerateHash : frame too large (>255).");

        apdu.push_back(static_cast<unsigned char>(frame.size()));
        apdu.insert(apdu.end(), frame.begin(), frame.end());

        if (isLastFrame && p2 == 0x00)
            apdu.push_back(0x00);
        return transmit(apdu, firstFrame, isLastFrame);
    };

    auto guardFailure = [&](const ByteVector &resp)
    {
        if (resp.size() < 2)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_GenerateHash : invalid response size.");
        }
    };

    while (offset < message.size())
    {
        const size_t remaining = message.size() - offset;
        const size_t overhead  = firstFrame ? 4 : 0;
        const size_t maxChunk  = sam::MAX_APDU_DATA_SIZE - overhead;
        const size_t chunkSize = (remaining > maxChunk) ? maxChunk : remaining;

        ByteVector chunk(message.begin() + offset, message.begin() + offset + chunkSize);
        offset += chunkSize;

        const bool lastFrame   = (offset >= message.size());
        const unsigned char p2 = lastFrame ? 0x00 : 0xAF;
        ByteVector resp        = buildAPDU(p2, chunk, firstFrame, lastFrame);
        guardFailure(resp);
        const uint16_t sw = (resp[resp.size() - 2] << 8) | resp[resp.size() - 1];
        firstFrame        = false;
        if (sw == 0x9000)
        {
            resp.resize(resp.size() - 2);
            return resp;
        }

        if (sw == 0x90AF)
            continue;
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateHash : unexpected status word.");
    }

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_GenerateHash : incomplete execution.");
}

void SAMAV2ISO7816Commands::PKI_GenerateSignature(unsigned char hashAlgo,
                                                  unsigned char keyNoSign,
                                                  const ByteVector &hash)
{
    if (hashAlgo > 0x03)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_GenerateSignature : invalid hashAlgo (must be 0x00..0x03).");

    if (keyNoSign > 0x01)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_GenerateSignature : invalid keyNoSign (must be 0x00 or 0x01).");

    switch (hashAlgo & 0x03)
    {
    case 0x00:
        if (hash.size() != 20)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_GenerateSignature : invalid hash size for SHA-1 "
                "(expected 20 bytes).");
        break;

    case 0x01:
        if (hash.size() != 28)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_GenerateSignature : invalid hash size for "
                                     "SHA-224 (expected 28 bytes).");
        break;

    case 0x02:
    case 0x03:
        if (hash.size() != 32)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_GenerateSignature : invalid hash size for "
                                     "SHA-256 (expected 32 bytes).");
        break;

    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateSignature : unsupported hash algorithm.");
    }

    const unsigned char p1 = (hashAlgo & 0x03);

    ByteVector apdu;
    apdu.reserve(5 + 1 + hash.size());
    apdu.push_back(d_cla);
    apdu.push_back(0x16);
    apdu.push_back(p1);
    apdu.push_back(0x00);

    const size_t lc = 1 + hash.size();
    if (lc > sam::MAX_APDU_DATA_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateSignature : payload too large.");

    apdu.push_back(static_cast<unsigned char>(lc));
    apdu.push_back(keyNoSign);
    apdu.insert(apdu.end(), hash.begin(), hash.end());

    const ByteVector response = transmit(apdu, true, true);

    if (response.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateSignature : invalid response length.");

    const uint16_t sw =
        (response[response.size() - 2] << 8) | response[response.size() - 1];

    if (sw != 0x9000)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateSignature failed with status word.");
}

ByteVector SAMAV2ISO7816Commands::PKI_SendSignature()
{
    const unsigned char RFU =
        0x00; // TODO : add in header (and apply changes in other functions) or remove

    ByteVector apdu;
    apdu.reserve(5);
    apdu.push_back(d_cla);
    apdu.push_back(0x1A);
    apdu.push_back(RFU);
    apdu.push_back(RFU);
    apdu.push_back(RFU);

    ByteVector response = transmit(apdu, true, true);

    if (response.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_SendSignature : response too short");

    const uint16_t sw =
        (response[response.size() - 2] << 8) | response[response.size() - 1];

    ByteVector data(response.begin(), response.end() - 2);

    if (sw == 0x9000)
    {
        if (data.size() < 8 || data.size() > 256)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_SendSignature : invalid signature size");
        return data;
    }

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_SendSignature : unknown status word");
}

void SAMAV2ISO7816Commands::PKI_VerifySignature(unsigned char hashAlgo,
                                                unsigned char keyNoVerif,
                                                const ByteVector &hash,
                                                const ByteVector &signature)
{
    if (hash.empty() || signature.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifySignature : empty hash or signature");

    if (hashAlgo > 0x03)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifySignature : invalid hashAlgo");

    ByteVector payload;
    payload.reserve(1 + hash.size() + signature.size());
    payload.push_back(keyNoVerif);
    payload.insert(payload.end(), hash.begin(), hash.end());
    payload.insert(payload.end(), signature.begin(), signature.end());

    auto buildAPDU = [&](unsigned char p1, unsigned char p2, const ByteVector &data,
                         bool first, bool last) -> ByteVector
    {
        if (data.size() > 0xFF)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_VerifySignature : frame too large");
        }

        ByteVector apdu;
        apdu.reserve(5 + data.size());
        apdu.push_back(d_cla);
        apdu.push_back(0x1B);
        apdu.push_back(p1);
        apdu.push_back(p2);
        apdu.push_back(static_cast<unsigned char>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        return transmit(apdu, first, last);
    };

    size_t offset          = 0;
    const unsigned char p1 = (hashAlgo & 0x03);

    while (offset < payload.size())
    {
        const size_t remaining  = payload.size() - offset;
        const size_t frameSize  = (std::min)(remaining, sam::MAX_APDU_DATA_SIZE);
        const bool isFirstFrame = (offset == 0);
        const bool isLastFrame  = (remaining <= sam::MAX_APDU_DATA_SIZE);

        ByteVector chunk(payload.begin() + offset, payload.begin() + offset + frameSize);
        offset += frameSize;

        const ByteVector resp =
            buildAPDU(p1, isLastFrame ? 0x00 : 0xAF, chunk, isFirstFrame, isLastFrame);

        if (resp.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_VerifySignature : invalid response size");


        const uint16_t sw = (resp[resp.size() - 2] << 8) | resp[resp.size() - 1];

        if (!isLastFrame)
        {
            if (sw == 0x90AF)
                continue;

            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_VerifySignature : chaining failed (expected 0x90AF)");
        }

        if (sw == 0x9000)
            return;

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifySignature : unexpected status word");
    }

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_VerifySignature : incomplete execution");
}

ByteVector SAMAV2ISO7816Commands::PKI_EncipherData(unsigned char hashAlgo,
                                                   unsigned char keyNoEnc,
                                                   const ByteVector &plainData)
{
    if (plainData.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherData : empty plaintext");

    if (hashAlgo > 0x03)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherData : invalid hashAlgo");

    const unsigned char p1 = (hashAlgo & 0x03);

    ByteVector payload;
    payload.reserve(1 + plainData.size());
    payload.push_back(keyNoEnc);
    payload.insert(payload.end(), plainData.begin(), plainData.end());

    if (payload.size() > sam::MAX_APDU_DATA_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherData : payload too large for single frame");

    ByteVector apdu;
    apdu.reserve(5 + payload.size() + 1);
    apdu.push_back(d_cla);
    apdu.push_back(0x13);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(static_cast<unsigned char>(payload.size()));
    apdu.insert(apdu.end(), payload.begin(), payload.end());
    apdu.push_back(0x00);

    ByteVector resp = transmit(apdu, true, true);

    if (resp.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherData : response too short");

    const uint16_t sw = (resp[resp.size() - 2] << 8) | resp[resp.size() - 1];

    ByteVector data(resp.begin(), resp.end() - 2);

    if (sw == 0x9000)
    {
        if (data.empty())
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_EncipherData : empty encrypted data");
        return data;
    }

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_EncipherData : unexpected status word");
}

ByteVector SAMAV2ISO7816Commands::PKI_DecipherData(unsigned char hashAlgo,
                                                   unsigned char keyNoDec,
                                                   const ByteVector &encData)
{
    if (encData.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_DecipherData : empty encrypted data");

    if (hashAlgo > 0x03)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_DecipherData : invalid hashAlgo");

    ByteVector payload;
    payload.reserve(1 + encData.size());
    payload.push_back(keyNoDec);
    payload.insert(payload.end(), encData.begin(), encData.end());

    auto buildAPDU = [&](unsigned char p1, unsigned char p2, const ByteVector &data,
                         bool first, bool last) -> ByteVector
    {
        ByteVector apdu;
        apdu.reserve(6 + data.size());
        apdu.push_back(d_cla);
        apdu.push_back(0x14);
        apdu.push_back(p1);
        apdu.push_back(p2);
        apdu.push_back(static_cast<unsigned char>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        apdu.push_back(0x00);
        return transmit(apdu, first, last);
    };

    size_t offset          = 0;
    const unsigned char p1 = (hashAlgo & 0x03);
    ByteVector result;

    while (offset < payload.size())
    {
        const size_t remaining  = payload.size() - offset;
        const size_t frameSize  = (std::min)(remaining, sam::MAX_APDU_DATA_SIZE);
        const bool isFirstFrame = (offset == 0);
        const bool isLastFrame  = (remaining <= sam::MAX_APDU_DATA_SIZE);

        ByteVector chunk(payload.begin() + offset, payload.begin() + offset + frameSize);
        offset += frameSize;

        const ByteVector resp =
            buildAPDU(p1, isLastFrame ? 0x00 : 0xAF, chunk, isFirstFrame, isLastFrame);

        if (resp.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_DecipherData : response too short");

        const uint16_t sw = (resp[resp.size() - 2] << 8) | resp[resp.size() - 1];

        if (!isLastFrame)
        {
            if (sw != 0x90AF)
                THROW_EXCEPTION_WITH_LOG(
                    LibLogicalAccessException,
                    "PKI_DecipherData : chaining failed (expected 0x90AF)");
            continue;
        }

        if (sw == 0x9000)
        {
            result.assign(resp.begin(), resp.end() - 2);
            return result;
        }

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_DecipherData : unexpected status word");
    }

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_DecipherData : incomplete execution");
}

void SAMAV2ISO7816Commands::PKI_ImportEccKey(
    unsigned char keyNo, unsigned short eccSet, unsigned char keyNoCEK,
    unsigned char keyNoVCEK, unsigned char keyNoKUC, unsigned char keyNoAEK,
    unsigned char keyNoVAEK, const ByteVector &eccPublicKey, bool settingsOnly)
{
    if (keyNo > 0x07)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportEccKey : invalid keyNo (0..7)");

    const unsigned char p1 = settingsOnly ? 0x01 : 0x00;

    ByteVector payload;
    payload.push_back(keyNo);
    payload.push_back(static_cast<unsigned char>((eccSet >> 8) & 0xFF));
    payload.push_back(static_cast<unsigned char>(eccSet & 0xFF));
    payload.push_back(keyNoCEK);
    payload.push_back(keyNoVCEK);
    payload.push_back(keyNoKUC);
    payload.push_back(keyNoAEK);
    payload.push_back(keyNoVAEK);

    if (!settingsOnly)
    {
        if (eccPublicKey.empty())
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportEccKey : eccPublicKey must be provided "
                                     "when settingsOnly = false");

        if (eccPublicKey[0] != 0x04)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportEccKey : ECC public key must start with "
                                     "0x04 (uncompressed format)");

        if (eccPublicKey.size() < 33 || eccPublicKey.size() > 65)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportEccKey : invalid ECC public key length "
                                     "(must be 33..65 bytes)");

        if (((eccPublicKey.size() - 1) % 2) != 0)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_ImportEccKey : invalid ECC public key structure");

        const unsigned short coordSize =
            static_cast<unsigned short>((eccPublicKey.size() - 1) / 2);

        payload.push_back(static_cast<unsigned char>((coordSize >> 8) & 0xFF));
        payload.push_back(static_cast<unsigned char>(coordSize & 0xFF));

        payload.insert(payload.end(), eccPublicKey.begin(), eccPublicKey.end());
    }
    if (payload.size() > sam::MAX_APDU_DATA_SIZE)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportEccKey : payload too large");
    }

    ByteVector apdu;
    apdu.reserve(5 + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(0x21);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(static_cast<unsigned char>(payload.size()));
    apdu.insert(apdu.end(), payload.begin(), payload.end());
    const ByteVector resp = transmit(apdu, true, true);

    if (resp.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportEccKey : response too short");

    const uint16_t sw = (resp[resp.size() - 2] << 8) | resp[resp.size() - 1];

    if (sw == 0x9000)
        return;

    if (sw == 0x6A80)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportEccKey : invalid key reference");

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_ImportEccKey : unexpected status word");
}

void SAMAV2ISO7816Commands::PKI_ImportEccCurve(unsigned char curveNo,
                                               unsigned char keyNoCCK,
                                               unsigned char keyNoVCCK,
                                               const ByteVector &eccCurve,
                                               bool settingsOnly)
{

    if (curveNo > 0x03)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_ImportEccCurve : curveNo out of range (0x00..0x03)");
    if (keyNoCCK != 0xFE && keyNoCCK != 0xFF && keyNoCCK > 0x7F)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "pkiImportEccCurve : invalid keyNoCCK");

    const unsigned char p1 = settingsOnly ? 0x01 : 0x00;

    ByteVector payload;
    payload.reserve(3 + eccCurve.size());
    payload.push_back(curveNo);
    payload.push_back(keyNoCCK);
    payload.push_back(keyNoVCCK);

    if (!settingsOnly)
    {
        if (eccCurve.empty())
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_ImportEccCurve : ECC curve data required when settingsOnly = false");

        if (eccCurve.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportEccCurve : ECC curve data too short");

        const unsigned char eccN = eccCurve[0];
        const unsigned char eccM = eccCurve[1];

        if (eccN < 0x10 || eccN > 0x20)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_ImportEccCurve : ECC_N out of range (0x10..0x20)");

        if (eccM < 0x10 || eccM > 0x20)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "PKI_ImportEccCurve : ECC_M out of range (0x10..0x20)");

        const size_t expectedSize =
            2 + (5 * static_cast<size_t>(eccN)) + static_cast<size_t>(eccM);

        if (eccCurve.size() != expectedSize)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportEccCurve : ECC curve length mismatch");

        payload.insert(payload.end(), eccCurve.begin(), eccCurve.end());
    }

    if (payload.size() > sam::MAX_APDU_DATA_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportEccCurve : APDU too large (Lc overflow)");

    ByteVector apdu;
    apdu.reserve(5 + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(0x22);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(static_cast<unsigned char>(payload.size()));
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector result = transmit(apdu, true, true);

    if (result.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportEccCurve : response too short");

    const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];

    if (sw == 0x9000)
        return;

    if (sw == 0x6A80)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportEccCurve: invalid curve number");

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_ImportEccCurve: unexpected status word");
}

ByteVector SAMAV2ISO7816Commands::PKI_ExportEccPublicKey(unsigned char keyNo)
{
    if (keyNo > 0x07)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_ExportEccPublicKey : keyNo out of range (0x00..0x07)");

    ByteVector apdu;
    apdu.reserve(5);
    apdu.push_back(d_cla);
    apdu.push_back(0x23);
    apdu.push_back(keyNo);
    apdu.push_back(0x00);
    apdu.push_back(0x00);

    const ByteVector result = transmit(apdu, true, true);

    if (result.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ExportEccPublicKey : response too short");

    const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];

    if (sw == 0x9000)
        return ByteVector(result.begin(), result.end() - 2);

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_ExportEccPublicKey: unexpected status word");
}

void SAMAV2ISO7816Commands::PKI_VerifyEccSignature(unsigned char keyNo,
                                                   unsigned char curveNo,
                                                   const ByteVector &message,
                                                   const ByteVector &signature)
{
    if (keyNo > 0x07)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_VerifyEccSignature : keyNo out of range (0x00..0x07)");
    if (curveNo > 0x03)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_VerifyEccSignature : curveNo out of range (0x00..0x03)");
    if (message.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifyEccSignature : message cannot be empty");
    if (message.size() > 0xFF)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifyEccSignature : message too large");
    if (signature.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifyEccSignature : signature cannot be empty");

    ByteVector payload;
    payload.reserve(3 + message.size() + signature.size());
    payload.push_back(keyNo);
    payload.push_back(curveNo);
    payload.push_back(static_cast<unsigned char>(message.size()));
    payload.insert(payload.end(), message.begin(), message.end());
    payload.insert(payload.end(), signature.begin(), signature.end());

    if (payload.size() > sam::MAX_APDU_DATA_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifyEccSignature : APDU too large (Lc overflow)");

    ByteVector apdu;
    apdu.reserve(5 + payload.size());
    apdu.push_back(0x80);
    apdu.push_back(0x20);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(static_cast<unsigned char>(payload.size()));
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector result = transmit(apdu, true, true);

    if (result.size() < 2)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifyEccSignature : response too short");
    }

    const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];

    if (sw == 0x9000)
        return;

    if (sw == 0x6A80)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifyEccSignature : invalid curve number");

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_VerifyEccSignature : unexpected status word");
}


}
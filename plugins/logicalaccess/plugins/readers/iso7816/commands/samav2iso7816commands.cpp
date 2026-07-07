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

sam::ApduResult SAMAV2ISO7816Commands::createfullProtectionCmd(const ByteVector &cmd, sam::ApduFormat format)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "createfullProtectionCmd Invalid command size.");

    constexpr size_t AES_BLOCK_SIZE         = 16;
    constexpr unsigned char ISO7816_PADDING = 0x80;
    ByteVector protectedCmd, encData;

    bool lc = false, le = false;
    if (format == sam::ApduFormat::Standard)
        getLcLe(cmd, lc, le);
    else
    {
        lc = true;
        le = (format == sam::ApduFormat::ExtendedWithLe);
    }

    auto cipher = std::make_shared<openssl::AESCipher>();

    if (!lc)
    {
        protectedCmd = cmd;
        protectedCmd.insert(protectedCmd.begin() + AV2_LC_POS, 0x08);
    }
    else
    {
        const size_t dataEnd = cmd.size() - (le ? 1u : 0u);
        protectedCmd.insert(protectedCmd.end(), cmd.begin(), cmd.begin() + AV2_HEADER_LENGTH);
        if (le)
            protectedCmd.push_back(cmd.back());
        ByteVector data(cmd.begin() + AV2_HEADER_LENGTH, cmd.begin() + dataEnd);
        if (data.size() % AES_BLOCK_SIZE != 0)
        {
            data.push_back(ISO7816_PADDING);
            data.resize(((data.size() + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE, 0x00);
        }
        /* generate IV because first encrypt */
        d_LastSessionIV = generateEncIV(true);
        const auto symkeySession = openssl::AESSymmetricKey::createFromData(d_sessionKey);
        const auto ivSession = openssl::AESInitializationVector::createFromData(d_LastSessionIV);
        cipher->cipher(data, encData, symkeySession, ivSession, false);
        protectedCmd.insert(protectedCmd.begin() + AV2_HEADER_LENGTH, encData.begin(), encData.end());
        protectedCmd[AV2_LC_POS] =
            (encData.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC) ? 0x00 : static_cast<unsigned char>(encData.size() + 8);
    }

    /* Set counter */
    ByteVector cmdCtr;
    cmdCtr.reserve(4);
    BufferHelper::setUInt32(cmdCtr, d_cmdCtr);
    std::reverse(cmdCtr.begin(), cmdCtr.end());
    protectedCmd.insert(protectedCmd.begin() + 2, cmdCtr.begin(), cmdCtr.end());

    const size_t blockReady = (protectedCmd.size() / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    if (blockReady >= AES_BLOCK_SIZE)
    {
        /* Creater our cipher buffer and keep last block */
        const auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_macSessionKey);
        const auto ivMac = openssl::AESInitializationVector::createFromData(d_lastMacIV);
        ByteVector macInput(protectedCmd.begin(), protectedCmd.begin() + blockReady);
        ByteVector macCiphertext(macInput.size());
        protectedCmd.erase(protectedCmd.begin(), protectedCmd.begin() + blockReady);
        cipher->cipher(macInput, macCiphertext, symkeyMac, ivMac, false);
        EXCEPTION_ASSERT_WITH_LOG(macCiphertext.size() >= AES_BLOCK_SIZE, LibLogicalAccessException, "Cipher output error.");
        d_lastMacIV.assign(macCiphertext.end() - AES_BLOCK_SIZE, macCiphertext.end());
    }
    ByteVector macFull = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, protectedCmd, d_lastMacIV, AES_BLOCK_SIZE);
    truncateMacBuffer(macFull);
    return {encData, macFull, le};
}

void SAMAV2ISO7816Commands::getLcLe(const ByteVector &cmd, bool &lc, bool &le)
{
    const size_t cmdSize = cmd.size();

    if (cmdSize < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "getLcLe cmd param is too small.");

    lc = false;
    le = false;

    // [CLA INS P1 P2]
    if (cmdSize == AV2_LC_POS)
        return;
    // [CLA INS P1 P2 LE]
    if (cmdSize == AV2_HEADER_LENGTH)
    {
        le = true;
        return;
    }
    const unsigned char lcByte = cmd[AV2_LC_POS];
    // [CLA INS P1 P2 LC DATA]
    if (cmdSize == static_cast<size_t>(lcByte) + AV2_HEADER_LENGTH)
    {
        lc = true;
        return;
    }
    // [CLA INS P1 P2 LC DATA LE]
    if (cmdSize == static_cast<size_t>(lcByte) + AV2_HEADER_LENGTH_WITH_LE)
    {
        lc = true;
        le = true;
        return;
    }
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getLcLe invalid command structure (size mismatch).");
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

    /* Set counter */
    BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
    std::reverse(cmdCtrVector.begin(), cmdCtrVector.end());
    myMac.insert(myMac.end(), cmdCtrVector.begin(), cmdCtrVector.end());

    if (response.size() != MAC_SIZE + STATUS_SIZE)
    {
        /* We have data Creater our cipher buffer and keep last block */
        auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_macSessionKey);
        auto ivMac     = openssl::AESInitializationVector::createFromData(d_lastMacIV);
        myMac.insert(myMac.end(), response.begin(), response.end() - MAC_SIZE - STATUS_SIZE);
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

// TODO update this function
ByteVector SAMAV2ISO7816Commands::createMacProtectionCmd(const ByteVector &cmd)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "createMacProtectionCmd Invalid command size.");
    constexpr unsigned char MAC_SIZE = 8;
    bool lc = false, le = false;
    bool lcvalue = 0; //TODO placeholder
    getLcLe(cmd, lc, le);
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
        EXCEPTION_ASSERT_WITH_LOG(tmp.size() >= 16, LibLogicalAccessException, "Cipher output too small.");
        d_lastMacIV.assign(tmp.end() - 16, tmp.end());
    }

    ByteVector encProtectedCmd = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, protectedCmd, d_lastMacIV, 16);
    truncateMacBuffer(encProtectedCmd);
    const size_t dataSize = lc ? lcvalue : 0;
    finalCmd.insert(finalCmd.begin() + AV2_HEADER_LENGTH + dataSize,
                    encProtectedCmd.begin(), encProtectedCmd.begin() + MAC_SIZE);
    return finalCmd;
}

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

//TODO merge (for single transmit func)
ByteVector SAMAV2ISO7816Commands::transmitSecureChained(const ByteVector &cmd, sam::ApduFormat format, const sam::ChainingLayout &layout)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid APDU : too short");

    try
    {
        auto protection = createfullProtectionCmd(cmd, format);
        auto frames = createChainedApduFrames(cmd, protection.encData, protection.mac, layout, protection.hasLe);
        ByteVector response = sendChainedFrames(frames, cmd[1]);
        return finalizeSecureResponse(std::move(response));
    }
    catch (const std::exception &e)
    {
        secureZero(d_sessionKey);
        secureZero(d_macSessionKey);
        resetIVs();
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, std::string("SAM transmit failed : ") + e.what());
    }
}

std::vector<ByteVector> SAMAV2ISO7816Commands::createChainedApduFrames(const ByteVector &cmd, const ByteVector &encData,
    const ByteVector &mac, const sam::ChainingLayout &layout, bool le)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid APDU : too short");

    constexpr size_t MAX_CHUNK_SIZE = sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC;
    constexpr size_t MAC_SIZE       = 8;
    constexpr unsigned char PX_MORE = 0xAF;
    constexpr unsigned char PX_LAST = 0x00;

    const unsigned char cla = cmd[0];
    const unsigned char ins = cmd[1];
    const unsigned char p1  = cmd[2];
    const unsigned char p2  = cmd[3];

    if (mac.size() < MAC_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC too short for final frame");

    std::vector<ByteVector> frames;
    const size_t frameCount = (encData.size() + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE;
    frames.reserve(frameCount);
    auto buildFrame = [&](size_t offset, size_t chunkSize, bool isLastChunk) -> ByteVector
    {
        ByteVector frame;
        frame.reserve(AV2_HEADER_LENGTH + chunkSize + (isLastChunk ? MAC_SIZE : 0));
        frame.push_back(cla);
        frame.push_back(ins);
        frame.push_back(p1);
        frame.push_back(p2);
        frame.push_back(0x00);
        auto it = encData.begin() + offset;
        frame.insert(frame.end(), it, it + chunkSize);
        if (isLastChunk)
            frame.insert(frame.end(), mac.begin(), mac.begin() + MAC_SIZE);
        frame[layout.lastFrameIndex] = static_cast<unsigned char>(isLastChunk ? PX_LAST : PX_MORE);
        if (offset > 0)
            frame[layout.modeIndex] = 0x00;
        const size_t lc = frame.size() - AV2_HEADER_LENGTH;
        if (lc > sam::MAX_APDU_DATA_SIZE)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "LC overflow in APDU frame");
        frame[AV2_LC_POS] = static_cast<unsigned char>(lc);
        return frame;
    };

    size_t offset = 0;
    while (offset < encData.size())
    {
        const size_t remaining = encData.size() - offset;
        const size_t chunkSize = (std::min)(MAX_CHUNK_SIZE, remaining);
        const bool isFirstChunk = (offset == 0);
        const bool isLastChunk = (offset + chunkSize >= encData.size());
        frames.emplace_back(buildFrame(offset, chunkSize, isLastChunk));
        offset += chunkSize;
    }

    if (le && !frames.empty())
        frames.back().push_back(cmd.back());

    return frames;
}

ByteVector SAMAV2ISO7816Commands::sendChainedFrames(const std::vector<ByteVector> &frames, unsigned char ins)
{
    constexpr unsigned char SW1_SUCCESS = 0x90;
    constexpr unsigned char SW2_MORE    = 0xAF;

    auto adapter = getISO7816ReaderCardAdapter();

    ByteVector response;

    unsigned char sw1 = 0;
    unsigned char sw2 = 0;
    for (size_t i = 0; i < frames.size(); ++i)
    {
        auto r = adapter->sendCommand(frames[i]);
        if (r.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid response");
        sw1 = r[r.size() - 2];
        sw2 = r[r.size() - 1];
        const bool lastFrame = (i + 1 == frames.size());
        response.insert(response.end(), r.begin(), r.end() - 2);
        if (!lastFrame && !(sw1 == SW1_SUCCESS && sw2 == SW2_MORE))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Expected 90AF for intermediate frame");
    }
    return receiveChainedResponse(ins, std::move(response), sw1, sw2);
}

ByteVector SAMAV2ISO7816Commands::receiveChainedResponse(unsigned char ins, ByteVector response, unsigned char sw1, unsigned char sw2)
{
    constexpr unsigned char SW1_SUCCESS = 0x90;
    constexpr unsigned char SW2_MORE    = 0xAF;
    constexpr unsigned char SW2_OK      = 0x00;

    auto adapter = getISO7816ReaderCardAdapter();

    const ByteVector continueApdu = {d_cla, ins, 0x00, 0x00, 0x00};

    while (sw1 == SW1_SUCCESS && sw2 == SW2_MORE)
    {
        auto r = adapter->sendCommand(continueApdu);

        if (r.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid AF response");

        sw1 = r[r.size() - 2];
        sw2 = r[r.size() - 1];

        response.insert(response.end(), r.begin(), r.end() - 2);

        if (sw1 != SW1_SUCCESS)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Unexpected status during response chaining");

        if (sw2 == SW2_MORE)
            continue;

        if (sw2 == SW2_OK)
            break;

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Unexpected status during response chaining");
    }
    if (!(sw1 == SW1_SUCCESS && sw2 == SW2_OK))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Expected final 9000");
    response.push_back(sw1);
    response.push_back(sw2);
    return response;
}

ByteVector SAMAV2ISO7816Commands::finalizeSecureResponse(ByteVector response)
{
    resetIVs();
    ++d_cmdCtr;
    response = verifyAndDecryptResponse(response);
    resetIVs(); // Necessary if function returns non-empty payload
    return response;
}

//TODO merge (for single transmit func)
ByteVector SAMAV2ISO7816Commands::transmitSecureResponseChained(const ByteVector &cmd, const sam::ChainingLayout &layout)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid APDU : too short");

    try
    {
        auto protection = createfullProtectionCmd(cmd);
        ByteVector response = sendChainedRespApdu(cmd, protection.encData, protection.mac, layout, protection.hasLe);
        return finalizeSecureResponse(std::move(response));
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

ByteVector SAMAV2ISO7816Commands::sendChainedRespApdu(
    const ByteVector &cmd, const ByteVector &encData, const ByteVector &mac, const sam::ChainingLayout &layout, bool le)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid APDU : too short");
    
    auto adapter = getISO7816ReaderCardAdapter();

    ByteVector first;
    first.push_back(cmd[0]);
    first.push_back(cmd[1]);
    first.push_back(cmd[2]);
    first.push_back(cmd[3]);
    first.push_back(0x00);
    first.insert(first.end(), encData.begin(), encData.begin() + encData.size());
    first.insert(first.end(), mac.begin(), mac.begin() + 8);
    first[AV2_LC_POS] = static_cast<unsigned char>(first.size() - AV2_HEADER_LENGTH);
    if (le)
        first.push_back(cmd.back());
    ByteVector repeat;
    repeat.push_back(cmd[0]);
    repeat.push_back(cmd[1]);
    repeat.push_back(0x00);
    repeat.push_back(0x00);
    repeat.push_back(0x00);

    ByteVector response;
    bool firstFrame = true;

    while (true)
    {
        ByteVector result;
        if (firstFrame)
            result = adapter->sendCommand(first);
        else
            result = adapter->sendCommand(repeat);
        if (result.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Invalid response length.");
        uint8_t sw1 = result[result.size() - 2];
        uint8_t sw2 = result[result.size() - 1];

        bool isFinal = (sw2 == 0x00);
        bool isMore  = (sw2 == 0xAF);

        if (!(sw1 == 0x90 && (isMore || isFinal)))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Invalid chaining status word");

        if (isFinal)
        {
            response.insert(response.end(), result.begin(), result.end());
            return response;
        }
        response.insert(response.end(), result.begin(), result.end() - 2);
        firstFrame = false;
    }
    return response;
}

//TODO merge (for single transmit func)
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
        /*if (protect == sam::HostMode::MAC) // TODO
        {
            protectedCmd = createMacProtectionCmd(cmd);
            ++d_cmdCtr;
        }*/
        if (first || !s_mode)
            cmd = createfullProtectionCmd(cmd).encData;
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
    unsigned char keyNoVCEK, unsigned char keyNoRef, const sam::AEKVAEK &accessKeys,
    unsigned short nLen, const ByteVector &pki_e,
    bool includeAccess)
{
    if (keyNo > 0x01)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateKeyPair : invalid keyNo");
    if (nLen < 0x40 || nLen > 0x100 || (nLen % 8) != 0)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
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
        if (!accessKeys)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "AEK/VAEK required but null");
    }

    const unsigned char p1 = (provideE ? 0x01 : 0x00) | (includeAccess ? 0x02 : 0x00);

    ByteVector payload;
    payload.reserve(includeAccess ? 12 + pki_e.size() : 10 + pki_e.size());
    payload.push_back(keyNo);
    sam::appendUInt16BE(payload, effectiveConfig);
    payload.push_back(keyNoCEK);
    payload.push_back(keyNoVCEK);
    payload.push_back(keyNoRef);
    if (includeAccess)
    {
        payload.push_back(accessKeys.keyNoAEK);
        payload.push_back(accessKeys.keyVAEK);
    }
    sam::appendUInt16BE(payload, nLen);
    sam::appendUInt16BE(payload, eLen);
    if (provideE)
        payload.insert(payload.end(), pki_e.begin(), pki_e.end());

    ByteVector apdu;
    apdu.push_back(d_cla);
    apdu.push_back(0x15);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    if (payload.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC)
        apdu.push_back(0x00);
    else
        apdu.push_back(static_cast<unsigned char>(payload.size()));
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector resp = transmitSecureChained(apdu, sam::ApduFormat::Extended);
    if (resp.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateKeyPair : invalid response length");

    const uint16_t sw = sam::parseStatusWord(resp);
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
    const sam::AEKVAEK &accessKeys, bool includeAccess, bool updateSettingsOnly)
{
    const bool hasPrivate = !pki_p.empty();

    if (keyNo > (hasPrivate ? 0x01 : 0x02))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportKey : keyNo out of range.");

    const size_t nLen = pki_n.size();
    const size_t eLen = pki_e.size();

    if (!updateSettingsOnly)
    {
        if (pki_n.empty() || pki_e.empty())
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : missing RSA components.");

        if (nLen < 0x40 || nLen > 0x100 || (nLen % 8) != 0)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : invalid modulus length.");

        if (nLen < 4 || (pki_n[0] == 0x00 && pki_n[1] == 0x00 &&
                         pki_n[2] == 0x00 && pki_n[3] == 0x00))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : modulus MSW must not be zero.");

        if (eLen < 0x04 || eLen > 0x100 || (eLen % 4) != 0 || eLen > nLen)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : invalid exponent length.");

        if ((pki_e.back() & 0x01) == 0)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : exponent must be odd.");

        if (pki_n[0] == 0x00)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : invalid modulus MSB.");
    }

    if (hasPrivate)
    {
        if (pki_dP.size() != pki_p.size())
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : dP length must equal p length.");

        if (pki_dQ.size() != pki_q.size())
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : dQ length must equal q length.");

        if (pki_ipq.size() != pki_q.size())
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : ipq length must equal q length.");

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

        if (!accessKeys)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportKey : AEK/VAEK required.");
    }

    const unsigned char p1 = (updateSettingsOnly ? 0x01 : 0x00) | (includeAccess ? 0x02 : 0x00);

    ByteVector payload;
    payload.push_back(keyNo);
    sam::appendUInt16BE(payload, effectiveConfig);
    payload.push_back(keyNoCEK);
    payload.push_back(keyNoVCEK);
    payload.push_back(refNoKUC);
    if (includeAccess)
    {
        payload.push_back(accessKeys.keyNoAEK);
        payload.push_back(accessKeys.keyVAEK);
    }
    if (!updateSettingsOnly)
    {
        sam::appendUInt16BE(payload, static_cast<uint16_t>(nLen));
        sam::appendUInt16BE(payload, static_cast<uint16_t>(eLen));
        if (hasPrivate)
        {
            if (pki_p.size() > 0xFFFF || pki_q.size() > 0xFFFF)
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "PKI_ImportKey : invalid CRT length overflow.");
            sam::appendUInt16BE(payload, static_cast<uint16_t>(pki_p.size()));
            sam::appendUInt16BE(payload, static_cast<uint16_t>(pki_q.size()));
        }
        payload.insert(payload.end(), pki_n.begin(), pki_n.end());
        payload.insert(payload.end(), pki_e.begin(), pki_e.end());
        if (hasPrivate)
        {
            payload.insert(payload.end(), pki_p.begin(), pki_p.end());
            payload.insert(payload.end(), pki_q.begin(), pki_q.end());
            payload.insert(payload.end(), pki_dP.begin(), pki_dP.end());
            payload.insert(payload.end(), pki_dQ.begin(), pki_dQ.end());
            payload.insert(payload.end(), pki_ipq.begin(), pki_ipq.end());
        }
    }

    ByteVector apdu;
    apdu.push_back(d_cla);
    apdu.push_back(0x19);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(payload.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC ? 0x00 : static_cast<unsigned char>(payload.size()));
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector resp = transmitSecureChained(apdu, sam::ApduFormat::Extended);
    if (resp.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportKey : invalid response length");
    const uint16_t sw = sam::parseStatusWord(resp);

    if (sw == 0x9000)
        return;
    if (sw == 0x6A80)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportKey : Key ref out of range.");

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_ImportKey : unexpected status word.");
}

ByteVector SAMAV2ISO7816Commands::PKI_ExportPrivateKey(unsigned char keyNo,
                                                       bool returnAEK)
{
    //constexpr size_t MIN_RESPONSE_SIZE = 3;
    //constexpr size_t MIN_KEY_DATA_SIZE = 13;

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

    ByteVector resp = transmitSecureResponseChained(apdu);
    if (resp.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ExportPrivateKey : invalid response length");
    const uint16_t sw = sam::parseStatusWord(resp);
    if (sw == 0x9000)
    {
        resp.resize(resp.size() - 2);
        return resp;
    }
    if (sw == 0x6986)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_ExportPrivateKey : private key export not allowed.");

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_ExportPrivateKey : unexpected status word.");
}

ByteVector SAMAV2ISO7816Commands::PKI_ExportPublicKey(unsigned char keyNo, bool returnAEK)
{
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

    ByteVector resp = transmitSecureResponseChained(apdu);
    if (resp.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ExportPublicKey : invalid response size.");
    const uint16_t sw = sam::parseStatusWord(resp);
    if (sw == 0x9000)
    {
        resp.resize(resp.size() - 2);
        return resp;
    }
    if (sw == 0x6986)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "PKI_ExportPublicKey : export not allowed.");

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "PKI_ExportPublicKey : unexpected status word.");
}

ByteVector SAMAV2ISO7816Commands::buildPlaintext(uint16_t changeCtr, const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries)
{
    ByteVector pt;

    pt.push_back((changeCtr >> 8) & 0xFF);
    pt.push_back(changeCtr & 0xFF);

    for (const auto &entry : entries)
    {
        if (!entry)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Null SAMBasicKeyEntry");

        //TODO : SAM must support both AV2 compatible (61 bytes) and AV3 (64 bytes) key entries
        auto *avEntry = dynamic_cast<SAMKeyEntry<KeyEntryAV2Information, SETAV2> *>(entry.get());
        if (!avEntry)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Entry is not a SAMKeyEntry AV2 type");

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

const EVP_MD *SAMAV2ISO7816Commands::getHash(sam::HashAlgo hashAlgo)
{
    switch (hashAlgo)
    {
    case sam::HashAlgo::SHA1: return EVP_sha1();
    case sam::HashAlgo::SHA224: return EVP_sha224();
    case sam::HashAlgo::SHA256: return EVP_sha256();
    default: throw std::runtime_error("Invalid hash");
    }
}

void SAMAV2ISO7816Commands::buildCryptogram(
    EVP_PKEY *encKey, EVP_PKEY *signKey, uint8_t keyNoEnc, uint8_t keyNoSign,
    uint16_t changeCtr, const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries,
    uint8_t hashAlgo, ByteVector &encFrame, ByteVector &signature)
{
    if (!sam::isSupportedHashAlgo(hashAlgo))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : unsupported or RFU hash algorithm.");

    const EVP_MD *md = getHash(static_cast<sam::HashAlgo>(hashAlgo));

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
    EVP_PKEY &encKey, EVP_PKEY &signKey, unsigned char keyNoEnc, unsigned char keyNoSign,
    bool requestAck, unsigned char keyNoAck, unsigned char hashAlgo,
    const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries, uint16_t changeCounter)
{
    if (entries.empty() || entries.size() > 3)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : invalid number of entries");

    EVP_PKEY *enc = &encKey;  //To avoid pointer as arg
    EVP_PKEY *sig = &signKey; //To avoid pointer as arg

    if (!enc || !sig)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid ENC or SIGN key");

    ByteVector encFrame;
    ByteVector signature;

    buildCryptogram(enc, sig, keyNoEnc, keyNoSign, changeCounter, entries,
                    hashAlgo, encFrame, signature);

    return PKI_UpdateKeyEntries(keyNoEnc, keyNoSign, requestAck, keyNoAck, hashAlgo,
                                static_cast<unsigned char>(entries.size()), encFrame,
                                signature);
}

ByteVector SAMAV2ISO7816Commands::PKI_UpdateKeyEntries(
    unsigned char keyNoEnc, unsigned char keyNoSign, bool requestAck, unsigned char keyNoAck,
    unsigned char hashAlgo, unsigned char nbKeyEntries, const ByteVector &encKeyFrame,
    const ByteVector &signature)
{
    if (hashAlgo > 0x03)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : invalid hashAlgo (must be 0...3)");

    if (nbKeyEntries == 0 || nbKeyEntries > 3)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : invalid nbKeys (must be 1...3)");

    if (keyNoEnc > 0x01)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : invalid keyNoEnc.");
    
    if (keyNoSign > 0x02)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : invalid keyNoSign.");
    
    if (requestAck && keyNoAck > 0x01)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : invalid keyNoAck.");

    if (encKeyFrame.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : encKeyFrame cannot be empty");

    if (signature.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "PKI_UpdateKeyEntries : signature cannot be empty");

    unsigned char p1 = (hashAlgo & 0x03) | ((nbKeyEntries & 0x03) << 2);

    ByteVector payload;
    payload.push_back(keyNoEnc);
    payload.push_back(keyNoSign);
    if (requestAck)
        payload.push_back(keyNoAck);
    payload.insert(payload.end(), encKeyFrame.begin(), encKeyFrame.end());
    payload.insert(payload.end(), signature.begin(), signature.end());

    size_t offset = 0;
    bool success  = false;
    ByteVector result;

    auto buildAPDU = [&](const ByteVector &data) -> ByteVector
    {
        ByteVector apdu;
        apdu.push_back(d_cla);
        apdu.push_back(0x1D);
        apdu.push_back(p1);
        apdu.push_back(0x00);
        if (data.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC)
            apdu.push_back(0x00);
        else
            apdu.push_back(static_cast<unsigned char>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        if (requestAck)
            apdu.push_back(0x00);
        return apdu;
    };

    const ByteVector fullPayload = buildAPDU(payload);
    ByteVector resp = transmitSecureChained(fullPayload, sam::ApduFormat::ExtendedWithLe);
    if (resp.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : response too short");

    const uint16_t sw = sam::parseStatusWord(resp);
    resp.resize(resp.size() - 2);
    if (sw == 0x9000)
        return resp;
    if (sw == 0x6A80)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : incorrect ChangeCtr");
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_UpdateKeyEntries : unexpected status word");
}

ByteVector SAMAV2ISO7816Commands::PKI_EncipherKeyEntries(
    unsigned char hashAlgo, unsigned char keyNoEnc, unsigned char keyNoSign,
    unsigned char keyNoDec, unsigned char keyNoVerif, unsigned short persoCtr,
    const std::vector<std::pair<unsigned char, unsigned char>> &keyEntries,
    const ByteVector &divInput)
{
    if (!sam::isSupportedHashAlgo(hashAlgo))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : unsupported or RFU hash algorithm.");

    if (keyEntries.empty() || keyEntries.size() > 3)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : 1 to 3 key entries allowed");

    if (!divInput.empty() && divInput.size() > 31)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : DivInput must be 1...31 bytes");

    if (keyNoEnc > 0x02)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : invalid keyNoEnc.");

    if (keyNoSign > 0x01)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : invalid keyNoSign.");

    if (keyNoDec > 0x01)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : invalid keyNoDec.");

    if (keyNoVerif > 0x02)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : invalid keyNoVerif.");

    for (const auto &k : keyEntries)
    {
        if (k.first > 0x7F || k.second > 0x7F)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_EncipherKeyEntries : invalid key entry pair");
    }

    const unsigned char p1 = (hashAlgo & 0x03) |
                             (static_cast<unsigned char>(keyEntries.size()) << 2) |
                             (!divInput.empty() ? 0x10 : 0x00);

    ByteVector payload;
    payload.reserve(6 + keyEntries.size() * 2 + divInput.size());
    payload.push_back(keyNoEnc);
    payload.push_back(keyNoSign);
    payload.push_back(keyNoDec);
    payload.push_back(keyNoVerif);
    sam::appendUInt16BE(payload, persoCtr);
    for (const auto &entry : keyEntries)
    {
        payload.push_back(entry.first);
        payload.push_back(entry.second);
    }
    if (!divInput.empty())
        payload.insert(payload.end(), divInput.begin(), divInput.end());

    ByteVector apdu;
    apdu.push_back(d_cla);
    apdu.push_back(0x12);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    if (payload.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC)
        apdu.push_back(0x00);
    else
        apdu.push_back(static_cast<unsigned char>(payload.size()));
    apdu.insert(apdu.end(), payload.begin(), payload.end());
    apdu.push_back(0x00);

    ByteVector resp = transmitSecureChained(apdu, sam::ApduFormat::ExtendedWithLe);
    if (resp.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : response too short.");

    const uint16_t sw = sam::parseStatusWord(resp);

    if (sw == 0x9000)
    {
        resp.resize(resp.size() - 2);
        return resp;
    }

    if (sw == 0x6A80)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherKeyEntries : at least one selected PKI key is not "
                                 "reserved for personalization.");

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_EncipherKeyEntries : unexpected status word.");
}

ByteVector SAMAV2ISO7816Commands::PKI_GenerateHash(unsigned char hashAlgo,
                                                   const ByteVector &message)
{
    if (!sam::isSupportedHashAlgo(hashAlgo))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateHash : unsupported or RFU hash algorithm.");

    if (message.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateHash : message cannot be empty.");

    const uint32_t messageLen = static_cast<uint32_t>(message.size());

    ByteVector apdu;
    apdu.reserve(5 + 4 + message.size() + 1);
    apdu.push_back(d_cla);
    apdu.push_back(0x17);
    apdu.push_back(hashAlgo);
    apdu.push_back(0x00);
    const size_t lc = 4 + message.size();
    if (lc > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC)
        apdu.push_back(0x00);
    else
        apdu.push_back(static_cast<unsigned char>(lc));
    sam::appendUInt32BE(apdu, messageLen);
    apdu.insert(apdu.end(), message.begin(), message.end());
    apdu.push_back(0x00);

    ByteVector response = transmitSecureChained(apdu, sam::ApduFormat::ExtendedWithLe);

    if (response.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateHash : invalid response size.");

    if (response.size() != sam::expectedHashSize(hashAlgo) + 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateHash : invalid hash length.");

    const uint16_t sw = sam::parseStatusWord(response);
    if (sw != 0x9000)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateHash : unexpected status word.");

    response.resize(response.size() - 2);
    return response;
}

void SAMAV2ISO7816Commands::PKI_GenerateSignature(unsigned char hashAlgo,
                                                  unsigned char keyNoSign,
                                                  const ByteVector &hash)
{
    if (!sam::isSupportedHashAlgo(hashAlgo))
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_GenerateSignature : unsupported or RFU hash algorithm.");

    if (keyNoSign > 0x01)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateSignature : invalid key number.");

    const size_t expectedSize = sam::expectedHashSize(hashAlgo);

    if (hash.size() != expectedSize)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_GenerateSignature : invalid hash size for selected algorithm.");

    ByteVector apdu;
    apdu.reserve(5 + 1 + hash.size());
    apdu.push_back(d_cla);
    apdu.push_back(0x16);
    apdu.push_back(hashAlgo);
    apdu.push_back(0x00);

    const size_t lc = 1 + hash.size();
    if (lc > 1 + expectedSize)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateSignature : payload too large.");

    apdu.push_back(static_cast<unsigned char>(lc));
    apdu.push_back(keyNoSign);
    apdu.insert(apdu.end(), hash.begin(), hash.end());

    const ByteVector response = transmit(apdu, true, true);

    if (response.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateSignature : invalid response size.");

    const uint16_t sw = sam::parseStatusWord(response);

    if (sw != 0x9000)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_GenerateSignature : unexpected status word.");

    return;
}

ByteVector SAMAV2ISO7816Commands::PKI_SendSignature()
{
    constexpr size_t MIN_SIG_SIZE = 8;
    constexpr size_t MAX_SIG_SIZE = 256;
    ByteVector signature = transmit({d_cla, 0x1A, 0x00, 0x00, 0x00}, true, true);

    if (signature.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_SendSignature : invalid response size.");

    if (sam::parseStatusWord(signature) != 0x9000)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_SendSignature : unexpected status word.");

    signature.resize(signature.size() - 2);

    if (signature.size() < MIN_SIG_SIZE || signature.size() > MAX_SIG_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_SendSignature : invalid signature size.");

    return signature;
}

void SAMAV2ISO7816Commands::PKI_VerifySignature(unsigned char hashAlgo,
                                                unsigned char keyNoVerif,
                                                const ByteVector &hash,
                                                const ByteVector &signature)
{
    if (!sam::isSupportedHashAlgo(hashAlgo))
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_VerifySignature : unsupported or RFU hash algorithm.");

    if (keyNoVerif > 0x02)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_VerifySignature : invalid verification key number.");

    if (hash.size() != sam::expectedHashSize(hashAlgo))
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_VerifySignature : invalid hash size for selected algorithm.");

    if (signature.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifySignature : signature cannot be empty.");

    ByteVector payload;
    payload.reserve(1 + hash.size() + signature.size());
    payload.push_back(keyNoVerif);
    payload.insert(payload.end(), hash.begin(), hash.end());
    payload.insert(payload.end(), signature.begin(), signature.end());

    ByteVector apdu;
    apdu.reserve(5 + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(0x1B);
    apdu.push_back(hashAlgo);
    apdu.push_back(0x00);
    if (payload.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC)
        apdu.push_back(0x00);
    else
        apdu.push_back(static_cast<unsigned char>(payload.size()));
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = transmitSecureChained(apdu, sam::ApduFormat::Extended);

    if (response.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifySignature : invalid response size.");

    if (sam::parseStatusWord(response) != 0x9000)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifySignature : unexpected status word.");

    return;
}

ByteVector SAMAV2ISO7816Commands::PKI_EncipherData(unsigned char hashAlgo,
                                                   unsigned char keyNoEnc,
                                                   const ByteVector &plainData)
{
    if (!sam::isSupportedHashAlgo(hashAlgo))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherData : unsupported or RFU hash algorithm.");

    if (keyNoEnc > 0x02)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherData : invalid encryption key number.");

    if (plainData.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherData : empty plaintext.");

    const unsigned char p1 = (hashAlgo & 0x03);
    const size_t lc = 1 + plainData.size();

    ByteVector apdu;
    apdu.reserve(5 + lc + 1);
    apdu.push_back(d_cla);
    apdu.push_back(0x13);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(static_cast<unsigned char>(lc));
    apdu.push_back(keyNoEnc);
    apdu.insert(apdu.end(), plainData.begin(), plainData.end());
    apdu.push_back(0x00);

    ByteVector encData = transmitSecureChained(apdu);

    if (encData.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherData : response too short");

    encData.resize(encData.size() - 2);

    if (sam::parseStatusWord(encData) != 0x9000)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherData : unexpected status word.");

    if (encData.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_EncipherData : empty encrypted data");

    return encData;
}

ByteVector SAMAV2ISO7816Commands::PKI_DecipherData(unsigned char hashAlgo,
                                                   unsigned char keyNoDec,
                                                   const ByteVector &encData)
{
    if (!sam::isSupportedHashAlgo(hashAlgo))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_DecipherData : unsupported or RFU hash algorithm.");

    if (keyNoDec > 0x01)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_DecipherData : invalid key number.");

    if (encData.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_DecipherData : empty encrypted data");

    const unsigned char p1 = (hashAlgo & 0x03);
    const size_t lc = 1 + encData.size();

    ByteVector apdu;
    apdu.reserve(5 + lc + 1);
    apdu.push_back(d_cla);
    apdu.push_back(0x14);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    if (lc > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC)
        apdu.push_back(0x00);
    else
        apdu.push_back(static_cast<unsigned char>(lc));
    apdu.push_back(keyNoDec);
    apdu.insert(apdu.end(), encData.begin(), encData.end());
    apdu.push_back(0x00);

    ByteVector plainData = transmitSecureChained(apdu, sam::ApduFormat::ExtendedWithLe);

    if (plainData.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_DecipherData : invalid response size.");

    if (sam::parseStatusWord(plainData) != 0x9000)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_DecipherData : unexpected status word.");

    plainData.resize(plainData.size() - 2);

    if (plainData.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_DecipherData : empty plaintext returned.");

    return plainData;
}

void SAMAV2ISO7816Commands::PKI_ImportEccKey(
    unsigned char keyNo, unsigned short eccSet, unsigned char keyNoCEK,
    unsigned char keyNoVCEK, unsigned char keyNoKUC, unsigned char keyNoAEK,
    unsigned char keyNoVAEK, const ByteVector &eccPublicKey, bool settingsOnly)
{
    if (keyNo > 0x07)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportEccKey : invalid keyNo (0...7)");

    constexpr size_t PKI_IMPORT_ECC_MAX_PAYLOAD = 0x4B;

    const unsigned char p1 = settingsOnly ? 0x01 : 0x00;

    ByteVector payload;
    payload.reserve(settingsOnly ? 8 : 10 + eccPublicKey.size());
    payload.push_back(keyNo);
    sam::appendUInt16BE(payload, eccSet);
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

        const size_t keySize = eccPublicKey.size();

        if (keySize < 33 || keySize > 65)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportEccKey : invalid ECC public key length "
                                     "(must be 33...65 bytes)");

        const unsigned short coordSize = static_cast<unsigned short>((keySize - 1) / 2);

        if (coordSize * 2 + 1 != keySize)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportEccKey : malformed ECC point structure");

        sam::appendUInt16BE(payload, coordSize);
        payload.insert(payload.end(), eccPublicKey.begin(), eccPublicKey.end());
    }

    if (payload.size() > PKI_IMPORT_ECC_MAX_PAYLOAD)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportEccKey : payload too large");

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

    const uint16_t sw = sam::parseStatusWord(resp);

    if (sw == 0x9000)
        return;

    if (sw == 0x6A80)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportEccKey : invalid key reference");

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_ImportEccKey : unexpected status word");
}

void SAMAV2ISO7816Commands::PKI_ImportEccCurve(unsigned char curveNo, unsigned char keyNoCCK,
                                               unsigned char keyNoVCCK,
                                               const ByteVector &eccCurve,
                                               bool settingsOnly)
{

    if (curveNo > 0x03)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "PKI_ImportEccCurve : curveNo out of range (0x00...0x03)");

    if (!(keyNoCCK == 0xFE || keyNoCCK == 0xFF || keyNoCCK <= 0x7F))
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
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                "PKI_ImportEccCurve : ECC curve data required when settingsOnly = false");

        if (eccCurve.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportEccCurve : ECC curve data too short");

        const unsigned char eccN = eccCurve[0];
        const unsigned char eccM = eccCurve[1];

        if (eccN < 0x10 || eccN > 0x20)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                "PKI_ImportEccCurve : ECC_N out of range (0x10...0x20)");

        if (eccM < 0x10 || eccM > 0x20)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                "PKI_ImportEccCurve : ECC_M out of range (0x10...0x20)");

        const size_t expectedSize = 2 + (5 * static_cast<size_t>(eccN)) + static_cast<size_t>(eccM);

        if (eccCurve.size() != expectedSize)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportEccCurve : ECC curve length mismatch");

        size_t offset = 2;

        auto checkBlock = [&](size_t len, const char *name)
        {
            if (offset + len > eccCurve.size())
                THROW_EXCEPTION_WITH_LOG(
                    LibLogicalAccessException,
                    std::string("PKI_ImportEccCurve: truncated field ") + name);
            offset += len;
        };

        checkBlock(eccN, "ECC_Prime");
        checkBlock(eccN, "ECC_A");
        checkBlock(eccN, "ECC_B");
        checkBlock(eccN, "ECC_Px");
        checkBlock(eccN, "ECC_Py");
        checkBlock(eccM, "ECC_Order");

        payload.insert(payload.end(), eccCurve.begin(), eccCurve.end());
    }

    if (payload.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC)
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

    const ByteVector response = transmit(apdu, true, true);

    if (response.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportEccCurve : response too short");

    const uint16_t sw = sam::parseStatusWord(response);

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
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "PKI_ExportEccPublicKey : keyNo out of range (0x00...0x07)");

    ByteVector apdu;
    apdu.reserve(5);
    apdu.push_back(d_cla);
    apdu.push_back(0x23);
    apdu.push_back(keyNo);
    apdu.push_back(0x00);
    apdu.push_back(0x00);

    ByteVector response = transmit(apdu, true, true);

    if (response.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ExportEccPublicKey : response too short");

    const uint16_t sw = sam::parseStatusWord(response);

    if (sam::parseStatusWord(response) != 0x9000)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ExportEccPublicKey: unexpected status word");

    response.resize(response.size() - 2);
    return response;
}

void SAMAV2ISO7816Commands::PKI_VerifyEccSignature(unsigned char keyNo,
                                                   unsigned char curveNo,
                                                   const ByteVector &message,
                                                   const ByteVector &signature)
{
    if (keyNo > 0x07)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "PKI_VerifyEccSignature : keyNo out of range (0x00...0x07)");
    if (curveNo > 0x03)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "PKI_VerifyEccSignature : curveNo out of range (0x00...0x03)");
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

    if (payload.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifyEccSignature : APDU too large (Lc overflow)");

    ByteVector apdu;
    apdu.reserve(5 + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(0x20);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(static_cast<unsigned char>(payload.size()));
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = transmit(apdu, true, true);

    if (response.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifyEccSignature : response too short");

    const uint16_t sw = sam::parseStatusWord(response);

    if (sw == 0x9000)
        return;

    if (sw == 0x6A80)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_VerifyEccSignature : invalid curve number");

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_VerifyEccSignature : unexpected status word");
}

}
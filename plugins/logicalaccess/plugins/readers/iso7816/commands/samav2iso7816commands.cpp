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
    if (sessionKeySize != AES_128_SIZE && sessionKeySize != AES_192_SIZE && sessionKeySize != AES_256_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            sam::errorMessage(__func__, "Invalid session key size."));

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
    if (sessionKeySize == AES_256_SIZE) /* AES 256 */
    {
        d_sessionKey.insert(d_sessionKey.end(), Keb.begin(), Keb.end());
        d_macSessionKey.insert(d_macSessionKey.end(), Kmb.begin(), Kmb.end());
    }
    else if (sessionKeySize == AES_192_SIZE) /* AES 192 */
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
                                 sam::errorMessage(__func__, "Only AES Key allowed."));

    constexpr size_t BLOCK_SIZE   = 16;
    constexpr size_t AES_128_SIZE = 16;
    constexpr size_t AES_192_SIZE = 24;
    constexpr size_t AES_256_SIZE = 32;
    const ByteVector keydata      = key->getData();
    const size_t keysize          = keydata.size();

    if (keysize != AES_128_SIZE && keysize != AES_192_SIZE && keysize != AES_256_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 sam::errorMessage(__func__, "Invalid AES key size."));

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

void SAMAV2ISO7816Commands::authenticateHost(std::shared_ptr<DESFireKey> key, unsigned char keyno)
{
    authenticateHost(key, keyno, sam::HostMode::FullProtect); // Host Mode: Full Protection
}

void SAMAV2ISO7816Commands::authenticateHost(std::shared_ptr<DESFireKey> key,
                                             unsigned char keyno, sam::HostMode hostmode)
{
    EXCEPTION_ASSERT_WITH_LOG(key != nullptr,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid key."));

    EXCEPTION_ASSERT_WITH_LOG(key->getKeyType() == DF_KEY_AES,
        LibLogicalAccessException, sam::errorMessage(__func__, "Only AES Key allowed."));

    constexpr unsigned char RND_SIZE = 12;
    const unsigned char mode         = sam::toByte(hostmode);
    const ByteVector emptyIV(sam::AES_BLOCK_SIZE, 0x00);
    auto adapter       = getISO7816ReaderCardAdapter();
    
    const ByteVector keycipher = key->getData();
    EXCEPTION_ASSERT_WITH_LOG(keycipher.size() == 16 || keycipher.size() == 24 || keycipher.size() == 32,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid AES key size."));

    /* Reset host authentication state. */
    d_hostMode = sam::HostMode::None;

    /* emptyIV and Clear Key */
    secureZero(d_sessionKey);
    secureZero(d_macSessionKey);
    d_lastMacIV     = emptyIV;
    d_LastSessionIV = emptyIV;

    const ByteVector data_p1 = {keyno, key->getKeyVersion(), mode};
    auto result = adapter->sendAPDUCommand(d_cla, sam::ins::host::AuthenticateHost,
        0x00, 0x00, static_cast<unsigned char>(data_p1.size()), data_p1, 0x00);

    EXCEPTION_ASSERT_WITH_LOG(result.getData().size() == 12 &&
        result.getSW1() == sam::sw::SuccessSW1 && result.getSW2() == sam::sw::MoreDataSW2,
        LibLogicalAccessException, sam::errorMessage(__func__, "P1 Failed."));

    d_macSessionKey = keycipher;
    auto cipher     = std::make_shared<openssl::AESCipher>();

    /* Create rnd2 for p3 - CMAC: rnd2 | Host Mode | ZeroPad */
    ByteVector rnd2 = result.getData();
    rnd2.push_back(mode);
    rnd2.resize(sam::AES_BLOCK_SIZE, 0x00); // ZeroPad

    ByteVector macHost = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, rnd2, d_lastMacIV, sam::AES_BLOCK_SIZE);
    truncateMacBuffer(macHost);

    ByteVector rnd1(RND_SIZE);
    EXCEPTION_ASSERT_WITH_LOG(RAND_bytes(rnd1.data(), static_cast<int>(rnd1.size())) == 1,
        LibLogicalAccessException, sam::errorMessage(__func__, "Cannot retrieve cryptographically strong bytes."));

    ByteVector data_p2;
    data_p2.reserve(sam::MAC_SIZE + rnd1.size());
    data_p2.insert(data_p2.end(), macHost.begin(), macHost.begin() + sam::MAC_SIZE);
    data_p2.insert(data_p2.end(), rnd1.begin(), rnd1.end());
    result = adapter->sendAPDUCommand(d_cla, sam::ins::host::AuthenticateHost,
        0x00, 0x00, static_cast<unsigned char>(data_p2.size()), data_p2, 0x00);
    EXCEPTION_ASSERT_WITH_LOG(result.getData().size() == 24 &&
        result.getSW1() == sam::sw::SuccessSW1 && result.getSW2() == sam::sw::MoreDataSW2,
        LibLogicalAccessException, sam::errorMessage(__func__, "P2 Failed."));

    /* Check CMAC - Create rnd1 for p3 - CMAC: rnd1 | P1 | other data */
    rnd1.insert(rnd1.end(), rnd2.begin() + RND_SIZE, rnd2.end()); // p2 data without rnd2
    macHost = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, rnd1, d_lastMacIV, sam::AES_BLOCK_SIZE);
    truncateMacBuffer(macHost);
    EXCEPTION_ASSERT_WITH_LOG(std::equal(macHost.begin(), macHost.begin() + sam::MAC_SIZE, result.getData().begin()),
        LibLogicalAccessException, sam::errorMessage(__func__, "P2 CMAC from SAM is wrong."));

    /* Create kxe - d_authKey */
    generateAuthEncKey(keycipher, rnd1, rnd2);
    // create rndA
    ByteVector rndA(sam::AES_BLOCK_SIZE);
    EXCEPTION_ASSERT_WITH_LOG(RAND_bytes(rndA.data(), static_cast<int>(rndA.size())) == 1,
        LibLogicalAccessException, sam::errorMessage(__func__, "Cannot retrieve cryptographically strong bytes."));

    // decipher rndB
    auto symkey = openssl::AESSymmetricKey::createFromData(d_authKey);
    auto iv     = openssl::AESInitializationVector::createFromData(d_lastMacIV);

    ByteVector encRndB(result.getData().begin() + sam::MAC_SIZE, result.getData().end());
    ByteVector dencRndB;
    cipher->decipher(encRndB, dencRndB, symkey, iv, false);

    // create rndB'
    ByteVector rndB1;
    rndB1.insert(rndB1.begin(), dencRndB.begin() + 2, dencRndB.begin() + dencRndB.size());
    rndB1.push_back(dencRndB[0]);
    rndB1.push_back(dencRndB[1]);

    ByteVector dataHost;
    dataHost.reserve(rndA.size() + rndB1.size());
    dataHost.insert(dataHost.end(), rndA.begin(), rndA.end());   // RndA
    dataHost.insert(dataHost.end(), rndB1.begin(), rndB1.end()); // RndB'

    iv = openssl::AESInitializationVector::createFromData(d_lastMacIV);
    ByteVector encHost;

    cipher->cipher(dataHost, encHost, symkey, iv, false);
    result = adapter->sendAPDUCommand(d_cla, sam::ins::host::AuthenticateHost,
        0x00, 0x00, static_cast<unsigned char>(encHost.size()), encHost, 0x00);
    EXCEPTION_ASSERT_WITH_LOG(result.getData().size() == sam::AES_BLOCK_SIZE &&
        result.getSW1() == sam::sw::SuccessSW1 && result.getSW2() == sam::sw::SuccessSW2,
        LibLogicalAccessException, sam::errorMessage(__func__, "P3 Failed."));

    ByteVector SAMrndA;
    iv = openssl::AESInitializationVector::createFromData(d_lastMacIV);
    cipher->decipher(result.getData(), SAMrndA, symkey, iv, false);
    SAMrndA.insert(SAMrndA.begin(), SAMrndA.end() - 2, SAMrndA.end());

    EXCEPTION_ASSERT_WITH_LOG(std::equal(SAMrndA.begin(), SAMrndA.begin() + sam::AES_BLOCK_SIZE, rndA.begin()),
        LibLogicalAccessException, sam::errorMessage(__func__, "P3 RndA from SAM is invalid."));

    generateSessionKey(rndA, dencRndB);
    d_cmdCtr = 0;
    d_hostMode = hostmode;

    secureZero(rnd1);
    secureZero(rnd2);
    secureZero(rndA);
    secureZero(dencRndB);
    secureZero(rndB1);
    secureZero(dataHost);
    secureZero(encHost);
    secureZero(encRndB);
    secureZero(SAMrndA);
    secureZero(macHost);
}

sam::ProtectedApdu SAMAV2ISO7816Commands::prepareProtectedApdu(const ByteVector &cmd, sam::ApduFormat format)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 sam::errorMessage(__func__, "Invalid command size."));

    ByteVector protectedCmd, encData;

    const auto apduInfo = getApduInfo(cmd, format);
    const bool encrypt = (d_hostMode == sam::HostMode::FullProtect);

    if (!apduInfo.hasLc)
    {
        protectedCmd = cmd;
        protectedCmd.insert(protectedCmd.begin() + AV2_LC_POS, sam::MAC_SIZE);
    }
    else
    {
        const size_t dataEnd = cmd.size() - (apduInfo.hasLe ? 1u : 0u);
        ByteVector data(cmd.begin() + AV2_HEADER_LENGTH, cmd.begin() + dataEnd);
        if (encrypt)
        {
            encData = encryptCommandData(data);
            protectedCmd.insert(protectedCmd.end(), cmd.begin(), cmd.begin() + AV2_HEADER_LENGTH);
            if (apduInfo.hasLe)
                protectedCmd.push_back(cmd.back());
            protectedCmd.insert(protectedCmd.begin() + AV2_HEADER_LENGTH, encData.begin(), encData.end());
            protectedCmd[AV2_LC_POS] =
                (encData.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC) ? 0x00 : static_cast<unsigned char>(encData.size() + sam::MAC_SIZE);
        }
        else
        {
            encData = data;
            protectedCmd = cmd;
            const size_t dataLen = data.size();
            protectedCmd[AV2_LC_POS] =
                (dataLen > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC) ? 0x00 : static_cast<unsigned char>(dataLen + sam::MAC_SIZE);
        }
    }

    /* Set counter */
    ByteVector cmdCtr;
    cmdCtr.reserve(4);
    BufferHelper::setUInt32(cmdCtr, d_cmdCtr);
    std::reverse(cmdCtr.begin(), cmdCtr.end());
    protectedCmd.insert(protectedCmd.begin() + 2, cmdCtr.begin(), cmdCtr.end());

    ByteVector macFull = computeCommandMac(protectedCmd);

    return {encData, macFull, apduInfo.hasLe};
}

ByteVector SAMAV2ISO7816Commands::computeCommandMac(ByteVector &protectedCmd)
{
    const auto cipher = std::make_shared<openssl::AESCipher>();
    const size_t blockReady = (protectedCmd.size() / sam::AES_BLOCK_SIZE) * sam::AES_BLOCK_SIZE;
    if (blockReady >= sam::AES_BLOCK_SIZE)
    {
        /* Encrypt complete blocks and preserve last MAC IV. */
        const auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_macSessionKey);
        const auto ivMac = openssl::AESInitializationVector::createFromData(d_lastMacIV);
        ByteVector macInput(protectedCmd.begin(), protectedCmd.begin() + blockReady);
        ByteVector macCiphertext(macInput.size());
        protectedCmd.erase(protectedCmd.begin(), protectedCmd.begin() + blockReady);
        cipher->cipher(macInput, macCiphertext, symkeyMac, ivMac, false);
        EXCEPTION_ASSERT_WITH_LOG(macCiphertext.size() >= sam::AES_BLOCK_SIZE, LibLogicalAccessException,
            sam::errorMessage(__func__, "Cipher output error."));
        d_lastMacIV.assign(macCiphertext.end() - sam::AES_BLOCK_SIZE, macCiphertext.end());
    }
    ByteVector mac = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, protectedCmd, d_lastMacIV, sam::AES_BLOCK_SIZE);
    truncateMacBuffer(mac);
    return mac;
}

ByteVector SAMAV2ISO7816Commands::encryptCommandData(const ByteVector &data)
{
    constexpr unsigned char ISO7816_PADDING = 0x80;

    ByteVector paddedData = data;
    if (paddedData.size() % sam::AES_BLOCK_SIZE != 0)
    {
        paddedData.push_back(ISO7816_PADDING);
        paddedData.resize(((paddedData.size() + sam::AES_BLOCK_SIZE - 1) / sam::AES_BLOCK_SIZE) * sam::AES_BLOCK_SIZE, 0x00);
    }
    openssl::AESCipher cipher;
    /* Generate session IV for command encryption */
    d_LastSessionIV = generateEncIV(true);
    const auto symkeySession = openssl::AESSymmetricKey::createFromData(d_sessionKey);
    const auto ivSession = openssl::AESInitializationVector::createFromData(d_LastSessionIV);
    ByteVector encrypted;
    cipher.cipher(paddedData, encrypted, symkeySession, ivSession, false);
    return encrypted;
}

void SAMAV2ISO7816Commands::getLcLe(const ByteVector &cmd, bool &lc, bool &le)
{
    const size_t cmdSize = cmd.size();

    EXCEPTION_ASSERT_WITH_LOG(cmdSize >= AV2_HEADER_LENGTH, LibLogicalAccessException,
        sam::errorMessage(__func__, "APDU is shorter than the command header."));

    lc = false;
    le = false;

    // Command header only : [CLA INS P1 P2]
    if (cmdSize == AV2_LC_POS)
        return;
    // Header only with Le : [CLA INS P1 P2 LE]
    if (cmdSize == AV2_HEADER_LENGTH)
    {
        le = true;
        return;
    }
    const unsigned char lcByte = cmd[AV2_LC_POS];
    const size_t expectedSizeWithLc = static_cast<size_t>(lcByte) + AV2_HEADER_LENGTH;
    const size_t expectedSizeWithLcLe = static_cast<size_t>(lcByte) + AV2_HEADER_LENGTH_WITH_LE;
    // Header, Lc and data : [CLA INS P1 P2 LC DATA]
    if (cmdSize == expectedSizeWithLc)
    {
        lc = true;
        return;
    }
    // Header, Lc, data and Le : [CLA INS P1 P2 LC DATA LE]
    if (cmdSize == expectedSizeWithLcLe)
    {
        lc = true;
        le = true;
        return;
    }
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, sam::errorMessage(__func__, "Invalid APDU structure."));
}

sam::ApduInfo SAMAV2ISO7816Commands::getApduInfo(const ByteVector &cmd, sam::ApduFormat format)
{
    sam::ApduInfo info;
    if (format == sam::ApduFormat::Standard || format == sam::ApduFormat::ExtendedResponseOnly)
        getLcLe(cmd, info.hasLc, info.hasLe);
    else
    {
        info.hasLc = true;
        info.hasLe = (format == sam::ApduFormat::ExtendedWithLe);
    }
    return info;
}

ByteVector SAMAV2ISO7816Commands::verifyAndDecryptResponse(const ByteVector &response)
{
    if (d_hostMode != sam::HostMode::MAC && d_hostMode != sam::HostMode::FullProtect)
        THROW_EXCEPTION_WITH_LOG( LibLogicalAccessException,
            sam::errorMessage(__func__, "Requires MAC or FullProtect host mode."));

    /* begin check mac */
    if (response.size() < sam::MAC_SIZE + sam::STATUS_WORD_SIZE)
        return response;

    const auto cipher = std::make_shared<openssl::AESCipher>();
    ByteVector mac(response.end() - sam::MAC_SIZE - sam::STATUS_WORD_SIZE, response.end() - sam::STATUS_WORD_SIZE);

    ByteVector macInput, cmdCtrVector, macCiphertext;
    macInput.push_back(response[response.size() - 2]);
    macInput.push_back(response[response.size() - 1]);

    /* Set counter */
    BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
    std::reverse(cmdCtrVector.begin(), cmdCtrVector.end());
    macInput.insert(macInput.end(), cmdCtrVector.begin(), cmdCtrVector.end());

    const bool hasPayload = response.size() > sam::MAC_SIZE + sam::STATUS_WORD_SIZE;
    if (hasPayload)
    {
        /* Encrypt complete MAC blocks and preserve chaining IV. */
        auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_macSessionKey);
        auto ivMac     = openssl::AESInitializationVector::createFromData(d_lastMacIV);
        macInput.insert(macInput.end(), response.begin(), response.end() - sam::MAC_SIZE - sam::STATUS_WORD_SIZE);
        const size_t blockReady = (macInput.size() / sam::AES_BLOCK_SIZE) * sam::AES_BLOCK_SIZE;
        ByteVector lastBlock(macInput.begin() + blockReady, macInput.end());
        macInput.erase(macInput.begin() + blockReady, macInput.end());
        cipher->cipher(macInput, macCiphertext, symkeyMac, ivMac, false);
        d_lastMacIV.assign(macCiphertext.end() - sam::AES_BLOCK_SIZE, macCiphertext.end());
        macInput = std::move(lastBlock);
    }
    macCiphertext = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, macInput, d_lastMacIV, sam::AES_BLOCK_SIZE);
    truncateMacBuffer(macCiphertext);
    if (!std::equal(macCiphertext.begin(), macCiphertext.begin() + sam::MAC_SIZE, mac.begin()))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            sam::errorMessage(__func__, "Response CMAC verification failed."));

    ByteVector data;
    if (hasPayload)
    {
        if (d_hostMode == sam::HostMode::FullProtect)
        {
            /* begin decrypt */
            /* generate IV because first decrypt */
            auto symkeySession = openssl::AESSymmetricKey::createFromData(d_sessionKey);
            d_LastSessionIV = generateEncIV(false);
            auto ivSession = openssl::AESInitializationVector::createFromData(d_LastSessionIV);
            ByteVector encData(response.begin(), response.end() - sam::MAC_SIZE - sam::STATUS_WORD_SIZE);
            cipher->decipher(encData, data, symkeySession, ivSession, false);
            int i = static_cast<int>(data.size()) - 1;
            while (i >= 0 && data[i] != 0x80 && data[i] == 0x00)
                --i;
            if (i >= 0)
                data.resize(i);
        }
        else
        {
            data.assign(response.begin(), response.end() - sam::MAC_SIZE - sam::STATUS_WORD_SIZE);
        }
    }
    const auto swOffset = response.size() - sam::STATUS_WORD_SIZE;
    data.push_back(response[swOffset]);
    data.push_back(response[swOffset + 1]);
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

ByteVector SAMAV2ISO7816Commands::transmit(ByteVector cmd, bool first, bool last, bool s_mode)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid APDU : too short");

    if (d_sessionKey.empty())
        return getISO7816ReaderCardAdapter()->sendCommand(cmd);

    TransmissionOptions options {first || !s_mode, last || !s_mode, first, last, first || s_mode};
    return executeProtectedExchange(cmd, sam::ApduFormat::Standard, sam::PKI_ECC_LAYOUT, options);
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

ByteVector SAMAV2ISO7816Commands::executeProtectedExchange(const ByteVector &cmd, sam::ApduFormat format,
    const sam::ChainingLayout &layout, const TransmissionOptions &options)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            sam::errorMessage(__func__, "APDU is shorter than the command header."));

    try
    {
        const auto protectedApdu = prepareProtectedCommand(cmd, format);
        const auto frames        = createApduFrames(cmd, protectedApdu, format, layout);
        return completeSecureExchange(sendChainedFrames(frames), options);
    }
    catch (const std::exception &e)
    {
        secureZero(d_sessionKey);
        secureZero(d_macSessionKey);
        resetIVs();
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            sam::errorMessage(__func__, std::string("SAM transmission failed: ") + e.what()));
    }
}

sam::ProtectedApdu SAMAV2ISO7816Commands::prepareProtectedCommand(const ByteVector &cmd, sam::ApduFormat format)
{
    switch (d_hostMode)
    {
    case sam::HostMode::Plain: return {cmd}; // Plain mode : APDU is transmitted unchanged
    case sam::HostMode::MAC:
    case sam::HostMode::FullProtect: return prepareProtectedApdu(cmd, format);
    case sam::HostMode::None:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, sam::errorMessage(__func__,
                                 "Host authentication has not been established."));
    }
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, sam::errorMessage(__func__, "Invalid host mode."));
}

ByteVector SAMAV2ISO7816Commands::completeSecureExchange(ByteVector response, const TransmissionOptions &options)
{
    if (options.resetIvBeforeResponse)
        resetIVs();
    if (options.advanceCommandCounter)
        ++d_cmdCtr;
    if (options.processResponse)
    {
        switch (d_hostMode)
        {
        case sam::HostMode::Plain: break;
        case sam::HostMode::MAC:
        case sam::HostMode::FullProtect:
            response = verifyAndDecryptResponse(std::move(response));
            return response;
        case sam::HostMode::None:
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                sam::errorMessage(__func__, "Host authentication has not been established."));
        default:
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, sam::errorMessage(__func__, "Invalid host mode."));
        }
    }
    if (options.resetIvAfterResponse)
        resetIVs();

    return response;
}

std::vector<ByteVector> SAMAV2ISO7816Commands::createApduFrames(const ByteVector &cmd,
    const sam::ProtectedApdu &protection, sam::ApduFormat format, const sam::ChainingLayout &layout)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid APDU : too short");

    switch (d_hostMode)
    {
    case sam::HostMode::Plain: return createPlainChainedApduFrames(cmd, format, layout);
    case sam::HostMode::MAC:
    case sam::HostMode::FullProtect: return createSecureChainedApduFrames(cmd, protection, format, layout);
    case sam::HostMode::None:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            sam::errorMessage(__func__, "Host authentication has not been established."));
    }

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             sam::errorMessage(__func__, "Invalid host mode."));
}

std::vector<ByteVector> SAMAV2ISO7816Commands::createSecureChainedApduFrames(
    const ByteVector &cmd, const sam::ProtectedApdu &protection, sam::ApduFormat format,
    const sam::ChainingLayout &layout)
{
    constexpr size_t MaxChunkSize = sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC;

    constexpr unsigned char MoreFrame = 0xAF;
    constexpr unsigned char LastFrame = 0x00;

    const unsigned char cla = cmd[0];
    const unsigned char ins = cmd[1];
    const unsigned char p1  = cmd[2];
    const unsigned char p2  = cmd[3];

    const ByteVector &encData = protection.encData;
    const ByteVector &mac     = protection.mac;
    const bool le             = protection.hasLe;

    EXCEPTION_ASSERT_WITH_LOG(mac.size() >= sam::MAC_SIZE, LibLogicalAccessException,
        sam::errorMessage(__func__, "protected APDU MAC is shorter than expected."));

    const size_t frameCount =
        encData.empty() ? (format == sam::ApduFormat::ExtendedResponseOnly ? 1u : 0u)
                        : (encData.size() + MaxChunkSize - 1) / MaxChunkSize;
    std::vector<ByteVector> frames;
    frames.reserve(frameCount);
    auto buildFrame = [&](size_t offset, size_t chunkSize, bool isLastFrame) -> ByteVector
    {
        ByteVector frame;
        frame.reserve(AV2_HEADER_LENGTH + chunkSize + (isLastFrame ? sam::MAC_SIZE : 0));
        frame.push_back(cla);
        frame.push_back(ins);
        frame.push_back(p1);
        frame.push_back(p2);
        frame.push_back(0x00);
        const auto it = encData.begin() + offset;
        frame.insert(frame.end(), it, it + chunkSize);
        if (isLastFrame)
            frame.insert(frame.end(), mac.begin(), mac.begin() + sam::MAC_SIZE);
        if (format != sam::ApduFormat::ExtendedResponseOnly)
        {
            frame[layout.lastFrameIndex] = isLastFrame ? LastFrame : MoreFrame;
            if (offset > 0)
                frame[layout.modeIndex] = 0x00;
        }
        const size_t lc = frame.size() - AV2_HEADER_LENGTH;
        EXCEPTION_ASSERT_WITH_LOG(lc <= sam::MAX_APDU_DATA_SIZE, LibLogicalAccessException,
            "APDU payload exceeds the maximum supported size.");
        frame[AV2_LC_POS] = static_cast<unsigned char>(lc);
        return frame;
    };

    size_t offset = 0;
    for (size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex)
    {
        const size_t remaining = encData.size() - offset;
        const size_t chunkSize = (std::min)(MaxChunkSize, remaining);
        const bool isLastFrame = (frameIndex + 1 == frameCount);
        frames.emplace_back(buildFrame(offset, chunkSize, isLastFrame));
        offset += chunkSize;
    }

    if (le && !frames.empty())
        frames.back().push_back(cmd.back());

    return frames;
}

//TODO merge parts of this function with createSecureChainedApduFrames
std::vector<ByteVector> SAMAV2ISO7816Commands::createPlainChainedApduFrames(const ByteVector &cmd,
    sam::ApduFormat format, const sam::ChainingLayout &layout)
{
    constexpr size_t MaxChunkSize = sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC;

    constexpr unsigned char MoreFrame = 0xAF;
    constexpr unsigned char LastFrame = 0x00;
    
    if (cmd.size() <= MaxChunkSize)
        return {cmd};

    const auto apduInfo    = getApduInfo(cmd, format);
    const size_t dataBegin = apduInfo.hasLc ? AV2_HEADER_LENGTH : cmd.size(); //TODO optimize with cmd.size() <= MaxChunkSize
    const size_t dataEnd   = cmd.size() - (apduInfo.hasLc ? 1u : 0u);
    const size_t dataSize  = dataEnd - dataBegin;

    const unsigned char cla = cmd[0];
    const unsigned char ins = cmd[1];
    const unsigned char p1  = cmd[2];
    const unsigned char p2  = cmd[3];

    bool first = true; //TODO Leave it like this for now but refactor it later

    std::vector<ByteVector> frames;
    frames.reserve((dataSize + MaxChunkSize - 1) / MaxChunkSize);
    auto buildFrame = [&](size_t offset, size_t chunkSize, bool isLastFrame) -> ByteVector
    {
        ByteVector frame;
        frame.reserve(AV2_HEADER_LENGTH + chunkSize + (isLastFrame && apduInfo.hasLe ? 1 : 0));
        frame.push_back(cla);
        frame.push_back(ins);
        frame.push_back(p1);
        frame.push_back(p2);
        frame.push_back(0x00);
        auto it = cmd.begin() + offset;
        frame.insert(frame.end(), it, it + chunkSize);
        if (format != sam::ApduFormat::ExtendedResponseOnly)
        {
            frame[layout.lastFrameIndex] = isLastFrame ? LastFrame : MoreFrame;
            if (!first)
                frame[layout.modeIndex] = 0x00;
        }
        const size_t lc = frame.size() - AV2_HEADER_LENGTH;
        EXCEPTION_ASSERT_WITH_LOG(lc <= sam::MAX_APDU_DATA_SIZE, LibLogicalAccessException,
            "APDU payload exceeds the maximum supported size.");
        frame[AV2_LC_POS] = static_cast<unsigned char>(lc);
        return frame;
    };
    size_t offset = AV2_HEADER_LENGTH;
    while (offset < dataSize)
    {
        const size_t remaining = dataSize - offset;
        const size_t chunkSize = (std::min)(MaxChunkSize, remaining);
        const bool isLastFrame = (offset + chunkSize == dataSize);
        frames.emplace_back(buildFrame(offset, chunkSize, isLastFrame));
        offset += chunkSize;
        first = false;
    }
    if (apduInfo.hasLe && !frames.empty())
        frames.back().push_back(cmd.back());
    return frames;
}

ByteVector SAMAV2ISO7816Commands::sendChainedFrames(const std::vector<ByteVector> &frames)
{
    EXCEPTION_ASSERT_WITH_LOG(!frames.empty(), LibLogicalAccessException,
        sam::errorMessage(__func__, "No APDU frames to send."));
    
    auto adapter = getISO7816ReaderCardAdapter();
    const ByteVector continueApdu{d_cla, frames.front()[1], 0x00, 0x00, 0x00};

    ByteVector response;
    unsigned char sw1 = 0;
    unsigned char sw2 = 0;

    auto appendResponse = [&](const ByteVector &r)
    {
        EXCEPTION_ASSERT_WITH_LOG(r.size() >= sam::STATUS_WORD_SIZE,
            LibLogicalAccessException, "APDU response does not contain a status word.");
        const auto swOffset = r.size() - sam::STATUS_WORD_SIZE;
        sw1 = r[swOffset];
        sw2 = r[swOffset + 1];
        response.insert(response.end(), r.begin(), r.end() - sam::STATUS_WORD_SIZE);
    };

    for (size_t i = 0; i < frames.size(); ++i)
    {
        appendResponse(adapter->sendCommand(frames[i]));
        const bool lastFrame = (i + 1 == frames.size());
        if (!lastFrame && (sw1 != sam::sw::SuccessSW1 || sw2 != sam::sw::MoreDataSW2))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                sam::errorMessage(__func__, "Unexpected status word after intermediate chained frame."));
    }
    while (sw1 == sam::sw::SuccessSW1 && sw2 == sam::sw::MoreDataSW2)
    {
        appendResponse(adapter->sendCommand(continueApdu));
        if (sw1 != sam::sw::SuccessSW1 || (sw2 != sam::sw::SuccessSW2 && sw2 != sam::sw::MoreDataSW2))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                sam::errorMessage(__func__, "Unexpected status word during response chaining."));
    }
    if (sw1 != sam::sw::SuccessSW1 || sw2 != sam::sw::SuccessSW2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, sam::errorMessage(__func__, "Expected 0x9000"));
    response.push_back(sw1);
    response.push_back(sw2);
    return response;
}

void SAMAV2ISO7816Commands::validateSuccessResponse(const ByteVector &response, const char *caller) const
{
    EXCEPTION_ASSERT_WITH_LOG(response.size() >= sam::STATUS_WORD_SIZE, LibLogicalAccessException,
        sam::errorMessage(caller, "APDU response does not contain a status word."));

    const size_t swOffset = response.size() - sam::STATUS_WORD_SIZE;

    EXCEPTION_ASSERT_WITH_LOG(response[swOffset] == sam::sw::SuccessSW1 && response[swOffset + 1] == sam::sw::SuccessSW2,
                              LibLogicalAccessException, sam::errorMessage(caller, "Unexpected status word."));
}

std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2>>
SAMAV2ISO7816Commands::getKeyEntry(unsigned char keyno)
{
    constexpr size_t EXPECTED_SIZE_MIN         = 14;
    constexpr size_t EXPECTED_SIZE_MAX         = 15;

    unsigned char cmd[] = {d_cla, sam::ins::key::GetKeyEntry, keyno, 0x00, 0x00};
    ByteVector cmd_vector(cmd, cmd + 5);
    ByteVector result = transmit(cmd_vector, true, true);

    EXCEPTION_ASSERT_WITH_LOG(result.size() == EXPECTED_SIZE_MIN || result.size() == EXPECTED_SIZE_MAX,
                              LibLogicalAccessException, sam::errorMessage(__func__, "Unexpected response size."));

    validateSuccessResponse(result, __func__);

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
    auto kucentry = std::make_shared<SAMKucEntry>();
    const unsigned char cmd[] = {d_cla, sam::ins::kuc::GetEntry, kucno, 0x00, 0x00};
    const ByteVector cmd_vector(cmd, cmd + 5);
    const ByteVector result = transmit(cmd_vector, true, true);

    EXCEPTION_ASSERT_WITH_LOG(result.size() == sizeof(SAMKUCEntryStruct) + sam::STATUS_WORD_SIZE, LibLogicalAccessException,
                              sam::errorMessage(__func__, "Invalid response size."));

    validateSuccessResponse(result, __func__);

    SAMKUCEntryStruct kucentrys;
    memcpy(&kucentrys, &result[0], sizeof(SAMKUCEntryStruct));
    kucentry->setKucEntryStruct(kucentrys);

    return kucentry;
}

void SAMAV2ISO7816Commands::changeKUCEntry(unsigned char kucno,
                                           std::shared_ptr<SAMKucEntry> kucEntry,
                                           std::shared_ptr<DESFireKey> /*key*/)
{
    if (d_sessionKey.size() == 0)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            sam::errorMessage(__func__, "AuthenticationHost must be executed before calling this command."));

    const unsigned char lc = 0x06;
    const unsigned char cmd[] = {d_cla, sam::ins::kuc::ChangeEntry, kucno, kucEntry->getUpdateMask(), lc};
    ByteVector cmd_vector(cmd, cmd + 5);

    const auto &entry      = kucEntry->getKucEntryStruct();
    const auto *entryBytes = reinterpret_cast<const unsigned char *>(&entry);
    cmd_vector.insert(cmd_vector.end(), entryBytes, entryBytes + lc);

    const ByteVector result = transmit(cmd_vector, true, true);
    validateSuccessResponse(result, __func__);
}

void SAMAV2ISO7816Commands::changeKeyEntry(unsigned char keyno,
    std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2>> keyentry,
    std::shared_ptr<DESFireKey> /*key*/)
{
    if (d_sessionKey.size() == 0)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "Failed: AuthentificationHost have to be done before use such command.");

    const unsigned char proMas = keyentry->getUpdateMask();

    size_t buffer_size  = SAM_KEY_BUFFER_SIZE + sizeof(KeyEntryAV2Information);
    unsigned char *data = new unsigned char[buffer_size]();

    memcpy(data, keyentry->getData(), SAM_KEY_BUFFER_SIZE);
    memcpy(data + SAM_KEY_BUFFER_SIZE, &keyentry->getKeyEntryInformation(),
           sizeof(KeyEntryAV2Information));
    ByteVector vectordata(data, data + buffer_size);
    delete[] data;

    const unsigned char lc = static_cast<unsigned char>(vectordata.size());
    unsigned char cmd[] = {d_cla, sam::ins::key::ChangeKeyEntry, keyno, proMas, lc};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    const ByteVector result = transmit(cmd_vector, true, true);
    validateSuccessResponse(result, __func__);
}

void SAMAV2ISO7816Commands::changeKeyEntryOffline(unsigned char keyno, const KeyEntryUpdateSettings &updateSettings,
    unsigned short changecnt, const ByteVector &encke)
{
    const unsigned char proMas = SAMBasicKeyEntry::getUpdateMask(updateSettings);

    ByteVector vectordata;
    vectordata.reserve(sizeof(changecnt) + encke.size());
    sam::appendUInt16BE(vectordata, changecnt);
    vectordata.insert(vectordata.end(), encke.begin(), encke.end());

    const unsigned char lc = static_cast<unsigned char>(vectordata.size());
    unsigned char cmd[] = {d_cla, sam::ins::key::ChangeKeyEntry, keyno, proMas, lc};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    const ByteVector result = transmit(cmd_vector, true, true);
    validateSuccessResponse(result, __func__);
}

void SAMAV2ISO7816Commands::changeKUCEntryOffline(
    unsigned char kucno, const KucEntryUpdateSettings &updateSettings,
    unsigned short changecnt, const ByteVector &enckuc)
{
    const unsigned char proMas = SAMKucEntry::getUpdateMask(updateSettings);

    ByteVector vectordata;
    vectordata.reserve(sizeof(changecnt) + enckuc.size());
    sam::appendUInt16BE(vectordata, changecnt);
    vectordata.insert(vectordata.end(), enckuc.begin(), enckuc.end());

    const unsigned char lc = static_cast<unsigned char>(vectordata.size());
    unsigned char cmd[] = {d_cla, sam::ins::kuc::ChangeEntry, kucno, proMas, lc};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    ByteVector result = transmit(cmd_vector, true, true);
    validateSuccessResponse(result, __func__);
}

void SAMAV2ISO7816Commands::disableKeyEntryOffline(unsigned char keyno, unsigned short changecnt, const ByteVector &encuid)
{
    ByteVector vectordata;
    vectordata.reserve(sizeof(changecnt) + encuid.size());
    sam::appendUInt16BE(vectordata, changecnt);
    vectordata.insert(vectordata.end(), encuid.begin(), encuid.end());

    const unsigned char lc = static_cast<unsigned char>(vectordata.size());
    unsigned char cmd[] = {d_cla, sam::ins::key::DisableKeyEntryOffline, keyno, 0x00, lc};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    ByteVector result = transmit(cmd_vector, true, true);
    validateSuccessResponse(result, __func__);
}

ByteVector SAMAV2ISO7816Commands::dumpSecretKey(unsigned char keyno, unsigned char keyversion, const ByteVector& divInput)
{
    const unsigned char p1 = divInput.empty() ? 0x00 : 0x02;
    const unsigned char lc = static_cast<unsigned char>(0x02 + divInput.size());

    EXCEPTION_ASSERT_WITH_LOG(lc <= sam::MAX_APDU_DATA_SIZE, LibLogicalAccessException,
        sam::errorMessage(__func__, "Diversification input is too large."));

    unsigned char cmd[] = {d_cla, sam::ins::key::DumpSecretKey, p1, 0x00, lc, keyno, keyversion, 0x00};
    ByteVector cmd_vector(cmd, cmd + 8);
    cmd_vector.insert(cmd_vector.end() - 1, divInput.begin(), divInput.end());

    const ByteVector result = transmit(cmd_vector);
    validateSuccessResponse(result, __func__);

    return ByteVector(result.begin(), result.end() - sam::STATUS_WORD_SIZE);
}

void SAMAV2ISO7816Commands::activateOfflineKey(unsigned char keyno, unsigned char keyversion, const ByteVector& divInput)
{
    const unsigned char p1 = static_cast<unsigned char>(divInput.size() > 0x00);
    const unsigned char lc = static_cast<unsigned char>(0x02 + divInput.size());
    ByteVector cmd_vector = {d_cla, sam::ins::offline::ActivateKey, p1, 0x00, lc, keyno, keyversion};
    cmd_vector.insert(cmd_vector.end(), divInput.begin(), divInput.end());
    const ByteVector result = transmit(cmd_vector);
    validateSuccessResponse(result, __func__);
}

ByteVector SAMAV2ISO7816Commands::decipherOfflineData(const ByteVector &data)
{
    const unsigned char lc = static_cast<unsigned char>(data.size());
    ByteVector decipherOfflineData = {d_cla, sam::ins::offline::DecipherData, 0x00, 0x00, lc, 0x00};
    decipherOfflineData.insert(decipherOfflineData.end() - 1, data.begin(), data.end());

    ByteVector result = transmit(decipherOfflineData);
    validateSuccessResponse(result, __func__);
    result.resize(result.size() - sam::STATUS_WORD_SIZE);
    return result;
}

ByteVector SAMAV2ISO7816Commands::encipherOfflineData(const ByteVector &data)
{
    const unsigned char lc = static_cast<unsigned char>(data.size());
    ByteVector encipherOfflineData = {d_cla, sam::ins::offline::EncipherData, 0x00, 0x00, lc, 0x00};
    encipherOfflineData.insert(encipherOfflineData.end() - 1, data.begin(), data.end());

    ByteVector result = transmit(encipherOfflineData);
    validateSuccessResponse(result, __func__);
    result.resize(result.size() - sam::STATUS_WORD_SIZE);
    return result;
}

ByteVector SAMAV2ISO7816Commands::cmacOffline(const ByteVector &data)
{
    unsigned char Rb = 0x87;

    ByteVector blankbuf;
    blankbuf.resize(sam::AES_BLOCK_SIZE, 0x00);
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

    int pad = (sam::AES_BLOCK_SIZE - (data.size() % sam::AES_BLOCK_SIZE)) % sam::AES_BLOCK_SIZE;
    if (data.size() == 0)
        pad = sam::AES_BLOCK_SIZE;

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
    if (cmac.size() > sam::AES_BLOCK_SIZE)
    {
        cmac = ByteVector(cmac.end() - sam::AES_BLOCK_SIZE, cmac.end());
    }

    return cmac;
}

void SAMAV2ISO7816Commands::PKI_GenerateKeyPair(
    unsigned char keyNo, unsigned short configSettings, unsigned char keyNoCEK,
    unsigned char keyNoVCEK, unsigned char keyNoRef, const sam::AEKVAEK &accessKeys,
    unsigned short nLen, const ByteVector &pki_e, bool includeAccess)
{
    EXCEPTION_ASSERT_WITH_LOG(keyNo <= 0x01, LibLogicalAccessException,
                              sam::errorMessage(__func__, "Invalid key number."));

    EXCEPTION_ASSERT_WITH_LOG(nLen >= 0x40 && nLen <= 0x100 && (nLen % 8) == 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid RSA modulus length (nLen)."));

    const bool provideExponent = !pki_e.empty();
    const unsigned short eLen = provideExponent ? static_cast<unsigned short>(pki_e.size()) : 0x04;

    if (provideExponent)
    {
        EXCEPTION_ASSERT_WITH_LOG(eLen >= 0x04 && eLen <= 0x100 && (eLen % 4) == 0 && eLen <= nLen,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid exponent length (PKI_eLen)."));
        EXCEPTION_ASSERT_WITH_LOG((pki_e.back() & 0x01) != 0,
            LibLogicalAccessException, sam::errorMessage(__func__, "Exponent must be odd."));
    }

    unsigned short effectiveConfig = configSettings;
    if (includeAccess)
    {
        EXCEPTION_ASSERT_WITH_LOG(accessKeys,
            LibLogicalAccessException, sam::errorMessage(__func__, "AEK/VAEK required but null."));
        effectiveConfig &= static_cast<unsigned short>(~sam::pki::ConfigDisableBit);
    }

    const unsigned char p1 = static_cast<unsigned char>(provideExponent ? 0x01 : 0x00) | (includeAccess ? 0x02 : 0x00);

    ByteVector payload;
    payload.reserve((includeAccess ? 12 : 10) + pki_e.size());
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
    if (provideExponent)
        payload.insert(payload.end(), pki_e.begin(), pki_e.end());

    const unsigned char lc =
        payload.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC ? 0x00 : static_cast<unsigned char>(payload.size());
    ByteVector apdu{d_cla, sam::ins::pki::GenerateKeyPair, p1, 0x00, lc};
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::Extended);
    validateSuccessResponse(response, __func__);
}

void SAMAV2ISO7816Commands::PKI_ImportKey(
    unsigned char keyNo, unsigned short configSettings, unsigned char keyNoCEK,
    unsigned char keyNoVCEK, unsigned char refNoKUC, const ByteVector &pki_n,
    const ByteVector &pki_e, const ByteVector &pki_p, const ByteVector &pki_q,
    const ByteVector &pki_dP, const ByteVector &pki_dQ, const ByteVector &pki_ipq,
    const sam::AEKVAEK &accessKeys, bool includeAccess, bool updateSettingsOnly)
{
    const bool hasPrivateKey = !pki_p.empty();

    EXCEPTION_ASSERT_WITH_LOG(keyNo <= (hasPrivateKey ? 0x01 : 0x02),
        LibLogicalAccessException, sam::errorMessage(__func__, "Key number out of range."));

    const size_t nLen = pki_n.size();
    const size_t eLen = pki_e.size();

    if (!updateSettingsOnly)
    {
        EXCEPTION_ASSERT_WITH_LOG(!pki_n.empty() && !pki_e.empty(),
            LibLogicalAccessException, sam::errorMessage(__func__, "Missing RSA components."));

        EXCEPTION_ASSERT_WITH_LOG(nLen >= 0x40 && nLen <= 0x100 && (nLen % 8) == 0,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid modulus length."));

        EXCEPTION_ASSERT_WITH_LOG(nLen >= 4 && !(pki_n[0] == 0x00 && pki_n[1] == 0x00 && pki_n[2] == 0x00 && pki_n[3] == 0x00),
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid modulus MSW."));

        EXCEPTION_ASSERT_WITH_LOG(eLen >= 0x04 && eLen <= 0x100 && (eLen % 4) == 0 && eLen <= nLen,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid exponent length."));

        EXCEPTION_ASSERT_WITH_LOG((pki_e.back() & 0x01) != 0,
            LibLogicalAccessException, sam::errorMessage(__func__, "Exponent must be odd."));

        EXCEPTION_ASSERT_WITH_LOG(pki_n[0] != 0x00,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid modulus MSB."));
    }

    if (hasPrivateKey)
    {
        EXCEPTION_ASSERT_WITH_LOG(!pki_p.empty() && !pki_q.empty() && !pki_dP.empty() && !pki_dQ.empty() && !pki_ipq.empty(),
            LibLogicalAccessException, sam::errorMessage(__func__, "Incomplete CRT key."));

        EXCEPTION_ASSERT_WITH_LOG(pki_dP.size() == pki_p.size(),
            LibLogicalAccessException, sam::errorMessage(__func__, "dP length mismatch (must equal p length)."));

        EXCEPTION_ASSERT_WITH_LOG(pki_dQ.size() == pki_q.size(),
            LibLogicalAccessException, sam::errorMessage(__func__, "dQ length mismatch (must equal q length)."));

        EXCEPTION_ASSERT_WITH_LOG(pki_ipq.size() == pki_q.size(),
            LibLogicalAccessException, sam::errorMessage(__func__, "ipq length mismatch (must equal q length)."));

        EXCEPTION_ASSERT_WITH_LOG(pki_p[0] != 0x00 && pki_q[0] != 0x00,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid CRT prime MSB."));

        const size_t nWords = (nLen + 3) / 4;
        const size_t pWords = (pki_p.size() + 3) / 4;
        const size_t qWords = (pki_q.size() + 3) / 4;

        EXCEPTION_ASSERT_WITH_LOG(pWords + 2 <= nWords && qWords + 2 <= nWords,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid CRT size relation."));
    }

    const bool disableRequested    = (configSettings & 0x0004) != 0;
    unsigned short effectiveConfig = configSettings;

    if (includeAccess)
    {
        EXCEPTION_ASSERT_WITH_LOG(accessKeys,
            LibLogicalAccessException, sam::errorMessage(__func__, "AEK/VAEK required."));

        if ((effectiveConfig & sam::pki::ConfigDisableBit) != 0)
            LOG(LogLevel::WARNINGS) << sam::errorMessage(__func__, "Overriding PKI_SET disable bit due to AEK.");
        effectiveConfig &= static_cast<unsigned short>(~sam::pki::ConfigDisableBit);
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
        if (hasPrivateKey)
        {
            EXCEPTION_ASSERT_WITH_LOG(pki_p.size() <= 0xFFFF && pki_q.size() <= 0xFFFF,
                                      LibLogicalAccessException, sam::errorMessage(__func__, "CRT size overflow."));
            sam::appendUInt16BE(payload, static_cast<uint16_t>(pki_p.size()));
            sam::appendUInt16BE(payload, static_cast<uint16_t>(pki_q.size()));
        }
        payload.insert(payload.end(), pki_n.begin(), pki_n.end());
        payload.insert(payload.end(), pki_e.begin(), pki_e.end());
        if (hasPrivateKey)
        {
            payload.insert(payload.end(), pki_p.begin(), pki_p.end());
            payload.insert(payload.end(), pki_q.begin(), pki_q.end());
            payload.insert(payload.end(), pki_dP.begin(), pki_dP.end());
            payload.insert(payload.end(), pki_dQ.begin(), pki_dQ.end());
            payload.insert(payload.end(), pki_ipq.begin(), pki_ipq.end());
        }
    }

    const unsigned char lc =
        payload.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC ? 0x00 : static_cast<unsigned char>(payload.size());
    ByteVector apdu{d_cla, sam::ins::pki::ImportKey, p1, 0x00, lc};
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::Extended);
    validateSuccessResponse(response, __func__);
}

ByteVector SAMAV2ISO7816Commands::PKI_ExportPrivateKey(unsigned char keyNo, bool returnAEK)
{
    EXCEPTION_ASSERT_WITH_LOG(keyNo <= 0x01,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid key reference number."));

    const unsigned char returnAekFlag = static_cast<unsigned char>(returnAEK ? 0x80 : 0x00);
    ByteVector apdu{d_cla, sam::ins::pki::ExportPrivateKey, keyNo, returnAekFlag, 0x00};
    ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::ExtendedResponseOnly);
    validateSuccessResponse(response, __func__);

    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    return response;
}

ByteVector SAMAV2ISO7816Commands::PKI_ExportPublicKey(unsigned char keyNo, bool returnAEK)
{
    EXCEPTION_ASSERT_WITH_LOG(keyNo <= 0x02,
        LibLogicalAccessException, sam::errorMessage(__func__, "Key number out of range."));

    const unsigned char returnAekFlag = static_cast<unsigned char>(returnAEK ? 0x80 : 0x00);
    ByteVector apdu{d_cla, sam::ins::pki::ExportPublicKey, keyNo, returnAekFlag, 0x00};
    ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::ExtendedResponseOnly);
    validateSuccessResponse(response, __func__);

    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    return response;
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
        apdu.push_back(sam::ins::pki::UpdateKeyEntries);
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
    ByteVector resp = executeProtectedExchange(fullPayload, sam::ApduFormat::ExtendedWithLe);
    if (resp.size() < sam::STATUS_WORD_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_UpdateKeyEntries : response too short");

    const uint16_t sw = sam::parseStatusWord(resp);
    resp.resize(resp.size() - sam::STATUS_WORD_SIZE);
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
    EXCEPTION_ASSERT_WITH_LOG(sam::isSupportedHashAlgo(hashAlgo),
        LibLogicalAccessException, sam::errorMessage(__func__, "Unsupported or RFU hash algorithm."));

    EXCEPTION_ASSERT_WITH_LOG(!keyEntries.empty() && keyEntries.size() <= 3,
        LibLogicalAccessException, sam::errorMessage(__func__, "1 to 3 key entries are allowed."));

    EXCEPTION_ASSERT_WITH_LOG(divInput.empty() || divInput.size() <= 31,
        LibLogicalAccessException, sam::errorMessage(__func__, "DivInput must be between 1 and 31 bytes."));

    EXCEPTION_ASSERT_WITH_LOG(keyNoEnc <= 0x02,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid keyNoEnc."));

    EXCEPTION_ASSERT_WITH_LOG(keyNoSign <= 0x01,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid keyNoSign."));

    EXCEPTION_ASSERT_WITH_LOG(keyNoDec <= 0x01,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid keyNoDec."));

    EXCEPTION_ASSERT_WITH_LOG(keyNoVerif <= 0x02,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid keyNoVerif."));

    for (const auto &entry : keyEntries)
        EXCEPTION_ASSERT_WITH_LOG(entry.first <= 0x7F && entry.second <= 0x7F,
                                  LibLogicalAccessException, sam::errorMessage(__func__, "Invalid key entry pair."));

    const unsigned char p1 = (hashAlgo & 0x03) |
                             (static_cast<unsigned char>(keyEntries.size()) << 2) |
                             (divInput.empty() ? 0x00 : 0x10);

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

    const unsigned char lc =
        payload.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC ? 0x00 : static_cast<unsigned char>(payload.size());
    ByteVector apdu{d_cla, sam::ins::pki::EncipherKeyEntries, p1, 0x00, lc};
    apdu.insert(apdu.end(), payload.begin(), payload.end());
    apdu.push_back(0x00);

    ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::ExtendedWithLe);
    validateSuccessResponse(response, __func__);

    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    return response;
}

ByteVector SAMAV2ISO7816Commands::PKI_GenerateHash(unsigned char hashAlgo, const ByteVector &message)
{
    EXCEPTION_ASSERT_WITH_LOG(sam::isSupportedHashAlgo(hashAlgo),
        LibLogicalAccessException, sam::errorMessage(__func__, "Unsupported or RFU hash algorithm."));

    EXCEPTION_ASSERT_WITH_LOG(!message.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Message cannot be empty."));

    const uint32_t messageLen = static_cast<uint32_t>(message.size());
    const size_t payloadSize  = sizeof(messageLen) + message.size();
    const unsigned char lc =
        payloadSize > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC ? 0x00 : static_cast<unsigned char>(payloadSize);

    ByteVector apdu;
    apdu.reserve(5 + payloadSize + 1);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::GenerateHash);
    apdu.push_back(hashAlgo);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    sam::appendUInt32BE(apdu, messageLen);
    apdu.insert(apdu.end(), message.begin(), message.end());
    apdu.push_back(0x00);

    ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::ExtendedWithLe);
    validateSuccessResponse(response, __func__);

    EXCEPTION_ASSERT_WITH_LOG(response.size() == sam::expectedHashSize(hashAlgo) + sam::STATUS_WORD_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid hash length."));

    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    return response;
}

void SAMAV2ISO7816Commands::PKI_GenerateSignature(unsigned char hashAlgo, unsigned char keyNoSign, const ByteVector &hash)
{
    EXCEPTION_ASSERT_WITH_LOG(sam::isSupportedHashAlgo(hashAlgo),
        LibLogicalAccessException, sam::errorMessage(__func__, "Unsupported or RFU hash algorithm."));

    EXCEPTION_ASSERT_WITH_LOG(keyNoSign <= 0x01,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid key number."));

    const size_t expectedSize = sam::expectedHashSize(hashAlgo);

    EXCEPTION_ASSERT_WITH_LOG(hash.size() == expectedSize,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid hash size for selected algorithm."));

    const unsigned char lc = static_cast<unsigned char>(1 + hash.size());

    ByteVector apdu;
    apdu.reserve(5 + lc);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::GenerateSignature);
    apdu.push_back(hashAlgo);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.push_back(keyNoSign);
    apdu.insert(apdu.end(), hash.begin(), hash.end());

    const ByteVector response = transmit(apdu, true, true);
    validateSuccessResponse(response, __func__);
}

ByteVector SAMAV2ISO7816Commands::PKI_SendSignature()
{
    constexpr size_t MIN_SIG_SIZE = 8;
    constexpr size_t MAX_SIG_SIZE = 256;

    ByteVector signature = transmit({d_cla, sam::ins::pki::SendSignature, 0x00, 0x00, 0x00}, true, true);
    validateSuccessResponse(signature, __func__);

    signature.resize(signature.size() - sam::STATUS_WORD_SIZE);

    EXCEPTION_ASSERT_WITH_LOG(signature.size() >= MIN_SIG_SIZE && signature.size() <= MAX_SIG_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid signature size."));

    return signature;
}

void SAMAV2ISO7816Commands::PKI_VerifySignature(unsigned char hashAlgo,
                                                unsigned char keyNoVerif,
                                                const ByteVector &hash,
                                                const ByteVector &signature)
{
    EXCEPTION_ASSERT_WITH_LOG(sam::isSupportedHashAlgo(hashAlgo),
        LibLogicalAccessException, sam::errorMessage(__func__, "Unsupported or RFU hash algorithm."));

    EXCEPTION_ASSERT_WITH_LOG(keyNoVerif <= 0x02,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid verification key number."));

    EXCEPTION_ASSERT_WITH_LOG(hash.size() == sam::expectedHashSize(hashAlgo),
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid hash size for selected algorithm."));

    EXCEPTION_ASSERT_WITH_LOG(!signature.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Signature cannot be empty."));

    ByteVector payload;
    payload.reserve(1 + hash.size() + signature.size());
    payload.push_back(keyNoVerif);
    payload.insert(payload.end(), hash.begin(), hash.end());
    payload.insert(payload.end(), signature.begin(), signature.end());

    const unsigned char lc =
        payload.size() > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC ? 0x00 : static_cast<unsigned char>(payload.size());
    ByteVector apdu;
    apdu.reserve(5 + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::VerifySignature);
    apdu.push_back(hashAlgo);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::Extended);
    validateSuccessResponse(response, __func__);
}

ByteVector SAMAV2ISO7816Commands::PKI_EncipherData(unsigned char hashAlgo,
                                                   unsigned char keyNoEnc,
                                                   const ByteVector &plainData)
{
    EXCEPTION_ASSERT_WITH_LOG(sam::isSupportedHashAlgo(hashAlgo),
        LibLogicalAccessException, sam::errorMessage(__func__, "Unsupported or RFU hash algorithm."));

    EXCEPTION_ASSERT_WITH_LOG(keyNoEnc <= 0x02,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid encryption key number."));

    EXCEPTION_ASSERT_WITH_LOG(!plainData.empty(), LibLogicalAccessException,
                              sam::errorMessage(__func__, "Empty plaintext."));

    const unsigned char p1 = hashAlgo & 0x03;
    const size_t payloadSize = 1 + plainData.size();
    const unsigned char lc =
        payloadSize > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC ? 0x00 : static_cast<unsigned char>(payloadSize);

    ByteVector apdu;
    apdu.reserve(5 + payloadSize + 1);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::EncipherData);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.push_back(keyNoEnc);
    apdu.insert(apdu.end(), plainData.begin(), plainData.end());
    apdu.push_back(0x00);

    ByteVector encData = executeProtectedExchange(apdu, sam::ApduFormat::ExtendedWithLe);
    validateSuccessResponse(encData, __func__);

    encData.resize(encData.size() - sam::STATUS_WORD_SIZE);

    EXCEPTION_ASSERT_WITH_LOG(!encData.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Empty encrypted data returned."));

    return encData;
}

ByteVector SAMAV2ISO7816Commands::PKI_DecipherData(unsigned char hashAlgo,
                                                   unsigned char keyNoDec,
                                                   const ByteVector &encData)
{
    EXCEPTION_ASSERT_WITH_LOG(sam::isSupportedHashAlgo(hashAlgo),
        LibLogicalAccessException, sam::errorMessage(__func__, "Unsupported or RFU hash algorithm."));

    EXCEPTION_ASSERT_WITH_LOG(keyNoDec <= 0x01,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid key number."));

    EXCEPTION_ASSERT_WITH_LOG(!encData.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Empty encrypted data."));

    const unsigned char p1 = hashAlgo & 0x03;
    const size_t payloadSize = 1 + encData.size();
    const unsigned char lc =
        payloadSize > sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC ? 0x00 : static_cast<unsigned char>(payloadSize);

    ByteVector apdu;
    apdu.reserve(5 + payloadSize + 1);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::DecipherData);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.push_back(keyNoDec);
    apdu.insert(apdu.end(), encData.begin(), encData.end());
    apdu.push_back(0x00);

    ByteVector plainData = executeProtectedExchange(apdu, sam::ApduFormat::ExtendedWithLe);
    validateSuccessResponse(plainData, __func__);

    plainData.resize(plainData.size() - sam::STATUS_WORD_SIZE);

    EXCEPTION_ASSERT_WITH_LOG(!plainData.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Empty plaintext returned."));

    return plainData;
}

void SAMAV2ISO7816Commands::PKI_ImportEccKey(
    unsigned char keyNo, unsigned short eccSet, unsigned char keyNoCEK,
    unsigned char keyNoVCEK, unsigned char keyNoKUC, unsigned char keyNoAEK,
    unsigned char keyNoVAEK, const ByteVector &eccPublicKey, bool settingsOnly)
{
    EXCEPTION_ASSERT_WITH_LOG(keyNo <= 0x07,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid keyNo (must be 0...7)."));

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
        EXCEPTION_ASSERT_WITH_LOG(!eccPublicKey.empty(), LibLogicalAccessException,
            sam::errorMessage(__func__, "ECC public key must be provided when settingsOnly is false."));

        EXCEPTION_ASSERT_WITH_LOG(eccPublicKey[0] == 0x04, LibLogicalAccessException,
            sam::errorMessage(__func__, "ECC public key must start with 0x04 (uncompressed format)."));

        const size_t keySize = eccPublicKey.size();

        EXCEPTION_ASSERT_WITH_LOG(keySize >= 33 && keySize <= 65, LibLogicalAccessException,
            sam::errorMessage(__func__, "Invalid ECC public key length (must be 33...65 bytes)."));

        const unsigned short coordSize = static_cast<unsigned short>((keySize - 1) / 2);

        EXCEPTION_ASSERT_WITH_LOG((coordSize * 2 + 1) == keySize,
            LibLogicalAccessException, sam::errorMessage(__func__, "Malformed ECC point structure."));

        sam::appendUInt16BE(payload, coordSize);
        payload.insert(payload.end(), eccPublicKey.begin(), eccPublicKey.end());
    }

    EXCEPTION_ASSERT_WITH_LOG(payload.size() <= PKI_IMPORT_ECC_MAX_PAYLOAD,
        LibLogicalAccessException, sam::errorMessage(__func__, "Payload too large."));

    const unsigned char lc = static_cast<unsigned char>(payload.size());

    ByteVector apdu;
    apdu.reserve(5 + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::ImportECCKey);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = transmit(apdu, true, true);
    validateSuccessResponse(response, __func__);
}

void SAMAV2ISO7816Commands::PKI_ImportEccCurve(unsigned char curveNo, unsigned char keyNoCCK,
                                               unsigned char keyNoVCCK,
                                               const ByteVector &eccCurve,
                                               bool settingsOnly)
{
    EXCEPTION_ASSERT_WITH_LOG(curveNo <= 0x03,
        LibLogicalAccessException, sam::errorMessage(__func__, "Curve number out of range (0x00...0x03)."));

    EXCEPTION_ASSERT_WITH_LOG(keyNoCCK == 0xFE || keyNoCCK == 0xFF || keyNoCCK <= 0x7F,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid keyNoCCK."));

    const unsigned char p1 = settingsOnly ? 0x01 : 0x00;

    ByteVector payload;
    payload.reserve(3 + eccCurve.size());
    payload.push_back(curveNo);
    payload.push_back(keyNoCCK);
    payload.push_back(keyNoVCCK);

    if (!settingsOnly)
    {
        EXCEPTION_ASSERT_WITH_LOG(!eccCurve.empty(), LibLogicalAccessException,
            sam::errorMessage(__func__, "ECC curve data required when settingsOnly is false."));

        EXCEPTION_ASSERT_WITH_LOG(eccCurve.size() >= 2,
            LibLogicalAccessException, sam::errorMessage(__func__, "ECC curve data too short."));

        const unsigned char eccN = eccCurve[0];
        const unsigned char eccM = eccCurve[1];

        EXCEPTION_ASSERT_WITH_LOG(eccN >= 0x10 && eccN <= 0x20,
            LibLogicalAccessException, sam::errorMessage(__func__, "ECC_N out of range (0x10...0x20)."));

        EXCEPTION_ASSERT_WITH_LOG(eccM >= 0x10 && eccM <= 0x20,
            LibLogicalAccessException, sam::errorMessage(__func__, "ECC_M out of range (0x10...0x20)."));

        const size_t expectedSize = 2 + (5 * static_cast<size_t>(eccN)) + static_cast<size_t>(eccM);

        EXCEPTION_ASSERT_WITH_LOG(eccCurve.size() == expectedSize,
            LibLogicalAccessException, sam::errorMessage(__func__, "ECC curve length mismatch."));

        size_t offset = 2;

        const auto checkBlock = [&](size_t length, const char *name)
        {
            EXCEPTION_ASSERT_WITH_LOG(offset + length <= eccCurve.size(),
                LibLogicalAccessException, sam::errorMessage(__func__, std::string("Truncated field ") + name + "."));
            offset += length;
        };

        checkBlock(eccN, "ECC_Prime");
        checkBlock(eccN, "ECC_A");
        checkBlock(eccN, "ECC_B");
        checkBlock(eccN, "ECC_Px");
        checkBlock(eccN, "ECC_Py");
        checkBlock(eccM, "ECC_Order");

        payload.insert(payload.end(), eccCurve.begin(), eccCurve.end());
    }

    EXCEPTION_ASSERT_WITH_LOG(payload.size() <= sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC,
        LibLogicalAccessException, sam::errorMessage(__func__, "APDU payload too large."));

    const unsigned char lc = static_cast<unsigned char>(payload.size());

    ByteVector apdu;
    apdu.reserve(5 + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::ImportECCCurve);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = transmit(apdu, true, true);
    validateSuccessResponse(response, __func__);
}

ByteVector SAMAV2ISO7816Commands::PKI_ExportEccPublicKey(unsigned char keyNo)
{
    EXCEPTION_ASSERT_WITH_LOG(keyNo <= 0x07,
        LibLogicalAccessException, sam::errorMessage(__func__, "Key number out of range (0x00...0x07)."));

    ByteVector apdu{d_cla, sam::ins::pki::ExportECCPublicKey, keyNo, 0x00, 0x00};

    ByteVector response = transmit(apdu, true, true);
    validateSuccessResponse(response, __func__);

    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    return response;
}

void SAMAV2ISO7816Commands::PKI_VerifyEccSignature(unsigned char keyNo,
                                                   unsigned char curveNo,
                                                   const ByteVector &message,
                                                   const ByteVector &signature)
{
    EXCEPTION_ASSERT_WITH_LOG(keyNo <= 0x07,
        LibLogicalAccessException, sam::errorMessage(__func__, "Key number out of range (0x00...0x07)."));

    EXCEPTION_ASSERT_WITH_LOG(curveNo <= 0x03,
        LibLogicalAccessException, sam::errorMessage(__func__, "Curve number out of range (0x00...0x03)."));

    EXCEPTION_ASSERT_WITH_LOG(!message.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Message cannot be empty."));

    EXCEPTION_ASSERT_WITH_LOG(message.size() <= 0xFF,
        LibLogicalAccessException, sam::errorMessage(__func__, "Message too large."));

    EXCEPTION_ASSERT_WITH_LOG(!signature.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Signature cannot be empty."));

    ByteVector payload;
    payload.reserve(3 + message.size() + signature.size());
    payload.push_back(keyNo);
    payload.push_back(curveNo);
    payload.push_back(static_cast<unsigned char>(message.size()));
    payload.insert(payload.end(), message.begin(), message.end());
    payload.insert(payload.end(), signature.begin(), signature.end());

    EXCEPTION_ASSERT_WITH_LOG(payload.size() <= sam::SAM_SECURE_CHANNEL_MAX_PLAIN_LC,
        LibLogicalAccessException, sam::errorMessage(__func__, "APDU payload too large."));

    const unsigned char lc = static_cast<unsigned char>(payload.size());

    ByteVector apdu;
    apdu.reserve(5 + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::VerifyECCSignature);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = transmit(apdu, true, true);
    validateSuccessResponse(response, __func__);
}

}
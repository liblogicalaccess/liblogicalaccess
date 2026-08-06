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
#include <logicalaccess/plugins/crypto/secure_memory.hpp>

#include <cstring>

#include <openssl/x509.h>

namespace
{
using logicalaccess::security::SecureZeroGuard;
using logicalaccess::security::secureZeroBuffer;

using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

EVP_PKEY_ptr loadPublicKeyFromDER(const ByteVector &der)
{
    EXCEPTION_ASSERT_WITH_LOG(!der.empty(), logicalaccess::LibLogicalAccessException,
        logicalaccess::sam::errorMessage(__func__, "DER public key cannot be empty."));

    const unsigned char *ptr = der.data();

    EVP_PKEY *key = d2i_PUBKEY(nullptr, &ptr, static_cast<long>(der.size()));

    EXCEPTION_ASSERT_WITH_LOG(key, logicalaccess::LibLogicalAccessException,
        logicalaccess::sam::errorMessage(__func__, "Failed to parse DER public key."));

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_base_id(key) == EVP_PKEY_RSA, logicalaccess::LibLogicalAccessException,
        logicalaccess::sam::errorMessage(__func__, "Expected an RSA public key."));

    EXCEPTION_ASSERT_WITH_LOG(ptr == der.data() + der.size(), logicalaccess::LibLogicalAccessException,
        logicalaccess::sam::errorMessage(__func__, "Trailing bytes after DER public key."));

    return EVP_PKEY_ptr(key, EVP_PKEY_free);
}

EVP_PKEY_ptr loadPrivateKeyFromDER(const ByteVector &der)
{
    EXCEPTION_ASSERT_WITH_LOG(!der.empty(), logicalaccess::LibLogicalAccessException,
        logicalaccess::sam::errorMessage(__func__, "DER private key cannot be empty."));

    const unsigned char *ptr = der.data();

    EVP_PKEY *key = d2i_AutoPrivateKey(nullptr, &ptr, static_cast<long>(der.size()));

    EXCEPTION_ASSERT_WITH_LOG(key, logicalaccess::LibLogicalAccessException,
        logicalaccess::sam::errorMessage(__func__, "Failed to parse DER private key."));

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_base_id(key) == EVP_PKEY_RSA, logicalaccess::LibLogicalAccessException,
        logicalaccess::sam::errorMessage(__func__, "Expected an RSA private key."));

    EXCEPTION_ASSERT_WITH_LOG(ptr == der.data() + der.size(), logicalaccess::LibLogicalAccessException,
        logicalaccess::sam::errorMessage(__func__, "Trailing bytes after DER private key."));

    return EVP_PKEY_ptr(key, EVP_PKEY_free);
}

using SessionVectorTags = std::array<std::array<unsigned char, 4>, 3>;

constexpr SessionVectorTags ONLINE_SESSION_VECTOR_TAGS{{
    {0x81, 0x00, 0x82, 0x00}, // AES 128
    {0x83, 0x84, 0x85, 0x86}, // AES 192
    {0x87, 0x88, 0x89, 0x8A}  // AES 256
}};

constexpr SessionVectorTags OFFLINE_SESSION_VECTOR_TAGS{{
    {0x71, 0x00, 0x72, 0x00}, // AES 128
    {0x73, 0x74, 0x75, 0x76}, // AES 192
    {0x77, 0x78, 0x79, 0x7A}  // AES 256
}};

constexpr std::size_t keySizeIndex(std::size_t keySize)
{
    switch (keySize)
    {
    case logicalaccess::sam::AES_128_KEY_SIZE: return 0;
    case logicalaccess::sam::AES_192_KEY_SIZE: return 1;
    case logicalaccess::sam::AES_256_KEY_SIZE: return 2;
    default:
        THROW_EXCEPTION_WITH_LOG(logicalaccess::LibLogicalAccessException,
            logicalaccess::sam::errorMessage(__func__, "Invalid AES key size."));
    }
}

[[nodiscard]]
inline logicalaccess::sam::ProtectedApdu makeUnprotectedApdu(const ByteVector &cmd)
{
    return {cmd};
}

} // anonymous namespace

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

void SAMAV2ISO7816Commands::generateSessionKey(const ByteVector &rnda, const ByteVector &rndb)
{
    EXCEPTION_ASSERT_WITH_LOG(rnda.size() == sam::AES_BLOCK_SIZE && rndb.size() == sam::AES_BLOCK_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid random number size."));
    
    constexpr std::size_t SESSION_VECTOR_TAG_INDEX = sam::AES_BLOCK_SIZE - 1;

    ByteVector SV1a(sam::AES_BLOCK_SIZE);
    ByteVector SV1b(sam::AES_BLOCK_SIZE);
    ByteVector SV2a(sam::AES_BLOCK_SIZE);
    ByteVector SV2b(sam::AES_BLOCK_SIZE);

    SecureZeroGuard cleanup{&SV1a, &SV1b, &SV2a, &SV2b};

    const auto copy_block = [](ByteVector &dst, const ByteVector &src1, std::size_t off1,
                         const ByteVector &src2, std::size_t off2, const ByteVector &src3, std::size_t off3)
    {
        std::copy(src1.begin() + off1, src1.begin() + off1 + 5, dst.begin());
        std::copy(src2.begin() + off2, src2.begin() + off2 + 5, dst.begin() + 5);
        std::copy(src3.begin() + off3, src3.begin() + off3 + 5, dst.begin() + 10);
    };

    copy_block(SV1a, rnda, 11, rndb, 11, rnda, 4);
    for (std::size_t x = 4; x <= 9; ++x)
        SV1a[x + 6] ^= rndb[x];

    copy_block(SV1b, rnda, 10, rndb, 10, rnda, 5);
    for (std::size_t x = 5; x <= 10; ++x)
        SV1b[x + 5] ^= rndb[x];

    copy_block(SV2a, rnda, 7, rndb, 7, rnda, 0);
    for (std::size_t x = 0; x <= 5; ++x)
        SV2a[x + 10] ^= rndb[x];

    copy_block(SV2b, rnda, 6, rndb, 6, rnda, 1);
    for (std::size_t x = 1; x <= 6; ++x)
        SV2b[x + 9] ^= rndb[x];

    const std::size_t aesKeySize = d_macSessionKey.size();
    EXCEPTION_ASSERT_WITH_LOG(sam::isValidAESKeySize(aesKeySize),
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid session key size."));

    const auto &tags = ONLINE_SESSION_VECTOR_TAGS[keySizeIndex(aesKeySize)];

    SV1a[SESSION_VECTOR_TAG_INDEX] = tags[0];
    SV1b[SESSION_VECTOR_TAG_INDEX] = tags[1];
    SV2a[SESSION_VECTOR_TAG_INDEX] = tags[2];
    SV2b[SESSION_VECTOR_TAG_INDEX] = tags[3];

    deriveSessionKeys(d_macSessionKey, SV1a, SV1b, SV2a, SV2b);
}

void SAMAV2ISO7816Commands::generateOfflineSessionKey(const std::shared_ptr<DESFireKey> &key, unsigned short changecnt)
{
    EXCEPTION_ASSERT_WITH_LOG(key->getKeyType() == DF_KEY_AES,
        LibLogicalAccessException, sam::errorMessage(__func__, "Only AES Key allowed."));

    ByteVector keydata      = key->getData();
    const std::size_t aesKeySize = keydata.size();

    EXCEPTION_ASSERT_WITH_LOG(sam::isValidAESKeySize(aesKeySize),
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid AES key size."));

    secureZero(d_sessionKey);
    secureZero(d_macSessionKey);
    d_sessionKey.clear();
    d_macSessionKey.clear();

    ByteVector SV1a(sam::AES_BLOCK_SIZE);
    ByteVector SV1b(sam::AES_BLOCK_SIZE);
    ByteVector SV2a(sam::AES_BLOCK_SIZE);
    ByteVector SV2b(sam::AES_BLOCK_SIZE);

    SecureZeroGuard cleanup{&keydata, &SV1a, &SV1b, &SV2a, &SV2b};

    const auto &tags = OFFLINE_SESSION_VECTOR_TAGS[keySizeIndex(aesKeySize)];

    SV1a[0] = SV1b[0] = SV2a[0] = SV2b[0] = static_cast<unsigned char>((changecnt >> 8) & 0xff);
    SV1a[1] = SV1b[1] = SV2a[1] = SV2b[1] = static_cast<unsigned char>(changecnt & 0xff);
    std::fill(SV1a.begin() + 2, SV1a.end(), tags[0]);
    std::fill(SV1b.begin() + 2, SV1b.end(), tags[1]);
    std::fill(SV2a.begin() + 2, SV2a.end(), tags[2]);
    std::fill(SV2b.begin() + 2, SV2b.end(), tags[3]);

    deriveSessionKeys(keydata, SV1a, SV1b, SV2a, SV2b);
}

void SAMAV2ISO7816Commands::deriveSessionKeys(const ByteVector &masterKey, const ByteVector &SV1a, const ByteVector &SV1b,
    const ByteVector &SV2a, const ByteVector &SV2b)
{
    auto symkey = openssl::AESSymmetricKey::createFromData(masterKey);
    const auto iv = openssl::AESInitializationVector::createNull(); // emptyIV

    openssl::AESCipher cipher;
    ByteVector Kea, Keb, Kma, Kmb;

    SecureZeroGuard cleanup{&Kea, &Keb, &Kma, &Kmb};

    cipher.cipher(SV1a, Kea, symkey, iv, false);
    cipher.cipher(SV1b, Keb, symkey, iv, false);
    cipher.cipher(SV2a, Kma, symkey, iv, false);
    cipher.cipher(SV2b, Kmb, symkey, iv, false);

    d_sessionKey = std::move(Kea);
    d_macSessionKey = std::move(Kma);

    mergeDerivedKeys(Keb, Kmb, masterKey.size());
}

void SAMAV2ISO7816Commands::mergeDerivedKeys(const ByteVector &sessionKeyExtension,
    const ByteVector &macSessionKeyExtension, std::size_t keySize)
{
    if (keySize == sam::AES_256_KEY_SIZE) /* AES 256 */
    {
        d_sessionKey.insert(d_sessionKey.end(), sessionKeyExtension.begin(), sessionKeyExtension.end());
        d_macSessionKey.insert(d_macSessionKey.end(), macSessionKeyExtension.begin(), macSessionKeyExtension.end());
    }
    else if (keySize == sam::AES_192_KEY_SIZE) /* AES 192 */
    {
        for (std::size_t i = 0; i < 8; ++i)
        {
            d_sessionKey[8 + i] ^= sessionKeyExtension[i];
            d_macSessionKey[8 + i] ^= macSessionKeyExtension[i];
        }
        d_sessionKey.insert(d_sessionKey.end(), sessionKeyExtension.end() - 8, sessionKeyExtension.end());
        d_macSessionKey.insert(d_macSessionKey.end(), macSessionKeyExtension.end() - 8, macSessionKeyExtension.end());
    }
}

void SAMAV2ISO7816Commands::authenticateHost(std::shared_ptr<DESFireKey> key, unsigned char keyno)
{
    authenticateHost(key, keyno, sam::HostMode::FullProtect);
}

void SAMAV2ISO7816Commands::authenticateHost(const std::shared_ptr<DESFireKey> &key,
                                             unsigned char keyno, sam::HostMode hostmode)
{
    EXCEPTION_ASSERT_WITH_LOG(key != nullptr,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid key."));

    EXCEPTION_ASSERT_WITH_LOG(key->getKeyType() == DF_KEY_AES,
        LibLogicalAccessException, sam::errorMessage(__func__, "Only AES Key allowed."));

    constexpr std::size_t RND_SIZE = 12;

    const unsigned char mode = sam::toByte(hostmode);
    const ByteVector emptyIV(sam::AES_BLOCK_SIZE, 0x00);
    auto adapter = getISO7816ReaderCardAdapter();
    
    ByteVector keycipher = key->getData();
    EXCEPTION_ASSERT_WITH_LOG(sam::isValidAESKeySize(keycipher.size()),
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid AES key size."));

    ByteVector rnd2;
    ByteVector macHost;
    ByteVector rnd1;
    ByteVector data_p2;
    ByteVector rndA;
    ByteVector encRndB;
    ByteVector dencRndB;
    ByteVector rndB1;
    ByteVector dataHost;
    ByteVector encHost;
    ByteVector SAMrndA;

    SecureZeroGuard cleanup{&keycipher, &rnd2, &macHost, &rnd1, &data_p2, &rndA,
        &encRndB, &dencRndB, &rndB1, &dataHost, &encHost, &SAMrndA};

    /* Reset previous authentication state. */
    d_hostMode = sam::HostMode::None;

    secureZero(d_sessionKey);
    secureZero(d_macSessionKey);
    d_lastMacIV     = emptyIV;
    d_LastSessionIV = emptyIV;

    const ByteVector data_p1 = {keyno, key->getKeyVersion(), mode};
    auto result = adapter->sendAPDUCommand(d_cla, sam::ins::host::AuthenticateHost,
        0x00, 0x00, static_cast<unsigned char>(data_p1.size()), data_p1, 0x00);

    EXCEPTION_ASSERT_WITH_LOG(result.getData().size() == RND_SIZE && sam::hasMoreData(result.getSW1(), result.getSW2()),
        LibLogicalAccessException, sam::errorMessage(__func__, "P1 Failed."));

    d_macSessionKey = keycipher;
    auto cipher     = std::make_shared<openssl::AESCipher>();

    /* Create rnd2 for p3 - CMAC: rnd2 | Host Mode | ZeroPad */
    rnd2 = result.getData();
    rnd2.push_back(mode);
    rnd2.resize(sam::AES_BLOCK_SIZE, 0x00); // ZeroPad

    macHost = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, rnd2, d_lastMacIV, sam::AES_BLOCK_SIZE);
    truncateMacBuffer(macHost);

    rnd1.resize(RND_SIZE);
    EXCEPTION_ASSERT_WITH_LOG(RAND_bytes(rnd1.data(), static_cast<int>(rnd1.size())) == 1,
        LibLogicalAccessException, sam::errorMessage(__func__, "Cannot retrieve cryptographically strong bytes."));

    data_p2.reserve(sam::MAC_SIZE + rnd1.size());
    data_p2.insert(data_p2.end(), macHost.begin(), macHost.begin() + sam::MAC_SIZE);
    data_p2.insert(data_p2.end(), rnd1.begin(), rnd1.end());
    result = adapter->sendAPDUCommand(d_cla, sam::ins::host::AuthenticateHost,
        0x00, 0x00, static_cast<unsigned char>(data_p2.size()), data_p2, 0x00);
    EXCEPTION_ASSERT_WITH_LOG(result.getData().size() == 24 && sam::hasMoreData(result.getSW1(), result.getSW2()),
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
    rndA.resize(sam::AES_BLOCK_SIZE);
    EXCEPTION_ASSERT_WITH_LOG(RAND_bytes(rndA.data(), static_cast<int>(rndA.size())) == 1,
        LibLogicalAccessException, sam::errorMessage(__func__, "Cannot retrieve cryptographically strong bytes."));

    // decipher rndB
    auto symkey = openssl::AESSymmetricKey::createFromData(d_authKey);
    auto iv     = openssl::AESInitializationVector::createFromData(d_lastMacIV);

    encRndB.assign(result.getData().begin() + sam::MAC_SIZE, result.getData().end());
    cipher->decipher(encRndB, dencRndB, symkey, iv, false);

    // Rotate RndB : RndB' = RndB rotated by two bytes
    rndB1.insert(rndB1.end(), dencRndB.begin() + 2, dencRndB.end());
    rndB1.insert(rndB1.end(), dencRndB.begin(), dencRndB.begin() + 2);

    dataHost.reserve(rndA.size() + rndB1.size());
    dataHost.insert(dataHost.end(), rndA.begin(), rndA.end());   // RndA
    dataHost.insert(dataHost.end(), rndB1.begin(), rndB1.end()); // RndB'

    iv = openssl::AESInitializationVector::createFromData(d_lastMacIV);

    cipher->cipher(dataHost, encHost, symkey, iv, false);
    result = adapter->sendAPDUCommand(d_cla, sam::ins::host::AuthenticateHost,
        0x00, 0x00, static_cast<unsigned char>(encHost.size()), encHost, 0x00);
    EXCEPTION_ASSERT_WITH_LOG(result.getData().size() == sam::AES_BLOCK_SIZE &&
        sam::isSuccess(result.getSW1(), result.getSW2()),
        LibLogicalAccessException, sam::errorMessage(__func__, "P3 Failed."));

    iv = openssl::AESInitializationVector::createFromData(d_lastMacIV);
    cipher->decipher(result.getData(), SAMrndA, symkey, iv, false);
    SAMrndA.insert(SAMrndA.begin(), SAMrndA.end() - 2, SAMrndA.end());

    EXCEPTION_ASSERT_WITH_LOG(std::equal(SAMrndA.begin(), SAMrndA.begin() + sam::AES_BLOCK_SIZE, rndA.begin()),
        LibLogicalAccessException, sam::errorMessage(__func__, "P3 RndA from SAM is invalid."));

    generateSessionKey(rndA, dencRndB);
    d_cmdCtr = 0;
    d_hostMode = hostmode;
}

sam::ProtectedApdu SAMAV2ISO7816Commands::prepareProtectedApdu(const ByteVector &cmd, sam::ApduFormat format)
{
    EXCEPTION_ASSERT_WITH_LOG(cmd.size() >= sam::APDU_HEADER_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid command size."));

    ByteVector protectedCmd, protectedData;

    const auto apduInfo = getApduInfo(cmd, format);
    const bool encrypt = (d_hostMode == sam::HostMode::FullProtect);

    if (!apduInfo.hasLc)
    {
        protectedCmd = cmd;
        protectedCmd.insert(protectedCmd.begin() + sam::APDU_LC_INDEX, sam::MAC_SIZE);
    }
    else
    {
        const std::size_t dataEnd = apduInfo.hasLe ? cmd.size() - 1 : cmd.size();
        const ByteVector data(cmd.begin() + sam::APDU_HEADER_SIZE, cmd.begin() + dataEnd);
        if (encrypt)
        {
            protectedData = encryptCommandData(data);
            protectedCmd.assign(cmd.begin(), cmd.begin() + sam::APDU_HEADER_SIZE);
            protectedCmd.insert(protectedCmd.end(), protectedData.begin(), protectedData.end());
            if (apduInfo.hasLe)
                protectedCmd.push_back(cmd.back());
        }
        else
        {
            protectedData = data;
            protectedCmd = cmd;
        }
        const std::size_t lcSize = protectedData.size();
        protectedCmd[sam::APDU_LC_INDEX] = lcSize > sam::MAX_SECURE_APDU_DATA_SIZE ?
            0x00 : static_cast<unsigned char>(lcSize + sam::MAC_SIZE);
    }

    /* Set counter */
    ByteVector cmdCtr;
    cmdCtr.reserve(sizeof(std::uint32_t));
    sam::appendUInt32BE(cmdCtr, d_cmdCtr);
    protectedCmd.insert(protectedCmd.begin() + 2, cmdCtr.begin(), cmdCtr.end());

    const ByteVector mac = computeCommandMac(protectedCmd);

    return {protectedData, mac, apduInfo.hasLe};
}

ByteVector SAMAV2ISO7816Commands::computeCommandMac(ByteVector &protectedCmd)
{
    const auto cipher = std::make_shared<openssl::AESCipher>();
    const std::size_t blockReady = (protectedCmd.size() / sam::AES_BLOCK_SIZE) * sam::AES_BLOCK_SIZE;
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
    const std::size_t commandSize = cmd.size();

    EXCEPTION_ASSERT_WITH_LOG(commandSize >= sam::APDU_COMMAND_HEADER_SIZE, LibLogicalAccessException,
        sam::errorMessage(__func__, "APDU is shorter than the command header."));

    lc = false;
    le = false;

    // Command header only : [CLA INS P1 P2]
    if (commandSize == sam::APDU_COMMAND_HEADER_SIZE)
        return;
    // Command header with Le : [CLA INS P1 P2 Le]
    if (commandSize == sam::APDU_HEADER_SIZE)
    {
        le = true;
        return;
    }
    const std::size_t lcSize = static_cast<std::size_t>(cmd[sam::APDU_LC_INDEX]);
    // Command header, Lc and data : [CLA INS P1 P2 Lc Data]
    if (commandSize == sam::APDU_HEADER_SIZE + lcSize)
    {
        lc = true;
        return;
    }
    // Command header, Lc, data and Le : [CLA INS P1 P2 Lc Data Le]
    if (commandSize == sam::APDU_HEADER_WITH_LE_SIZE + lcSize)
    {
        lc = true;
        le = true;
        return;
    }
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, sam::errorMessage(__func__, "Invalid APDU structure."));
}

sam::ApduInfo SAMAV2ISO7816Commands::getApduInfo(const ByteVector &cmd, sam::ApduFormat format)
{
    sam::ApduInfo info{};
    if (format == sam::ApduFormat::SingleFrame)
        getLcLe(cmd, info.hasLc, info.hasLe);
    else
    {
        info.hasLc = true;
        info.hasLe = (format == sam::ApduFormat::ChainedWithLe);
    }
    return info;
}

std::size_t SAMAV2ISO7816Commands::getMaxSingleFramePayloadSize(bool hasLe) const
{
    const std::size_t leSize = hasLe ? 1u : 0u;
    switch (d_hostMode)
    {
    case sam::HostMode::Plain: return sam::MAX_APDU_PAYLOAD_SIZE - leSize;
    case sam::HostMode::MAC:
    case sam::HostMode::FullProtect: return sam::MAX_SECURE_APDU_DATA_SIZE;
    case sam::HostMode::None:
    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            sam::errorMessage(__func__, "Host authentication has not been established."));
    }
    return 0;
}

sam::ApduFormat SAMAV2ISO7816Commands::getApduFormat(std::size_t payloadSize, bool hasLe) const
{
    if (payloadSize <= getMaxSingleFramePayloadSize(hasLe))
        return sam::ApduFormat::SingleFrame;
    return hasLe ? sam::ApduFormat::ChainedWithLe : sam::ApduFormat::Chained;
}

ByteVector SAMAV2ISO7816Commands::verifyAndDecryptResponse(const ByteVector &response)
{
    EXCEPTION_ASSERT_WITH_LOG(d_hostMode == sam::HostMode::MAC || d_hostMode == sam::HostMode::FullProtect,
        LibLogicalAccessException, sam::errorMessage(__func__, "Requires MAC or FullProtect host mode."));

    /* begin check mac */
    if (response.size() < sam::MAC_SIZE + sam::STATUS_WORD_SIZE)
        return response;

    constexpr std::size_t CMD_COUNTER_SIZE = sizeof(std::uint32_t);

    const auto cipher = std::make_shared<openssl::AESCipher>();
    const std::size_t payloadSize = response.size() - sam::MAC_SIZE - sam::STATUS_WORD_SIZE;
    const auto swBegin    = response.end() - sam::STATUS_WORD_SIZE;
    const bool hasPayload = (payloadSize != 0);

    const ByteVector receivedMac(response.begin() + payloadSize, response.begin() + payloadSize + sam::MAC_SIZE);

    ByteVector macInput, cmdCtrVector, macCiphertext;
    macInput.reserve(sam::STATUS_WORD_SIZE + CMD_COUNTER_SIZE + payloadSize);
    macInput.insert(macInput.end(), swBegin, response.end());

    /* Set counter */
    cmdCtrVector.reserve(CMD_COUNTER_SIZE);
    sam::appendUInt32BE(cmdCtrVector, d_cmdCtr);
    macInput.insert(macInput.end(), cmdCtrVector.begin(), cmdCtrVector.end());

    if (hasPayload)
    {
        macInput.insert(macInput.end(), response.begin(), response.begin() + payloadSize);
        const std::size_t blockReady = (macInput.size() / sam::AES_BLOCK_SIZE) * sam::AES_BLOCK_SIZE;
        if (blockReady != 0)
        {
            /* Encrypt complete MAC blocks and preserve chaining IV. */
            const auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_macSessionKey);
            const auto ivMac = openssl::AESInitializationVector::createFromData(d_lastMacIV);
            ByteVector lastBlock(macInput.begin() + blockReady, macInput.end());
            macInput.erase(macInput.begin() + blockReady, macInput.end());
            cipher->cipher(macInput, macCiphertext, symkeyMac, ivMac, false);
            EXCEPTION_ASSERT_WITH_LOG(macCiphertext.size() >= sam::AES_BLOCK_SIZE,
                LibLogicalAccessException, sam::errorMessage(__func__, "Cipher output error."));
            d_lastMacIV.assign(macCiphertext.end() - sam::AES_BLOCK_SIZE, macCiphertext.end());
            macInput = std::move(lastBlock);
        }
    }
    macCiphertext = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, macInput, d_lastMacIV, sam::AES_BLOCK_SIZE);
    truncateMacBuffer(macCiphertext);
    EXCEPTION_ASSERT_WITH_LOG(std::equal(macCiphertext.begin(), macCiphertext.begin() + sam::MAC_SIZE, receivedMac.begin()),
        LibLogicalAccessException, sam::errorMessage(__func__, "Response CMAC verification failed."));

    ByteVector data;
    if (hasPayload)
    {
        if (d_hostMode == sam::HostMode::FullProtect)
        {
            /* begin decrypt */
            /* generate IV because first decrypt */
            const auto symkeySession = openssl::AESSymmetricKey::createFromData(d_sessionKey);
            d_LastSessionIV = generateEncIV(false);
            const auto ivSession = openssl::AESInitializationVector::createFromData(d_LastSessionIV);
            const ByteVector encData(response.begin(), response.begin() + payloadSize);
            cipher->decipher(encData, data, symkeySession, ivSession, false);
            int i = static_cast<int>(data.size()) - 1;
            while (i >= 0 && data[i] != 0x80 && data[i] == 0x00)
                --i;
            if (i >= 0)
                data.resize(i);
        }
        else
        {
            data.assign(response.begin(), response.begin() + payloadSize);
        }
    }
    data.insert(data.end(), swBegin, response.end());
    return data;
}

ByteVector SAMAV2ISO7816Commands::generateEncIV(bool encrypt) const
{
    constexpr std::size_t COUNTER_SIZE   = 4;
    constexpr std::size_t PREFIX_SIZE    = 4;
    constexpr std::size_t REPEAT_COUNT   = 3;
    constexpr unsigned char FILL_ENCRYPT = 0x01;
    constexpr unsigned char FILL_DECRYPT = 0x02;

    ByteVector ivInput(PREFIX_SIZE + COUNTER_SIZE * REPEAT_COUNT);

    const unsigned char fill = encrypt ? FILL_ENCRYPT : FILL_DECRYPT;
    std::fill(ivInput.begin(), ivInput.begin() + PREFIX_SIZE, fill);

    ByteVector cmdCtrVector;
    cmdCtrVector.reserve(sizeof(std::uint32_t));
    sam::appendUInt32BE(cmdCtrVector, d_cmdCtr);

    for (std::size_t i = 0; i < REPEAT_COUNT; ++i)
        std::copy(cmdCtrVector.begin(), cmdCtrVector.end(),
                  ivInput.begin() + PREFIX_SIZE + (i * COUNTER_SIZE));

    const auto symkeyMac = openssl::AESSymmetricKey::createFromData(d_sessionKey);
    const auto iv = openssl::AESInitializationVector::createFromData(d_LastSessionIV);
    openssl::AESCipher cipher;

    ByteVector encryptedIV;
    cipher.cipher(ivInput, encryptedIV, symkeyMac, iv, false);

    return encryptedIV;
}

ByteVector SAMAV2ISO7816Commands::transmit(ByteVector cmd, bool first, bool last, bool s_mode)
{
    EXCEPTION_ASSERT_WITH_LOG(cmd.size() >= sam::APDU_COMMAND_HEADER_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid APDU : too short"));

    if (d_sessionKey.empty())
        return getISO7816ReaderCardAdapter()->sendCommand(cmd);

    const TransmissionOptions options {first || !s_mode, last || !s_mode, first, last, first || s_mode};
    return executeProtectedExchange(cmd, sam::ApduFormat::SingleFrame, sam::PKI_ECC_LAYOUT, options);
}

void SAMAV2ISO7816Commands::resetIVs()
{
    secureZero(d_LastSessionIV);
    secureZero(d_lastMacIV);
}

void SAMAV2ISO7816Commands::secureZero(ByteVector &buffer) noexcept
{
    secureZeroBuffer(buffer);
}

ByteVector SAMAV2ISO7816Commands::executeProtectedExchange(const ByteVector &cmd, sam::ApduFormat format,
    const sam::ChainingLayout &layout, const TransmissionOptions &options)
{
    EXCEPTION_ASSERT_WITH_LOG(cmd.size() >= sam::APDU_COMMAND_HEADER_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "APDU is shorter than the command header."));

    try
    {
        const auto protectedApdu = options.protectRequest
            ? prepareProtectedCommand(cmd, format) // Apply current secure messaging mode
            : makeUnprotectedApdu(cmd); // Explicitly bypass protection for this exchange
        const auto frames = createApduFrames(cmd, protectedApdu, format, layout, options.protectRequest);
        auto response = sendChainedFrames(frames, options.processResponse);
        return completeSecureExchange(std::move(response), options);
    }
    catch (const std::exception &e)
    {
        secureZero(d_sessionKey);
        secureZero(d_macSessionKey);
        resetIVs();
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            sam::errorMessage(__func__, std::string("SAM transmission failed : ") + e.what()));
    }
}

sam::ProtectedApdu SAMAV2ISO7816Commands::prepareProtectedCommand(const ByteVector &cmd, sam::ApduFormat format)
{
    switch (d_hostMode)
    {
    case sam::HostMode::Plain: return makeUnprotectedApdu(cmd); // Plain host mode : APDU is transmitted unchanged
    case sam::HostMode::MAC:
    case sam::HostMode::FullProtect: return prepareProtectedApdu(cmd, format);
    case sam::HostMode::None:
    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            sam::errorMessage(__func__, "Host authentication has not been established."));
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
            break;
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
    const sam::ProtectedApdu &protection, sam::ApduFormat format, const sam::ChainingLayout &layout, bool protectRequest)
{
    EXCEPTION_ASSERT_WITH_LOG(cmd.size() >= sam::APDU_COMMAND_HEADER_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid APDU header."));

    EXCEPTION_ASSERT_WITH_LOG(layout.hasModeIndex() == layout.hasLastFrameIndex(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Incomplete APDU chaining layout."));

    if (layout.hasChaining())
        EXCEPTION_ASSERT_WITH_LOG(layout.modeIndex < sam::APDU_HEADER_SIZE && layout.lastFrameIndex < sam::APDU_HEADER_SIZE,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid APDU chaining layout."));

    // Build an unprotected APDU frame
    // The transmission mode overrides HostMode : no encryption and no MAC shall be applied for this exchange
    if (!protectRequest)
        return createPlainChainedApduFrames(cmd, format, layout);

    switch (d_hostMode)
    {
    case sam::HostMode::Plain: return createPlainChainedApduFrames(cmd, format, layout);
    case sam::HostMode::MAC:
    case sam::HostMode::FullProtect: return createSecureChainedApduFrames(cmd, protection, format, layout);
    case sam::HostMode::None:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            sam::errorMessage(__func__, "Host authentication has not been established."));
    }

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, sam::errorMessage(__func__, "Invalid host mode."));
}

std::vector<ByteVector> SAMAV2ISO7816Commands::createSecureChainedApduFrames(
    const ByteVector &cmd, const sam::ProtectedApdu &protection, sam::ApduFormat format,
    const sam::ChainingLayout &layout)
{
    
    EXCEPTION_ASSERT_WITH_LOG(protection.mac.size() >= sam::MAC_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid MAC size."));

    constexpr std::size_t maxChunkSize = sam::MAX_SECURE_APDU_DATA_SIZE;

    const bool updateChainingFields = format != sam::ApduFormat::SingleFrame && layout.hasChaining();

    const auto &encData = protection.encData;
    const auto &mac     = protection.mac;
    const bool appendLe = protection.hasLe;

    const std::size_t maxPayloadSize = sam::MAX_APDU_PAYLOAD_SIZE - (appendLe ? 1u : 0u);

    const std::size_t frameCount = std::max<std::size_t>(1, (encData.size() + maxChunkSize - 1) / maxChunkSize);
    std::vector<ByteVector> frames;
    frames.reserve(frameCount);

    const unsigned char cla = cmd[0];
    const unsigned char ins = cmd[1];
    const unsigned char p1  = cmd[2];
    const unsigned char p2  = cmd[3];

    auto buildFrame = [&](std::size_t offset, std::size_t chunkSize, bool lastFrame) -> ByteVector
    {
        ByteVector frame;
        frame.reserve(sam::APDU_HEADER_SIZE + chunkSize +
            (lastFrame ? sam::MAC_SIZE : 0u) + (appendLe && lastFrame ? 1u : 0u));
        frame.push_back(cla);
        frame.push_back(ins);
        frame.push_back(p1);
        frame.push_back(p2);
        frame.push_back(0x00);
        frame.insert(frame.end(), encData.begin() + offset, encData.begin() + offset + chunkSize);

        if (lastFrame)
            frame.insert(frame.end(), mac.begin(), mac.begin() + sam::MAC_SIZE);
        if (updateChainingFields)
        {
            frame[layout.lastFrameIndex] = lastFrame ? sam::chaining::End : sam::chaining::Continue;
            if (offset != 0)
                frame[layout.modeIndex] = 0x00;
        }
        const std::size_t lc = frame.size() - sam::APDU_HEADER_SIZE;
        EXCEPTION_ASSERT_WITH_LOG(lc <= maxPayloadSize,
            LibLogicalAccessException, sam::errorMessage(__func__, "Secure APDU payload exceeds the maximum supported size."));
        frame[sam::APDU_LC_INDEX] = static_cast<unsigned char>(lc);
        return frame;
    };

    std::size_t offset = 0;
    for (std::size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex)
    {
        const std::size_t remaining = encData.size() - offset;
        const std::size_t chunkSize = (std::min)(maxChunkSize, remaining);
        const bool lastFrame = (frameIndex == frameCount - 1);
        frames.emplace_back(buildFrame(offset, chunkSize, lastFrame));
        offset += chunkSize;
    }

    if (appendLe && !frames.empty())
        frames.back().push_back(cmd.back());

    return frames;
}

std::vector<ByteVector> SAMAV2ISO7816Commands::createPlainChainedApduFrames(const ByteVector &cmd,
    sam::ApduFormat format, const sam::ChainingLayout &layout)
{
    const auto apduInfo = getApduInfo(cmd, format);

    const std::size_t maxChunkSize = sam::MAX_APDU_PAYLOAD_SIZE;
    const std::size_t lastFrameCapacity = apduInfo.hasLe ? maxChunkSize - 1u : maxChunkSize;

    if (cmd.size() <= sam::MAX_APDU_SIZE)
        return {cmd};

    const bool updateChainingFields = format != sam::ApduFormat::SingleFrame && layout.hasChaining();

    const std::size_t dataBegin = apduInfo.hasLc ? sam::APDU_HEADER_SIZE : cmd.size();
    const std::size_t dataEnd = apduInfo.hasLc ? cmd.size() - (apduInfo.hasLe ? 1u : 0u) : cmd.size();
    const std::size_t dataSize = dataEnd - dataBegin;

    // Reserve is only an estimate. When Le is present, the last frame has one byte less capacity.
    std::vector<ByteVector> frames;
    frames.reserve(std::max<std::size_t>(1, (dataSize + maxChunkSize - 1) / maxChunkSize));

    const unsigned char cla = cmd[0];
    const unsigned char ins = cmd[1];
    const unsigned char p1  = cmd[2];
    const unsigned char p2  = cmd[3];

    auto buildFrame = [&](std::size_t offset, std::size_t chunkSize, bool lastFrame) -> ByteVector
    {
        ByteVector frame;
        frame.reserve(sam::APDU_HEADER_SIZE + chunkSize + (lastFrame && apduInfo.hasLe ? 1u : 0u));
        frame.push_back(cla);
        frame.push_back(ins);
        frame.push_back(p1);
        frame.push_back(p2);
        if (dataSize == 0) // No command payload : preserve ISO7816 Case 1 and 2 encoding by omitting Lc.
            return frame;
        frame.push_back(0x00);
        frame.insert(frame.end(), cmd.begin() + dataBegin + offset, cmd.begin() + dataBegin + offset + chunkSize);
        if (updateChainingFields)
        {
            frame[layout.lastFrameIndex] = lastFrame ? sam::chaining::End : sam::chaining::Continue;
            if (offset != 0)
                frame[layout.modeIndex] = 0x00;
        }
        const std::size_t lc = frame.size() - sam::APDU_HEADER_SIZE;
        EXCEPTION_ASSERT_WITH_LOG(lc <= maxChunkSize,
            LibLogicalAccessException, sam::errorMessage(__func__, "APDU payload exceeds the maximum supported size."));
        frame[sam::APDU_LC_INDEX] = static_cast<unsigned char>(lc);
        return frame;
    };

    std::size_t offset = 0;
    do
    {
        const std::size_t remaining = dataSize - offset;
        const bool lastFrame = remaining <= lastFrameCapacity;
        const std::size_t currentChunkCapacity = lastFrame ? lastFrameCapacity : maxChunkSize;
        const std::size_t chunkSize = (std::min)(currentChunkCapacity, remaining);
        frames.emplace_back(buildFrame(offset, chunkSize, lastFrame));
        offset += chunkSize;
    } while (offset < dataSize);

    if (apduInfo.hasLe && !frames.empty())
        frames.back().push_back(cmd.back());

    return frames;
}

ByteVector SAMAV2ISO7816Commands::sendChainedFrames(const std::vector<ByteVector> &frames, bool expectResponse)
{
    EXCEPTION_ASSERT_WITH_LOG(!frames.empty(), LibLogicalAccessException,
        sam::errorMessage(__func__, "No APDU frames to send."));
    
    auto adapter = getISO7816ReaderCardAdapter();
    const ByteVector continueApdu{d_cla, frames.front()[1], 0x00, 0x00, 0x00};

    ByteVector response;
    unsigned char sw1 = 0;
    unsigned char sw2 = 0;

    auto appendResponseData = [&](const ByteVector &apdu)
    {
        EXCEPTION_ASSERT_WITH_LOG(apdu.size() >= sam::STATUS_WORD_SIZE,
            LibLogicalAccessException, sam::errorMessage(__func__, "APDU response does not contain a status word."));
        const auto swBegin = apdu.end() - sam::STATUS_WORD_SIZE;
        sw1 = swBegin[0];
        sw2 = swBegin[1];
        response.insert(response.end(), apdu.begin(), swBegin);
    };
    for (std::size_t i = 0; i < frames.size(); ++i)
    {
        appendResponseData(adapter->sendCommand(frames[i]));
        const bool lastFrame = (i == frames.size() - 1);
        if (!lastFrame && !sam::hasMoreData(sw1, sw2))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                sam::errorMessage(__func__, "Unexpected status word after intermediate chained frame."));
    }
    if (expectResponse) // Only execute SAM response chaining if this exchange expects a
                        // response
    {
        while (sam::hasMoreData(sw1, sw2))
        {
            appendResponseData(adapter->sendCommand(continueApdu));
            if (!sam::validState(sw1, sw2))
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                    sam::errorMessage(__func__, "Unexpected status word during response chaining."));
        }
        if (!sam::isSuccess(sw1, sw2))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, sam::errorMessage(__func__, "Expected 0x9000"));
    }
    response.push_back(sw1);
    response.push_back(sw2);
    return response;
}

void SAMAV2ISO7816Commands::validateSuccessResponse(const ByteVector &response, const char *caller) const
{
    std::uint16_t sw;

    EXCEPTION_ASSERT_WITH_LOG(sam::tryParseStatusWord(response, sw),
        LibLogicalAccessException, sam::errorMessage(caller, "APDU response does not contain a status word."));

    EXCEPTION_ASSERT_WITH_LOG(sam::isSuccess(static_cast<unsigned char>(sw >> 8), static_cast<unsigned char>(sw & 0xFF)),
        LibLogicalAccessException, sam::errorMessage(caller, "Unexpected status word."));
}

std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2>> SAMAV2ISO7816Commands::getKeyEntry(unsigned char keyno)
{
    constexpr std::size_t EXPECTED_SIZE_MIN = 14;
    constexpr std::size_t EXPECTED_SIZE_MAX = 15;

    unsigned char cmd[] = {d_cla, sam::ins::key::GetKeyEntry, keyno, 0x00, 0x00};
    ByteVector cmd_vector(cmd, cmd + 5);
    ByteVector result = transmit(cmd_vector, true, true);

    EXCEPTION_ASSERT_WITH_LOG(result.size() == EXPECTED_SIZE_MIN || result.size() == EXPECTED_SIZE_MAX,
                              LibLogicalAccessException, sam::errorMessage(__func__, "Unexpected response size."));

    validateSuccessResponse(result, __func__);

    const std::size_t resultSize = result.size();
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
    EXCEPTION_ASSERT_WITH_LOG(!d_sessionKey.empty(), LibLogicalAccessException,
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
    EXCEPTION_ASSERT_WITH_LOG(!d_sessionKey.empty(), LibLogicalAccessException,
        sam::errorMessage(__func__, "Failed : AuthenticateHost has to be done before using this command."));

    const unsigned char proMas = keyentry->getUpdateMask();

    std::size_t buffer_size = SAM_KEY_BUFFER_SIZE + sizeof(KeyEntryAV2Information);
    unsigned char *data = new unsigned char[buffer_size]();

    memcpy(data, keyentry->getData(), SAM_KEY_BUFFER_SIZE);
    memcpy(data + SAM_KEY_BUFFER_SIZE, &keyentry->getKeyEntryInformation(), sizeof(KeyEntryAV2Information));
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

ByteVector SAMAV2ISO7816Commands::dumpSecretKey(unsigned char keyno, unsigned char keyversion, const ByteVector &divInput)
{
    const unsigned char p1 = divInput.empty() ? 0x00 : 0x02;
    const unsigned char lc = static_cast<unsigned char>(0x02 + divInput.size());

    EXCEPTION_ASSERT_WITH_LOG(lc <= sam::MAX_SECURE_APDU_DATA_SIZE, LibLogicalAccessException,
        sam::errorMessage(__func__, "Diversification input is too large."));

    unsigned char cmd[] = {d_cla, sam::ins::key::DumpSecretKey, p1, 0x00, lc, keyno, keyversion, 0x00};
    ByteVector cmd_vector(cmd, cmd + 8);
    cmd_vector.insert(cmd_vector.end() - 1, divInput.begin(), divInput.end());

    const ByteVector result = transmit(cmd_vector);
    validateSuccessResponse(result, __func__);

    return ByteVector(result.begin(), result.end() - sam::STATUS_WORD_SIZE);
}

void SAMAV2ISO7816Commands::activateOfflineKey(unsigned char keyno, unsigned char keyversion, const ByteVector &divInput)
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
    unsigned short nLen, unsigned short eLen, const ByteVector &pki_e, bool includeAccess)
{
    EXCEPTION_ASSERT_WITH_LOG(keyNo <= 0x01,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid key number."));

    EXCEPTION_ASSERT_WITH_LOG(nLen >= 0x40 && nLen <= 0x100 && (nLen % 8) == 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid RSA modulus length (nLen)."));

    EXCEPTION_ASSERT_WITH_LOG(eLen >= 0x04 && eLen <= 0x100 && (eLen % 4) == 0 && eLen <= nLen,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid exponent length (eLen)."));

    const bool provideExponent = !pki_e.empty();

    if (provideExponent)
    {
        EXCEPTION_ASSERT_WITH_LOG(static_cast<unsigned short>(pki_e.size()) == eLen,
            LibLogicalAccessException, sam::errorMessage(__func__, "PKI_e length does not match PKI_eLen."));

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

    constexpr std::size_t BASE_PAYLOAD_SIZE = 10;
    constexpr std::size_t ACCESS_KEYS_SIZE  = 2;

    ByteVector payload;
    payload.reserve(BASE_PAYLOAD_SIZE + (includeAccess ? ACCESS_KEYS_SIZE : 0u) + pki_e.size());
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

    const sam::ApduFormat format = getApduFormat(payload.size(), false);
    const unsigned char lc = (format == sam::ApduFormat::SingleFrame ? static_cast<unsigned char>(payload.size()) : 0x00);
    ByteVector apdu{d_cla, sam::ins::pki::GenerateKeyPair, p1, 0x00, lc};
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = executeProtectedExchange(apdu, format);
    validateSuccessResponse(response, __func__);
}

void SAMAV2ISO7816Commands::PKI_ImportKey(
    unsigned char keyNo, unsigned short configSettings, unsigned char keyNoCEK,
    unsigned char keyNoVCEK, unsigned char refNoKUC, const ByteVector &pki_n,
    const ByteVector &pki_e, const ByteVector &pki_p, const ByteVector &pki_q,
    const ByteVector &pki_dP, const ByteVector &pki_dQ, const ByteVector &pki_ipq,
    const sam::AEKVAEK &accessKeys, bool includeAccess, bool updateSettingsOnly)
{
    const bool hasPrivateKey = !pki_p.empty() && !pki_q.empty() &&
                                       !pki_dP.empty() && !pki_dQ.empty() && !pki_ipq.empty();
    
    const bool hasAnyPrivateComponent = !pki_p.empty() || !pki_q.empty() ||
                                        !pki_dP.empty() || !pki_dQ.empty() || !pki_ipq.empty();

    if (hasAnyPrivateComponent)
        EXCEPTION_ASSERT_WITH_LOG(hasPrivateKey,
            LibLogicalAccessException, sam::errorMessage(__func__, "Incomplete CRT key."));

    EXCEPTION_ASSERT_WITH_LOG(keyNo <= (hasPrivateKey ? 0x01 : 0x02),
        LibLogicalAccessException, sam::errorMessage(__func__, "Key number out of range."));

    const std::size_t nLen = pki_n.size();
    const std::size_t eLen = pki_e.size();
    const std::size_t pLen = hasPrivateKey ? pki_p.size() : 0;
    const std::size_t qLen = hasPrivateKey ? pki_q.size() : 0;


    if (!updateSettingsOnly)
    {
        EXCEPTION_ASSERT_WITH_LOG(!pki_n.empty() && !pki_e.empty(),
            LibLogicalAccessException, sam::errorMessage(__func__, "Missing RSA public key components."));

        EXCEPTION_ASSERT_WITH_LOG(nLen >= 0x40 && nLen <= 0x100 && (nLen % 8) == 0,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid modulus length."));

        EXCEPTION_ASSERT_WITH_LOG(nLen >= 4 && !(pki_n[0] == 0x00 && pki_n[1] == 0x00 && pki_n[2] == 0x00 && pki_n[3] == 0x00),
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid modulus MSW."));

        EXCEPTION_ASSERT_WITH_LOG(eLen >= 0x04 && eLen <= 0x100 && (eLen % 4) == 0 && eLen <= nLen,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid exponent length."));

        EXCEPTION_ASSERT_WITH_LOG((pki_e.back() & 0x01) != 0,
            LibLogicalAccessException, sam::errorMessage(__func__, "Exponent must be odd."));
    }

    if (hasPrivateKey)
    {
        EXCEPTION_ASSERT_WITH_LOG(!pki_p.empty() && !pki_q.empty() && !pki_dP.empty() && !pki_dQ.empty() && !pki_ipq.empty(),
            LibLogicalAccessException, sam::errorMessage(__func__, "Incomplete CRT key."));

        EXCEPTION_ASSERT_WITH_LOG(pLen >= 0x04 && pLen <= 0xF8,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid p length."));

        EXCEPTION_ASSERT_WITH_LOG(qLen >= 0x04 && qLen <= 0xF8,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid q length."));

        EXCEPTION_ASSERT_WITH_LOG(pki_dP.size() == pLen,
            LibLogicalAccessException, sam::errorMessage(__func__, "dP length mismatch (must equal p length)."));

        EXCEPTION_ASSERT_WITH_LOG(pki_dQ.size() == qLen,
            LibLogicalAccessException, sam::errorMessage(__func__, "dQ length mismatch (must equal q length)."));

        EXCEPTION_ASSERT_WITH_LOG(pki_ipq.size() == qLen,
            LibLogicalAccessException, sam::errorMessage(__func__, "ipq length mismatch (must equal q length)."));

        EXCEPTION_ASSERT_WITH_LOG(pki_p[0] != 0x00 && pki_q[0] != 0x00,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid CRT prime MSB."));

        const std::size_t nWords = (nLen + 3) / 4;
        const std::size_t pWords = (pLen + 3) / 4;
        const std::size_t qWords = (qLen + 3) / 4;

        EXCEPTION_ASSERT_WITH_LOG(pWords + 2 <= nWords && qWords + 2 <= nWords,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid CRT size relation."));
    }

    unsigned short effectiveConfig = configSettings;

    if (includeAccess)
    {
        EXCEPTION_ASSERT_WITH_LOG(accessKeys,
            LibLogicalAccessException, sam::errorMessage(__func__, "AEK/VAEK required."));

        if ((effectiveConfig & sam::pki::ConfigDisableBit) != 0)
            LOG(LogLevel::WARNINGS) << sam::errorMessage(__func__, "Overriding PKI_SET disable bit due to AEK.");
        effectiveConfig &= static_cast<unsigned short>(~sam::pki::ConfigDisableBit);
    }

    const unsigned char p1 = static_cast<unsigned char>((updateSettingsOnly ? 0x01 : 0x00) | (includeAccess ? 0x02 : 0x00));

    ByteVector payload;
    payload.reserve(6 + (includeAccess ? 2 : 0) +
        (!updateSettingsOnly ? 2 + nLen + 2 + eLen + (hasPrivateKey ? 2 + pLen + 2 + qLen + pLen + qLen + qLen : 0) : 0));
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
            sam::appendUInt16BE(payload, static_cast<uint16_t>(pLen));
            sam::appendUInt16BE(payload, static_cast<uint16_t>(qLen));
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

    const sam::ApduFormat format = getApduFormat(payload.size(), false);
    const unsigned char lc = (format == sam::ApduFormat::SingleFrame ? static_cast<unsigned char>(payload.size()) : 0x00);
    ByteVector apdu{d_cla, sam::ins::pki::ImportKey, p1, 0x00, lc};
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = executeProtectedExchange(apdu, format);
    validateSuccessResponse(response, __func__);
}

ByteVector SAMAV2ISO7816Commands::PKI_ExportPrivateKey(unsigned char keyNo, bool returnAEK)
{
    EXCEPTION_ASSERT_WITH_LOG(keyNo <= 0x01,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid key reference number."));

    const unsigned char returnAekFlag = static_cast<unsigned char>(returnAEK ? 0x80 : 0x00);
    ByteVector apdu{d_cla, sam::ins::pki::ExportPrivateKey, keyNo, returnAekFlag, sam::iso7816::LeResponse};
    ByteVector response = executeProtectedExchange(apdu);
    validateSuccessResponse(response, __func__);

    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    return response;
}

ByteVector SAMAV2ISO7816Commands::PKI_ExportPublicKey(unsigned char keyNo, bool returnAEK)
{
    EXCEPTION_ASSERT_WITH_LOG(keyNo <= 0x02,
        LibLogicalAccessException, sam::errorMessage(__func__, "Key number out of range."));

    const unsigned char returnAekFlag = static_cast<unsigned char>(returnAEK ? 0x80 : 0x00);
    ByteVector apdu{d_cla, sam::ins::pki::ExportPublicKey, keyNo, returnAekFlag, sam::iso7816::LeResponse};
    ByteVector response = executeProtectedExchange(apdu);
    validateSuccessResponse(response, __func__);

    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    return response;
}

ByteVector SAMAV2ISO7816Commands::buildPlaintext(std::uint16_t changeCtr,
    const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries)
{
    constexpr std::size_t HEADER_SIZE = 2;
    constexpr std::size_t MAX_KEY_ENTRY_SIZE = 64;

    ByteVector pt;
    pt.reserve(HEADER_SIZE + entries.size() * (HEADER_SIZE + MAX_KEY_ENTRY_SIZE));
    sam::appendUInt16BE(pt, changeCtr);

    for (const auto &entry : entries)
    {
        EXCEPTION_ASSERT_WITH_LOG(entry,
            LibLogicalAccessException, sam::errorMessage(__func__, "Null SAMBasicKeyEntry."));

        const unsigned char keyNo = entry->getKeyEntryNumber();
        const unsigned char updateMask = entry->getUpdateMask();
        const ByteVector newEntry = entry->serializeKSTKeyEntry();

        pt.push_back(keyNo);
        pt.push_back(updateMask);
        pt.insert(pt.end(), newEntry.begin(), newEntry.end());
    }
    return pt;
}

ByteVector SAMAV2ISO7816Commands::rsa_oaep_encrypt(EVP_PKEY *pubKey, const ByteVector &plaintext, const EVP_MD *md)
{
    EXCEPTION_ASSERT_WITH_LOG(pubKey,
        LibLogicalAccessException, sam::errorMessage(__func__, "Public key is null."));

    EXCEPTION_ASSERT_WITH_LOG(md,
        LibLogicalAccessException, sam::errorMessage(__func__, "Hashing algorithm is null."));

    using EVP_PKEY_CTX_ptr = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;

    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new(pubKey, nullptr), EVP_PKEY_CTX_free);

    EXCEPTION_ASSERT_WITH_LOG(ctx,
        LibLogicalAccessException, sam::errorMessage(__func__, "Failed to create EVP_PKEY_CTX."));

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_encrypt_init(ctx.get()) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "EVP_PKEY_encrypt_init failed."));

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "Failed to set RSA OAEP padding."));

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_CTX_set_rsa_oaep_md(ctx.get(), md) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "Failed to set OAEP hash algorithm."));

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_CTX_set_rsa_mgf1_md(ctx.get(), md) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "Failed to set MGF1 hash algorithm."));

    std::size_t outLen = 0;

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_encrypt(ctx.get(), nullptr, &outLen, plaintext.data(), plaintext.size()) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "Failed to determine ciphertext length."));

    ByteVector out(outLen);

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_encrypt(ctx.get(), out.data(), &outLen, plaintext.data(), plaintext.size()) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "RSA OAEP encryption failed."));

    out.resize(outLen);
    return out;
}

ByteVector SAMAV2ISO7816Commands::rsa_pss_sign(EVP_PKEY *privKey, const ByteVector &data, const EVP_MD *md)
{
    EXCEPTION_ASSERT_WITH_LOG(privKey,
        LibLogicalAccessException, sam::errorMessage(__func__, "Private key is null."));

    EXCEPTION_ASSERT_WITH_LOG(md,
        LibLogicalAccessException, sam::errorMessage(__func__, "Hashing algorithm is null."));
    
    using EVP_MD_CTX_ptr = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

    EVP_MD_CTX_ptr ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);

    EXCEPTION_ASSERT_WITH_LOG(ctx,
        LibLogicalAccessException, sam::errorMessage(__func__, "Failed to create EVP_MD_CTX."));

    EVP_PKEY_CTX *pkeyCtx = nullptr;

    EXCEPTION_ASSERT_WITH_LOG(EVP_DigestSignInit(ctx.get(), &pkeyCtx, md, nullptr, privKey) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "EVP_DigestSignInit failed."));

    EXCEPTION_ASSERT_WITH_LOG(pkeyCtx,
        LibLogicalAccessException, sam::errorMessage(__func__, "Missing RSA signing context."));

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_CTX_set_rsa_padding(pkeyCtx, RSA_PKCS1_PSS_PADDING) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "Failed to set RSA PSS padding."));

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_CTX_set_rsa_mgf1_md(pkeyCtx, md) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "Failed to configure MGF1 hash algorithm."));

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_CTX_set_rsa_pss_saltlen(pkeyCtx, EVP_MD_size(md)) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "Failed to configure RSA PSS salt length."));

    EXCEPTION_ASSERT_WITH_LOG(EVP_DigestSignUpdate(ctx.get(), data.data(), data.size()) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "EVP_DigestSignUpdate failed."));

    std::size_t sigLen = 0;

    EXCEPTION_ASSERT_WITH_LOG(EVP_DigestSignFinal(ctx.get(), nullptr, &sigLen) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "Failed to determine signature length."));

    ByteVector sig(sigLen);

    EXCEPTION_ASSERT_WITH_LOG(EVP_DigestSignFinal(ctx.get(), sig.data(), &sigLen) > 0,
        LibLogicalAccessException, sam::errorMessage(__func__, "RSA PSS signature generation failed."));

    sig.resize(sigLen);
    return sig;
}

const EVP_MD *SAMAV2ISO7816Commands::getHash(sam::HashAlgo hashAlgo)
{
    switch (hashAlgo)
    {
    case sam::HashAlgo::SHA1: return EVP_sha1();
    case sam::HashAlgo::SHA224: return EVP_sha224();
    case sam::HashAlgo::SHA256: return EVP_sha256();
    default:
        EXCEPTION_ASSERT_WITH_LOG(false,
            LibLogicalAccessException, sam::errorMessage(__func__, "Unsupported hash algorithm."));
    }
    return nullptr; // Unreachable but avoids compiler warning
}

void SAMAV2ISO7816Commands::buildCryptogram(
    EVP_PKEY *encKey, EVP_PKEY *signKey, std::uint8_t keyNoEnc, std::uint8_t keyNoSign,
    std::uint16_t changeCtr, const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries,
    std::uint8_t hashAlgo, ByteVector &encFrame, ByteVector &signature)
{
    EXCEPTION_ASSERT_WITH_LOG(sam::isSupportedHashAlgo(hashAlgo),
        LibLogicalAccessException, sam::errorMessage(__func__, "Unsupported or RFU hash algorithm."));

    const EVP_MD *md = getHash(static_cast<sam::HashAlgo>(hashAlgo));

    EXCEPTION_ASSERT_WITH_LOG(md,
        LibLogicalAccessException, sam::errorMessage(__func__, "Failed to resolve hash algorithm."));

    // plaintext
    ByteVector plaintext = buildPlaintext(changeCtr, entries);

    // encryption
    encFrame = rsa_oaep_encrypt(encKey, plaintext, md);

    // signature
    ByteVector toSign;
    toSign.reserve(2 + encFrame.size());
    toSign.push_back(keyNoEnc);
    toSign.push_back(keyNoSign);
    toSign.insert(toSign.end(), encFrame.begin(), encFrame.end());
    signature = rsa_pss_sign(signKey, toSign, md);
}

ByteVector SAMAV2ISO7816Commands::PKI_UpdateKeyEntries(
    const ByteVector &encPublicKeyDer, const ByteVector &signPrivateKeyDer, unsigned char keyNoEnc,
    unsigned char keyNoSign, bool requestAck, unsigned char keyNoAck, unsigned char hashAlgo,
    const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries, std::uint16_t changeCounter)
{
    EXCEPTION_ASSERT_WITH_LOG(!entries.empty() && entries.size() <= 3,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid number of entries."));

    auto encKey  = loadPublicKeyFromDER(encPublicKeyDer);
    auto signKey = loadPrivateKeyFromDER(signPrivateKeyDer);

    ByteVector encryptedFrame;
    ByteVector signature;

    buildCryptogram(encKey.get(), signKey.get(), keyNoEnc, keyNoSign, changeCounter, entries, hashAlgo, encryptedFrame, signature);

    return PKI_UpdateKeyEntries(keyNoEnc, keyNoSign, requestAck, keyNoAck, hashAlgo,
                                static_cast<unsigned char>(entries.size()), encryptedFrame, signature);
}

ByteVector SAMAV2ISO7816Commands::PKI_UpdateKeyEntries(
    unsigned char keyNoEnc, unsigned char keyNoSign, bool requestAck, unsigned char keyNoAck,
    unsigned char hashAlgo, unsigned char nbKeyEntries, const ByteVector &encKeyFrame,
    const ByteVector &signature)
{
    constexpr unsigned char MAX_HASH_ALGO   = 0x03;
    constexpr unsigned char MAX_KEY_ENTRIES = 0x03;
    constexpr unsigned char MAX_ENC_KEY_NO  = 0x01;
    constexpr unsigned char MAX_SIGN_KEY_NO = 0x02;
    constexpr unsigned char MAX_ACK_KEY_NO  = 0x01;

    EXCEPTION_ASSERT_WITH_LOG(hashAlgo <= MAX_HASH_ALGO,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid hash algorithm (must be 0...3)."));

    EXCEPTION_ASSERT_WITH_LOG(nbKeyEntries >= 1 && nbKeyEntries <= MAX_KEY_ENTRIES,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid number of key entries (must be 1...3)."));

    EXCEPTION_ASSERT_WITH_LOG(keyNoEnc <= MAX_ENC_KEY_NO,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid encryption key number."));

    EXCEPTION_ASSERT_WITH_LOG(keyNoSign <= MAX_SIGN_KEY_NO,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid signing key number."));

    EXCEPTION_ASSERT_WITH_LOG(!requestAck || keyNoAck <= MAX_ACK_KEY_NO,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid acknowledgment key number."));

    EXCEPTION_ASSERT_WITH_LOG(!encKeyFrame.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Encrypted key frame cannot be empty."));

    EXCEPTION_ASSERT_WITH_LOG(!signature.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Signature cannot be empty."));

    const unsigned char p1 = static_cast<unsigned char>((hashAlgo & 0x03) | ((nbKeyEntries & 0x03) << 2));

    ByteVector payload;
    payload.reserve(2 + (requestAck ? 1 : 0) + encKeyFrame.size() + signature.size());
    payload.push_back(keyNoEnc);
    payload.push_back(keyNoSign);
    if (requestAck)
        payload.push_back(keyNoAck);
    payload.insert(payload.end(), encKeyFrame.begin(), encKeyFrame.end());
    payload.insert(payload.end(), signature.begin(), signature.end());

    const sam::ApduFormat format = getApduFormat(payload.size(), requestAck);
    const unsigned char lc = (format == sam::ApduFormat::SingleFrame ? static_cast<unsigned char>(payload.size()) : 0x00);
    ByteVector apdu;
    apdu.reserve(payload.size() + (requestAck ? sam::APDU_HEADER_WITH_LE_SIZE : sam::APDU_HEADER_SIZE));
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::UpdateKeyEntries);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());
    if (requestAck)
        apdu.push_back(sam::iso7816::LeResponse);

    ByteVector response = executeProtectedExchange(apdu, format);
    validateSuccessResponse(response, __func__);

    if (!requestAck)
        return {};

    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    return response;
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
        LibLogicalAccessException, sam::errorMessage(__func__, "DivInput must be empty or between 1 and 31 bytes."));

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
    payload.reserve(6u + keyEntries.size() * 2u + divInput.size());
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

    const unsigned char lc = static_cast<unsigned char>(payload.size());
    ByteVector apdu{d_cla, sam::ins::pki::EncipherKeyEntries, p1, 0x00, lc};
    apdu.insert(apdu.end(), payload.begin(), payload.end());
    apdu.push_back(sam::iso7816::LeResponse);

    ByteVector response = executeProtectedExchange(apdu);
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

    const std::uint32_t messageLen = static_cast<std::uint32_t>(message.size());
    const std::size_t payloadSize  = sizeof(messageLen) + message.size();
    
    const sam::ApduFormat format = getApduFormat(payloadSize, true);
    const unsigned char lc = (format == sam::ApduFormat::SingleFrame ? static_cast<unsigned char>(payloadSize) : 0x00);

    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_WITH_LE_SIZE + payloadSize);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::GenerateHash);
    apdu.push_back(hashAlgo);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    sam::appendUInt32BE(apdu, messageLen);
    apdu.insert(apdu.end(), message.begin(), message.end());
    apdu.push_back(sam::iso7816::LeResponse);

    ByteVector response = executeProtectedExchange(apdu, format);
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

    const std::size_t expectedSize = sam::expectedHashSize(hashAlgo);

    EXCEPTION_ASSERT_WITH_LOG(hash.size() == expectedSize,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid hash size for selected algorithm."));

    const unsigned char lc = static_cast<unsigned char>(1u + hash.size());

    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_SIZE + lc);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::GenerateSignature);
    apdu.push_back(hashAlgo);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.push_back(keyNoSign);
    apdu.insert(apdu.end(), hash.begin(), hash.end());

    const ByteVector response = executeProtectedExchange(apdu);
    validateSuccessResponse(response, __func__);
}

ByteVector SAMAV2ISO7816Commands::PKI_SendSignature()
{
    constexpr std::size_t MIN_SIG_SIZE = 8;
    constexpr std::size_t MAX_SIG_SIZE = 256;

    const ByteVector apdu = {d_cla, sam::ins::pki::SendSignature, 0x00, 0x00, sam::iso7816::LeResponse};
    ByteVector signature = executeProtectedExchange(apdu);
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
    payload.reserve(1u + hash.size() + signature.size());
    payload.push_back(keyNoVerif);
    payload.insert(payload.end(), hash.begin(), hash.end());
    payload.insert(payload.end(), signature.begin(), signature.end());

    const sam::ApduFormat format = getApduFormat(payload.size(), false);
    const unsigned char lc = (format == sam::ApduFormat::SingleFrame ? static_cast<unsigned char>(payload.size()) : 0x00);
    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_SIZE + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::VerifySignature);
    apdu.push_back(hashAlgo);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = executeProtectedExchange(apdu, format);
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
    const std::size_t payloadSize = 1u + plainData.size();
    const sam::ApduFormat format = getApduFormat(payloadSize, true);
    const unsigned char lc = (format == sam::ApduFormat::SingleFrame ? static_cast<unsigned char>(payloadSize) : 0x00);

    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_WITH_LE_SIZE + payloadSize);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::EncipherData);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.push_back(keyNoEnc);
    apdu.insert(apdu.end(), plainData.begin(), plainData.end());
    apdu.push_back(sam::iso7816::LeResponse);

    ByteVector encData = executeProtectedExchange(apdu, format);
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
    const std::size_t payloadSize = 1u + encData.size();
    const sam::ApduFormat format = getApduFormat(payloadSize, true);
    const unsigned char lc = (format == sam::ApduFormat::SingleFrame ? static_cast<unsigned char>(payloadSize) : 0x00);

    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_WITH_LE_SIZE + payloadSize);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::DecipherData);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.push_back(keyNoDec);
    apdu.insert(apdu.end(), encData.begin(), encData.end());
    apdu.push_back(sam::iso7816::LeResponse);

    ByteVector plainData = executeProtectedExchange(apdu, format);
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

    constexpr std::size_t PKI_IMPORT_ECC_MAX_PAYLOAD = 0x4B;

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

        const std::size_t keySize = eccPublicKey.size();

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
    apdu.reserve(sam::APDU_HEADER_SIZE + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::ImportECCKey);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = executeProtectedExchange(apdu);
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
    payload.reserve(3u + eccCurve.size());
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

        constexpr std::size_t ECC_CURVE_HEADER_SIZE = 2u;
        const std::size_t expectedSize = ECC_CURVE_HEADER_SIZE +
            (5u * static_cast<std::size_t>(eccN)) + static_cast<std::size_t>(eccM);

        EXCEPTION_ASSERT_WITH_LOG(eccCurve.size() == expectedSize,
            LibLogicalAccessException, sam::errorMessage(__func__, "ECC curve length mismatch."));

        std::size_t offset = ECC_CURVE_HEADER_SIZE;

        const auto checkBlock = [&](std::size_t length, const char *name)
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

    EXCEPTION_ASSERT_WITH_LOG(payload.size() <= sam::MAX_SECURE_APDU_DATA_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "APDU payload too large."));

    const unsigned char lc = static_cast<unsigned char>(payload.size());

    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_SIZE + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::ImportECCCurve);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = executeProtectedExchange(apdu);
    validateSuccessResponse(response, __func__);
}

ByteVector SAMAV2ISO7816Commands::PKI_ExportEccPublicKey(unsigned char keyNo)
{
    EXCEPTION_ASSERT_WITH_LOG(keyNo <= 0x07,
        LibLogicalAccessException, sam::errorMessage(__func__, "Key number out of range (0x00...0x07)."));

    ByteVector apdu{d_cla, sam::ins::pki::ExportECCPublicKey, keyNo, 0x00, sam::iso7816::LeResponse};

    ByteVector response = executeProtectedExchange(apdu);
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
    payload.reserve(3u + message.size() + signature.size());
    payload.push_back(keyNo);
    payload.push_back(curveNo);
    payload.push_back(static_cast<unsigned char>(message.size()));
    payload.insert(payload.end(), message.begin(), message.end());
    payload.insert(payload.end(), signature.begin(), signature.end());

    EXCEPTION_ASSERT_WITH_LOG(payload.size() <= sam::MAX_SECURE_APDU_DATA_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "APDU payload too large."));

    const unsigned char lc = static_cast<unsigned char>(payload.size());

    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_SIZE + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::pki::VerifyECCSignature);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = executeProtectedExchange(apdu);
    validateSuccessResponse(response, __func__);
}

}
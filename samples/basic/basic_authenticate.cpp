/**
 * \file basic_authenticate.cpp
 * \brief DUOX certificate + ISOGeneralAuthenticate functional test
 *
 * This test intentionally remains an integration/functional test rather than a unit-test abstraction.
 * The goal is to preserve exhaustive hardware-level validation while making ownership, lifecycle, invariants
 * and diagnostics explicit
 */

#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>

#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/cards/duox/duoxcommands.hpp>
#include <logicalaccess/plugins/cards/duox/duoxCertificateValidator.hpp>
#include <logicalaccess/plugins/crypto/x509V3Certificate.hpp>

#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{

// ==============================================
// Configuration
// ==============================================

namespace config
{

constexpr char READER_PROVIDER[] = "PCSC";
constexpr char DESFIRE_READER_NAME[] = "HID Global OMNIKEY 5422CL Smartcard Reader 0";

constexpr unsigned int CARD_INSERTION_TIMEOUT_MS = 15000;

// Safety invariant : application tests must never operate at PICC level
constexpr std::uint32_t PICC_AID = 0x000000;
constexpr std::uint32_t TEST_AID = 0x123456;

// AES keys used exclusively for card/application provisioning
constexpr std::uint8_t PICC_MASTER_KEY        = 0;
constexpr std::uint8_t APPLICATION_MASTER_KEY = 0;

// DUOX ECC authentication key
constexpr std::uint8_t CARD_ECC_KEY_NO = 0;

// DUOX CA Root Key
constexpr std::uint8_t CA_ROOT_KEY_NO = 0;

// Certificate file
constexpr std::uint8_t CERTIFICATE_FILE_NO = 1;

// NIST P-256 sizes
constexpr std::size_t P256_PUBLIC_KEY_SIZE  = 65;
constexpr std::size_t P256_PRIVATE_KEY_SIZE = 32;

// DUOX certificate size limit
constexpr std::size_t DUOX_MAX_CERTIFICATE_SIZE = 880;

// Test certificate properties
constexpr unsigned int CA_VALIDITY_DAYS     = 3650;
constexpr unsigned int DEVICE_VALIDITY_DAYS = 365;

constexpr std::uint32_t CA_SERIAL     = 100;
constexpr std::uint32_t DEVICE_SERIAL = 1;

constexpr char CA_COMMON_NAME[]     = "DUOX TEST CA";
constexpr char DEVICE_COMMON_NAME[] = "DUOX TEST DEVICE";

const ByteVector CA_ISSUER{'D', 'U', 'O', 'X', ' ', 'T', 'E', 'S', 'T', ' ', 'C', 'A'};

// Certificate file access configuration
constexpr logicalaccess::DESFireAccessRights DUOX_TEST_CERTIFICATE_FILE_ACCESS_RIGHTS
    {logicalaccess::AR_FREE, logicalaccess::AR_FREE, logicalaccess::AR_FREE, logicalaccess::AR_FREE};

// ManageKeyPair configuration
constexpr std::uint16_t ECC_KEY_POLICY = (1U << 7); // ECC-based Mutual Authentication
constexpr std::uint32_t KUC_LIMIT = 0;

constexpr auto COMM_MODE = logicalaccess::DUOXCommunicationMode::Full;

constexpr std::uint8_t ACCESS_RIGHTS     = 0x0E;
constexpr std::uint16_t CA_ACCESS_RIGHTS = 0x0000;
constexpr std::uint8_t COMM_MODE_BITS    = static_cast<std::uint8_t>(COMM_MODE) << 4;
constexpr std::uint8_t ECC_WRITE_ACCESS  = ACCESS_RIGHTS | COMM_MODE_BITS;
constexpr std::uint8_t CA_WRITE_ACCESS   = ACCESS_RIGHTS | COMM_MODE_BITS;
constexpr std::uint8_t CA_READ_ACCESS    = ACCESS_RIGHTS | COMM_MODE_BITS;

constexpr std::uint8_t CA_CRL_FILE      = 0;
constexpr std::uint32_t CA_CRL_FILE_AID = PICC_AID;

} // namespace config

// ==============================================
// Test modes
// ==============================================

enum class ReaderCertificateMode
{
    WithCertA,
    WithoutCertA
};

// Select exactly one diagnostic mode per execution
constexpr ReaderCertificateMode TEST_READER_CERTIFICATE_MODE = ReaderCertificateMode::WithCertA;

// ==============================================
// Generic utilities
// ==============================================

[[noreturn]] void fail(const std::string &message)
{
    throw std::runtime_error(message);
}

template <typename T>
void require(const T &condition, const std::string &message)
{
    if (!static_cast<bool>(condition))
        throw std::runtime_error(message);
}

template <typename T>
void requireNotNull(const std::shared_ptr<T> &value, const char *description)
{
    if (!value)
        fail(std::string(description) + " is null.");
}

std::string byteToHex(std::uint8_t value)
{
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(value);
    return stream.str();
}

std::string hexDump(const ByteVector &data)
{
    std::ostringstream stream;

    for (std::size_t i = 0; i < data.size(); ++i)
    {
        if (i != 0)
            stream << ' ';
        stream << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(data[i]);
    }

    return stream.str();
}

std::string formatAid(std::uint32_t aid)
{
    std::ostringstream stream;
    stream << std::uppercase << std::hex << std::setw(6) << std::setfill('0') << aid;
    return stream.str();
}

const char *readerCertificateModeName(ReaderCertificateMode mode)
{
    switch (mode)
    {
    case ReaderCertificateMode::WithCertA: return "WITH Cert.A";
    case ReaderCertificateMode::WithoutCertA: return "WITHOUT Cert.A";
    }
    return "<invalid>";
}

bool certificateExpected(ReaderCertificateMode mode)
{
    switch (mode)
    {
    case ReaderCertificateMode::WithCertA: return true;
    case ReaderCertificateMode::WithoutCertA: return false;
    }
    fail("Invalid ReaderCertificateMode.");
}

// ==============================================
// Output
// ==============================================

namespace output
{

void printSection(const std::string &title)
{
    std::cout << "\n========================================\n"
              << ' ' << title << '\n'
              << "========================================\n";
}

void printInfo(const std::string &message)
{
    std::cout << "[INFO] " << message << '\n';
}

void printPass(const std::string &message)
{
    std::cout << "[PASS] " << message << '\n';
}

void printWarning(const std::string &message)
{
    std::cout << "[WARNING] " << message << '\n';
}

void printError(const std::string &message)
{
    std::cerr << "[ERROR] " << message << '\n';
}

void printPlain(const std::string &message)
{
    std::cout << message << '\n';
}

} // namespace output

// ============================================================================
// Test configuration validation
// ============================================================================

void validateTestConfiguration()
{
    require(config::TEST_AID != config::PICC_AID, "FATAL safety failure : TEST_AID must never equal PICC_AID.");
    require(config::DUOX_MAX_CERTIFICATE_SIZE > 0, "DUOX certificate size limit must be non-zero.");
    require(config::P256_PUBLIC_KEY_SIZE == 65, "Unexpected P-256 public key size configuration.");
    require(config::P256_PRIVATE_KEY_SIZE == 32, "Unexpected P-256 private key size configuration.");
    require(config::CERTIFICATE_FILE_NO != 0, "Certificate file number must not be zero for this test.");
}

// ============================================================================
// OpenSSL RAII
// ============================================================================

template <typename T, void (*Deleter)(T *)>
class OpenSSLHandle
{
  public:
    OpenSSLHandle() noexcept = default;

    explicit OpenSSLHandle(T *value) noexcept
        : value_(value)
    {
    }

    ~OpenSSLHandle()
    {
        reset();
    }

    OpenSSLHandle(const OpenSSLHandle &)            = delete;
    OpenSSLHandle &operator=(const OpenSSLHandle &) = delete;

    OpenSSLHandle(OpenSSLHandle &&other) noexcept
        : value_(other.value_)
    {
        other.value_ = nullptr;
    }

    OpenSSLHandle &operator=(OpenSSLHandle &&other) noexcept
    {
        if (this == &other)
            return *this;

        reset();

        value_       = other.value_;
        other.value_ = nullptr;

        return *this;
    }

    T *get() const noexcept
    {
        return value_;
    }

    explicit operator bool() const noexcept
    {
        return value_ != nullptr;
    }

    void reset(T *value = nullptr) noexcept
    {
        if (value_)
            Deleter(value_);
        value_ = value;
    }

  private:
    T *value_ = nullptr;
};

using EVPKey = OpenSSLHandle<EVP_PKEY, EVP_PKEY_free>;
using X509Handle = OpenSSLHandle<X509, X509_free>;
using X509ExtensionHandle = OpenSSLHandle<X509_EXTENSION, X509_EXTENSION_free>;
using EVPContext = OpenSSLHandle<EVP_PKEY_CTX, EVP_PKEY_CTX_free>;
using BIGNUMHandle = OpenSSLHandle<BIGNUM, BN_free>;
using ParamBuilderHandle = OpenSSLHandle<OSSL_PARAM_BLD, OSSL_PARAM_BLD_free>;
using ParamHandle = OpenSSLHandle<OSSL_PARAM, OSSL_PARAM_free>;

// ==============================================
// OpenSSL error handling
// ==============================================

std::string opensslErrorString()
{
    std::ostringstream stream;
    bool first = true;

    while (const unsigned long error = ERR_get_error())
    {
        char buffer[256] = {};

        ERR_error_string_n(error, buffer, sizeof(buffer));

        if (!first)
            stream << " | ";

        stream << buffer;
        first = false;
    }

    return first ? "No OpenSSL error available" : stream.str();
}

[[noreturn]] void throwOpenSSL(const std::string &operation)
{
    fail(operation + " : " + opensslErrorString());
}

void checkOpenSSLResult(int result, const char *operation)
{
    if (result != 1)
        throwOpenSSL(operation);
}

// ==============================================
// DESFire / DUOX object acquisition
// ==============================================

// TODO recreate a DUOXTestContext class later to contain set of commands as std::shared_ptr

std::shared_ptr<logicalaccess::DESFireCommands> getDESFireCommands(const std::shared_ptr<logicalaccess::Chip> &chip)
{
    requireNotNull(chip, "Chip");
    auto commands = std::dynamic_pointer_cast<logicalaccess::DESFireCommands>(chip->getCommands());
    requireNotNull(commands, "DESFireCommands");
    return commands;
}

std::shared_ptr<logicalaccess::DUOXCommands> getDUOXCommands(const std::shared_ptr<logicalaccess::Chip> &chip)
{
    requireNotNull(chip, "Chip");
    auto commands = std::dynamic_pointer_cast<logicalaccess::DUOXCommands>(chip->getCommands());
    requireNotNull(commands, "DUOXCommands");
    return commands;
}

std::shared_ptr<logicalaccess::DESFireChip> getDESFireChip(const std::shared_ptr<logicalaccess::Chip> &chip)
{
    requireNotNull(chip, "Chip");
    auto desfireChip = std::dynamic_pointer_cast<logicalaccess::DESFireChip>(chip);
    requireNotNull(desfireChip, "DESFireChip");
    return desfireChip;
}

// ==============================================
// DESFire state validation
// ==============================================

void assertCurrentAid(const std::shared_ptr<logicalaccess::Chip> &chip, std::uint32_t expectedAid)
{
    const auto desfireChip = getDESFireChip(chip);
    const auto crypto      = desfireChip->getCrypto();

    if (!crypto)
        fail("DESFire crypto context is unavailable.");

    if (crypto->d_currentAid != expectedAid)
    {
        std::ostringstream message;
        message << "Wrong current AID. Expected 0x" << formatAid(expectedAid)
                << ", actual 0x" << formatAid(crypto->d_currentAid);
        fail(message.str());
    }
}

void assertTestApplicationSafety()
{
    require(config::TEST_AID != config::PICC_AID, "Safety failure : test application must never target PICC level.");
}

// ==============================================
// DESFire key helpers
// ==============================================

std::shared_ptr<logicalaccess::DESFireKey> createZeroAESKey()
{
    auto key = std::make_shared<logicalaccess::DESFireKey>();
    key->setKeyType(logicalaccess::DF_KEY_AES);
    key->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
    return key;
}

// ==============================================
// Card/application helpers
// ==============================================

void selectPICC(const std::shared_ptr<logicalaccess::DESFireCommands> &desfire)
{
    requireNotNull(desfire, "DESFireCommands");
    desfire->selectApplication(config::PICC_AID);
}

void selectTestApplication(const std::shared_ptr<logicalaccess::DESFireCommands> &desfire,
                           const std::shared_ptr<logicalaccess::Chip> &chip)
{
    requireNotNull(desfire, "DESFireCommands");
    assertTestApplicationSafety();
    desfire->selectApplication(config::TEST_AID);
    assertCurrentAid(chip, config::TEST_AID);
    output::printPass("Dedicated DUOX test application selected.");
}

void authenticatePICC(const std::shared_ptr<logicalaccess::DUOXCommands> &duox)
{
    requireNotNull(duox, "DUOXCommands");
    assertTestApplicationSafety();
    output::printInfo("Authenticating PICC Master Key.");
    duox->authenticateEV2First(config::PICC_MASTER_KEY, createZeroAESKey());
    output::printPass("PICC Master Key authenticated.");
}

void authenticateApplication(const std::shared_ptr<logicalaccess::DUOXCommands> &duox,
                             const std::shared_ptr<logicalaccess::Chip> &chip)
{
    requireNotNull(duox, "DUOXCommands");
    assertTestApplicationSafety();
    output::printInfo("Authenticating application Master Key.");
    duox->authenticateEV2First(config::APPLICATION_MASTER_KEY, createZeroAESKey());
    assertCurrentAid(chip, config::TEST_AID);
    output::printPass("Application Master Key authenticated.");
}

std::vector<unsigned int> getApplications(const std::shared_ptr<logicalaccess::DESFireCommands> &desfire)
{
    requireNotNull(desfire, "DESFireCommands");
    return desfire->getApplicationIDs();
}

bool applicationExists(const std::vector<unsigned int> &applications, std::uint32_t aid)
{
    return std::find(applications.begin(), applications.end(), aid) != applications.end();
}

bool testApplicationExists(const std::shared_ptr<logicalaccess::DESFireCommands> &desfire)
{
    return applicationExists(getApplications(desfire), config::TEST_AID);
}

// ==============================================
// Card preparation
// ==============================================

enum class CardPreparationMode
{
    EraseCard,
    DeleteTestApplication
};

constexpr CardPreparationMode CARD_PREPARATION_MODE = CardPreparationMode::EraseCard;

// Required to release NV-Memory
void eraseCard(const std::shared_ptr<logicalaccess::DESFireCommands> &desfire,
               const std::shared_ptr<logicalaccess::DUOXCommands> &duox)
{
    requireNotNull(desfire, "DESFireCommands");
    requireNotNull(duox, "DUOXCommands");

    assertTestApplicationSafety();

    output::printWarning("FULL PICC ERASE requested.");

    selectPICC(desfire);
    output::printPass("Authenticating PICC Master Key for test-application lifecycle.");
    authenticatePICC(duox);
    desfire->erase();
    selectPICC(desfire);

    const auto applications = getApplications(desfire);
    require(applications.empty(), "PICC erase completed but applications are still reported.");

    output::printPass("PICC erased successfully.");
}

void deleteExistingTestApplication(
    const std::shared_ptr<logicalaccess::DESFireCommands> &desfire,
    const std::shared_ptr<logicalaccess::DUOXCommands> &duox)
{
    requireNotNull(desfire, "DESFireCommands");
    requireNotNull(duox, "DUOXCommands");
    assertTestApplicationSafety();

    selectPICC(desfire);

    if (!testApplicationExists(desfire))
    {
        output::printInfo("Dedicated test application does not exist.");
        return;
    }

    authenticatePICC(duox);

    output::printWarning("Deleting existing test application 0x" + formatAid(config::TEST_AID));
    desfire->deleteApplication(config::TEST_AID);

    require(!testApplicationExists(desfire), "Dedicated test application still exists after deletion.");

    output::printPass("Existing dedicated test application deleted.");
}

void prepareCard(const std::shared_ptr<logicalaccess::DESFireCommands> &desfire,
                 const std::shared_ptr<logicalaccess::DUOXCommands> &duox)
{
    assertTestApplicationSafety();
    output::printSection("DUOX CARD PREPARATION");

    switch (CARD_PREPARATION_MODE)
    {
    case CardPreparationMode::EraseCard: eraseCard(desfire, duox); return;

    case CardPreparationMode::DeleteTestApplication: deleteExistingTestApplication(desfire, duox); return;
    }

    fail("Unsupported card preparation mode.");
}

// ==============================================
// P-256 key helpers
// ==============================================

void assertP256PublicKey(const ByteVector &key, const char *description)
{
    require(key.size() == config::P256_PUBLIC_KEY_SIZE, std::string(description) + " must contain exactly 65 bytes.");
    require(key.front() == 0x04U, std::string(description) + " must be an uncompressed EC point.");
}

void assertPublicKeysEqual(const ByteVector &actual, const ByteVector &expected, const char *errorMessage)
{
    assertP256PublicKey(actual, "Actual public key");
    assertP256PublicKey(expected, "Expected public key");

    require(actual == expected, errorMessage);
}

EVPKey generateP256Key()
{
    EVP_PKEY_CTX *rawContext = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);

    if (!rawContext)
        throwOpenSSL("EVP_PKEY_CTX_new_id");

    EVPContext context(rawContext);

    checkOpenSSLResult(EVP_PKEY_keygen_init(context.get()),
        "EVP_PKEY_keygen_init");
    checkOpenSSLResult(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(context.get(), NID_X9_62_prime256v1),
        "EVP_PKEY_CTX_set_ec_paramgen_curve_nid");

    EVP_PKEY *rawKey = nullptr;
    checkOpenSSLResult(EVP_PKEY_keygen(context.get(), &rawKey),
        "EVP_PKEY_keygen");

    return EVPKey(rawKey);
}

void assertP256EVPKey(EVP_PKEY *key, const char *description)
{
    require(key != nullptr, std::string(description) + " is null.");
    require(EVP_PKEY_base_id(key) == EVP_PKEY_EC, std::string(description) + " is not an EC key.");

    char group[80]          = {};
    std::size_t groupLength = 0;

    checkOpenSSLResult(EVP_PKEY_get_utf8_string_param(key, OSSL_PKEY_PARAM_GROUP_NAME, group, sizeof(group), &groupLength),
                       "EVP_PKEY_get_utf8_string_param(group)");

    require(std::string(group, groupLength) == "prime256v1", std::string(description) + " is not a P-256 key.");
}

ByteVector extractP256PublicKey(EVP_PKEY *key)
{
    assertP256EVPKey(key, "EC key");

    std::size_t length = 0;

    checkOpenSSLResult(EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, nullptr, 0, &length),
                       "EVP_PKEY_get_octet_string_param(size)");

    require(length == config::P256_PUBLIC_KEY_SIZE, "Unexpected P-256 public key size.");

    ByteVector result(length);
    checkOpenSSLResult(EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, result.data(), result.size(), &length),
                       "EVP_PKEY_get_octet_string_param(public)");

    require(length == result.size(), "Unable to extract complete P-256 public key.");

    assertP256PublicKey(result, "Extracted P-256 public key");

    return result;
}

ByteVector extractP256PrivateKey(EVP_PKEY *key)
{
    assertP256EVPKey(key, "EC key");

    BIGNUM *rawPrivateKey = nullptr;
    checkOpenSSLResult( EVP_PKEY_get_bn_param(key, OSSL_PKEY_PARAM_PRIV_KEY, &rawPrivateKey),
        "EVP_PKEY_get_bn_param(private)");

    BIGNUMHandle privateKey(rawPrivateKey);
    require(BN_num_bytes(privateKey.get()) <= static_cast<int>(config::P256_PRIVATE_KEY_SIZE),
            "P-256 private key is larger than 32 bytes.");

    ByteVector result(config::P256_PRIVATE_KEY_SIZE, 0);

    const int written = BN_bn2binpad(privateKey.get(), result.data(), static_cast<int>(result.size()));
    require(written == static_cast<int>(result.size()), "Unable to serialize P-256 private key.");

    return result;
}

EVPKey makeP256PublicKey(const ByteVector &publicKey)
{
    assertP256PublicKey(publicKey, "Public key");

    EVP_PKEY_CTX *rawContext = EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr);

    if (!rawContext)
        throwOpenSSL("EVP_PKEY_CTX_new_from_name");

    EVPContext context(rawContext);

    checkOpenSSLResult(EVP_PKEY_fromdata_init(context.get()), "EVP_PKEY_fromdata_init");

    OSSL_PARAM_BLD *rawBuilder = OSSL_PARAM_BLD_new();

    if (!rawBuilder)
        throwOpenSSL("OSSL_PARAM_BLD_new");

    ParamBuilderHandle builder(rawBuilder);

    checkOpenSSLResult(OSSL_PARAM_BLD_push_utf8_string(builder.get(), OSSL_PKEY_PARAM_GROUP_NAME, "prime256v1", 0),
                       "OSSL_PARAM_BLD_push_utf8_string(group)");

    checkOpenSSLResult(OSSL_PARAM_BLD_push_octet_string(builder.get(), OSSL_PKEY_PARAM_PUB_KEY,
        publicKey.data(), publicKey.size()), "OSSL_PARAM_BLD_push_octet_string(public)");

    OSSL_PARAM *rawParams = OSSL_PARAM_BLD_to_param(builder.get());

    if (!rawParams)
        throwOpenSSL("OSSL_PARAM_BLD_to_param");

    ParamHandle params(rawParams);

    EVP_PKEY *rawKey = nullptr;

    const int result = EVP_PKEY_fromdata(context.get(), &rawKey, EVP_PKEY_PUBLIC_KEY, params.get());
    if (result != 1)
        throwOpenSSL("EVP_PKEY_fromdata");

    return EVPKey(rawKey);
}

// ==============================================
// X.509
// ==============================================

void setCertificateValidity(X509 *certificate, unsigned int validityDays)
{
    require(certificate != nullptr, "Certificate is null.");

    if (!X509_gmtime_adj(X509_get_notBefore(certificate), -60))
        throwOpenSSL("X509_gmtime_adj(notBefore)");

    const long seconds = static_cast<long>(validityDays) * 24L * 60L * 60L;
    if (!X509_gmtime_adj(X509_get_notAfter(certificate), seconds))
        throwOpenSSL("X509_gmtime_adj(notAfter)");
}

void setCommonName(X509_NAME *name, const char *commonName)
{
    require(name != nullptr, "X509 name is null.");
    require(commonName != nullptr, "Common name is null.");

    checkOpenSSLResult(X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
        reinterpret_cast<const unsigned char *>(commonName), -1, -1, 0), "X509_NAME_add_entry_by_txt(CN)");
}

void addBasicConstraints(X509 *issuer, X509 *subject, const char *value)
{
    require(subject != nullptr, "Subject certificate is null.");
    require(value != nullptr, "Basic constraints value is null.");

    X509V3_CTX context;
    X509V3_set_ctx_nodb(&context);
    X509V3_set_ctx(&context, issuer, subject, nullptr, nullptr, 0);

    X509_EXTENSION *rawExtension = X509V3_EXT_conf_nid(nullptr, &context, NID_basic_constraints, const_cast<char *>(value));

    if (!rawExtension)
        throwOpenSSL("X509V3_EXT_conf_nid");

    X509ExtensionHandle extension(rawExtension);

    checkOpenSSLResult(X509_add_ext(subject, extension.get(), -1), "X509_add_ext");
}

ByteVector encodeDER(X509 *certificate)
{
    require(certificate != nullptr, "Cannot encode null X.509 certificate.");

    const int size = i2d_X509(certificate, nullptr);
    if (size <= 0)
        throwOpenSSL("i2d_X509(size)");

    ByteVector result(static_cast<std::size_t>(size));
    unsigned char *cursor = result.data();

    if (i2d_X509(certificate, &cursor) != size)
        throwOpenSSL("i2d_X509");

    return result;
}

// ==============================================
// Test Certificate Authority
// ==============================================

class TestCA
{
  public:
    static TestCA create()
    {
        TestCA ca;

        ca.privateKey_ = generateP256Key();
        ca.createCertificate();

        ca.validateOwnCertificate();

        return ca;
    }

    TestCA() = default;

    TestCA(const TestCA &)                = delete;
    TestCA &operator=(const TestCA &)     = delete;
    TestCA(TestCA &&) noexcept            = default;
    TestCA &operator=(TestCA &&) noexcept = default;

    EVP_PKEY *privateKey() const noexcept
    {
        return privateKey_.get();
    }

    ByteVector publicKey() const
    {
        ensureInitialized();
        return extractP256PublicKey(privateKey_.get());
    }

    // \brief Return the self-signed CA certificate in DER format
    ByteVector certificateDER() const
    {
        ensureInitialized();
        return encodeDER(certificate_.get());
    }

    ByteVector issueCertificate(const ByteVector &subjectPublicKey) const
    {
        ensureInitialized();
        assertP256PublicKey(subjectPublicKey, "Subject public key");

        const EVPKey subjectKey = makeP256PublicKey(subjectPublicKey);
        X509Handle certificate(X509_new());
        if (!certificate)
            throwOpenSSL("X509_new(device)");

        X509 *x509 = certificate.get();
        checkOpenSSLResult(X509_set_version(x509, 2),
            "X509_set_version(device)");
        checkOpenSSLResult(ASN1_INTEGER_set(X509_get_serialNumber(x509), config::DEVICE_SERIAL),
            "ASN1_INTEGER_set(device)");

        setCertificateValidity(x509, config::DEVICE_VALIDITY_DAYS);

        X509_NAME *caSubject = X509_get_subject_name(certificate_.get());
        require(caSubject != nullptr, "Test CA subject is unavailable.");
        checkOpenSSLResult(X509_set_issuer_name(x509, caSubject),
                           "X509_set_issuer_name(device)");

        X509_NAME *subject = X509_get_subject_name(x509);
        require(subject != nullptr, "Device certificate subject is unavailable.");

        setCommonName(subject, config::DEVICE_COMMON_NAME);
        checkOpenSSLResult(X509_set_pubkey(x509, subjectKey.get()),
                           "X509_set_pubkey(device)");

        if (X509_sign(x509, privateKey_.get(), EVP_sha256()) <= 0)
            throwOpenSSL("X509_sign(device)");

        return encodeDER(x509);
    }

  private:
    void ensureInitialized() const
    {
        require(privateKey_ && certificate_, "Test CA is not initialized.");
    }

    void createCertificate()
    {
        certificate_.reset(X509_new());

        if (!certificate_)
            throwOpenSSL("X509_new(CA)");

        X509 *certificate = certificate_.get();
        checkOpenSSLResult(X509_set_version(certificate, 2), "X509_set_version(CA)"); // X.509 v3 certificate
        // Use a deterministic serial number (this is a functional-test certificate)
        checkOpenSSLResult(ASN1_INTEGER_set(X509_get_serialNumber(certificate), config::CA_SERIAL), "ASN1_INTEGER_set(CA)");

        setCertificateValidity(certificate, config::CA_VALIDITY_DAYS);

        X509_NAME *subject = X509_get_subject_name(certificate); // Build the CA subject
        require(subject != nullptr, "CA subject is unavailable.");

        setCommonName(subject, config::CA_COMMON_NAME);
        // This is a self-signed CA, therefore issuer == subject
        checkOpenSSLResult(X509_set_issuer_name(certificate, subject), "X509_set_issuer_name(CA)");
        // The CA certificate contains the CA public key
        checkOpenSSLResult(X509_set_pubkey(certificate, privateKey_.get()), "X509_set_pubkey(CA)");

        addBasicConstraints(certificate, certificate, "critical,CA:TRUE"); // Mark this certificate as a CA
        if (X509_sign(certificate, privateKey_.get(), EVP_sha256()) <= 0) // Self-sign the CA certificate
            throwOpenSSL("X509_sign(CA)");
    }

    void validateOwnCertificate() const
    {
        const ByteVector certificate = certificateDER();

        require(!certificate.empty(),
            "Generated Test CA certificate is empty.");
        require(certificate.size() <= config::DUOX_MAX_CERTIFICATE_SIZE,
                "Generated Test CA certificate exceeds DUOX size limit.");

        logicalaccess::X509V3Certificate parsed(certificate);
        require(parsed.isValid(),
            "Generated Test CA certificate is invalid X.509.");
        require(parsed.getVersion() == 2,
                "Generated Test CA certificate is not X.509 v3.");
        require(parsed.getPublicKeyType() == EVP_PKEY_EC,
                "Generated Test CA certificate does not contain an EC key.");
        require(parsed.getPublicKeyCurveNID() == NID_X9_62_prime256v1,
                "Generated Test CA certificate is not P-256.");
    }

    EVPKey privateKey_;
    X509Handle certificate_;
};

// ==============================================
// CA verification
// ==============================================

std::shared_ptr<logicalaccess::PublicKey> makePublicKeyWrapper(const ByteVector &publicKey)
{
    assertP256PublicKey(publicKey, "CA public key");

    const EVPKey key = makeP256PublicKey(publicKey);
    EVP_PKEY *copy = EVP_PKEY_dup(key.get());

    if (!copy)
        throwOpenSSL("EVP_PKEY_dup");

    return std::make_shared<logicalaccess::PublicKey>(copy);
}

void verifyCertificateSignature(const ByteVector &certificateDER, const TestCA &ca)
{
    logicalaccess::X509V3Certificate certificate(certificateDER);
    const auto caPublicKey = makePublicKeyWrapper(ca.publicKey());
    require(certificate.verify(caPublicKey), "Certificate is not signed by Test CA.");
}

void verifyCertificatePublicKey(const ByteVector &certificateDER,
                                const ByteVector &expectedPublicKey,
                                const std::string &name)
{
    logicalaccess::X509V3Certificate certificate(certificateDER);

    require(certificate.isValid(), name + " is invalid X.509.");
    require(certificate.getVersion() == 2, name + " is not X.509 v3.");
    require(certificate.getPublicKeyType() == EVP_PKEY_EC, name + " public key is not EC.");
    require(certificate.getPublicKeyCurveNID() == NID_X9_62_prime256v1, name + " public key is not P-256.");

    EVPKey certificateKey(certificate.getEVPPublicKey());
    require(certificateKey, name + " has no public key.");

    const EVPKey expectedKey = makeP256PublicKey(expectedPublicKey);
    require(EVP_PKEY_eq(certificateKey.get(), expectedKey.get()) == 1, name + " public key does not match expected key.");
}

void validateCertificate(const ByteVector &certificateDER, const ByteVector &expectedPublicKey,
    const TestCA &ca, const std::string &name)
{
    require(!certificateDER.empty(), name + " is empty.");
    require(certificateDER.size() <= config::DUOX_MAX_CERTIFICATE_SIZE, name + " exceeds DUOX certificate size.");

    logicalaccess::X509V3Certificate certificate(certificateDER);
    require(certificate.isValid(), name + " is invalid X.509.");

    verifyCertificatePublicKey(certificateDER, expectedPublicKey, name);

    const auto caPublicKey = makePublicKeyWrapper(ca.publicKey());

    logicalaccess::duox::CertificateAccessRights caRootRights;
    caRootRights.arType = config::CA_ACCESS_RIGHTS == 0 ? 0 : static_cast<std::uint8_t>(config::CA_ACCESS_RIGHTS);
    caRootRights.accessRight = config::ACCESS_RIGHTS;

    const auto validation = logicalaccess::duox::DUOXCertificateValidator::validateChain(
        {certificateDER}, caPublicKey, logicalaccess::CurveID::NIST_P256, caRootRights);

    require(validation.certificateCount == 1U, name + " produced an unexpected DUOX certificate chain length.");

    verifyCertificateSignature(certificateDER, ca);

    output::printPass(name + " contains the expected P-256 public key and is signed by Test CA.");
}

// ==============================================
// Reader authentication identity
// ==============================================

struct ReaderIdentity
{
    // The actual EVP key is retained for local verification
    EVPKey key;

    // Exact bytes passed to DUOX Part 2
    ByteVector privateKey;

    // Public key embedded in certificate
    ByteVector publicKey;

    // Cert.A supplied to DUOX Part 2 when certificate authentication is enabled
    ByteVector certificate;
};

ReaderIdentity createReaderIdentity(const TestCA &ca)
{
    ReaderIdentity identity;

    // ------------------------------------------------------------
    // Generate ONE reader key pair.
    // ------------------------------------------------------------

    output::printInfo("Generating reader P-256 identity.");
    identity.key = generateP256Key();

    // ------------------------------------------------------------
    // Extract both halves from THE SAME EVP_PKEY.
    // ------------------------------------------------------------

    identity.privateKey = extractP256PrivateKey(identity.key.get());
    identity.publicKey  = extractP256PublicKey(identity.key.get());

    // ------------------------------------------------------------
    // Certificate contains THAT SAME public key.
    // ------------------------------------------------------------

    identity.certificate = ca.issueCertificate(identity.publicKey);

    // ------------------------------------------------------------
    // Validate the complete relationship locally.
    // ------------------------------------------------------------

    validateCertificate(identity.certificate, identity.publicKey, ca, "Reader Cert.A");
    output::printPass("Reader identity created : Priv.A <-> Pub.A <-> Cert.A.");

    return identity;
}

void validateReaderIdentity(const ReaderIdentity &reader)
{
    require(reader.key, "Reader EVP key is null.");
    require(reader.privateKey.size() == config::P256_PRIVATE_KEY_SIZE, "Reader private key must be 32 bytes.");

    assertP256PublicKey(reader.publicKey, "Reader public key");
    require(!reader.certificate.empty(), "Reader certificate is empty.");

    // Re-create the public key from the private key and make sure that it's the same public key that is inside Cert.A
    const ByteVector extractedPublicKey = extractP256PublicKey(reader.key.get());
    assertPublicKeysEqual(extractedPublicKey, reader.publicKey, "Reader key public component changed.");

    const EVPKey reconstructedPublicKey = makeP256PublicKey(reader.publicKey);
    require(reconstructedPublicKey, "Unable to reconstruct reader public key.");

    output::printPass("Reader Priv.A / Pub.A / Cert.A are internally consistent.");
}

// ==============================================
// Card ECC key
// ==============================================

ByteVector generateCardKeyPair(const std::shared_ptr<logicalaccess::DUOXCommands> &duox)
{
    requireNotNull(duox, "DUOXCommands");
    output::printInfo("Generating card ECC P-256 key pair with ManageKeyPair.");

    const ByteVector publicKey = duox->manageKeyPair(
        config::CARD_ECC_KEY_NO, logicalaccess::DUOXManageKeyPairOption::GenerateKeyPair,
        logicalaccess::CurveID::NIST_P256, config::ECC_KEY_POLICY,
        config::ECC_WRITE_ACCESS, config::KUC_LIMIT, {}, config::COMM_MODE);

    assertP256PublicKey(publicKey, "Card public key");
    output::printInfo("Card Pub.B = " + hexDump(publicKey));
    output::printPass("Card P-256 key pair generated.");

    return publicKey;
}

// ==============================================
// CA Root Key
// ==============================================

void installCARootKey(const std::shared_ptr<logicalaccess::DUOXCommands> &duox, const TestCA &ca)
{
    requireNotNull(duox, "DUOXCommands");

    const ByteVector publicKey = ca.publicKey();
    assertP256PublicKey(publicKey, "CA public key");

    output::printInfo("Installing Test CA as DUOX CA Root Key.");
    output::printInfo("CA public key = " + hexDump(publicKey));

    // One of the most important pieces of the test :
    // CA public key installed in DUOX MUST be the public key corresponding to the private key that signed Cert.A/Cert.B
    duox->manageCARootKey(config::CA_ROOT_KEY_NO, logicalaccess::CurveID::NIST_P256,
                          config::CA_ACCESS_RIGHTS, config::CA_WRITE_ACCESS,
                          config::CA_READ_ACCESS, config::CA_CRL_FILE,
                          config::CA_CRL_FILE_AID, publicKey, config::CA_ISSUER,
                          config::COMM_MODE);

    output::printPass("DUOX CA Root Key installed.");
}

// ==============================================
// Certificate file
// ==============================================

void validateCertificateFileSize(std::size_t size)
{
    require(size > 0, "Certificate file size must be greater than zero.");
    require(size <= config::DUOX_MAX_CERTIFICATE_SIZE, "Certificate file size exceeds DUOX certificate limit.");
}

void createCertificateFile(const std::shared_ptr<logicalaccess::DESFireCommands> &desfire, std::size_t size)
{
    requireNotNull(desfire, "DESFireCommands");
    validateCertificateFileSize(size);

    output::printInfo("Creating StandardData certificate file.");

    desfire->createStdDataFile(config::CERTIFICATE_FILE_NO, logicalaccess::CM_PLAIN,
                               config::DUOX_TEST_CERTIFICATE_FILE_ACCESS_RIGHTS,
                               static_cast<unsigned int>(size));

    output::printPass("Certificate file created.");
}


void writeCertificate(const std::shared_ptr<logicalaccess::DESFireCommands> &desfire, const ByteVector &certificate)
{
    requireNotNull(desfire, "DESFireCommands");
    require(!certificate.empty(), "Certificate cannot be empty.");
    validateCertificateFileSize(certificate.size());

    output::printInfo("Writing certificate to StandardData file.");
    desfire->writeData(config::CERTIFICATE_FILE_NO, 0, certificate, logicalaccess::CM_PLAIN);
    output::printPass("Certificate written to card.");
}

ByteVector readCertificateFile(const std::shared_ptr<logicalaccess::DESFireCommands> &desfire,
                    std::uint8_t fileNo, std::size_t size, logicalaccess::EncryptionMode communicationMode)
{
    requireNotNull(desfire, "DESFireCommands");

    require(size > 0, "Certificate read size must be greater than zero.");
    output::printInfo("Reading certificate from StandardData file.");

    const ByteVector certificate = desfire->readData(fileNo, 0, static_cast<unsigned int>(size), communicationMode);
    require(certificate.size() == size, "Certificate file returned an unexpected number of bytes.");
    output::printPass("Certificate read from StandardData file.");

    return certificate;
}

void storeCardCertificate(const std::shared_ptr<logicalaccess::DESFireCommands> &desfire, const ByteVector &certificate)
{
    requireNotNull(desfire, "DESFireCommands");
    require(!certificate.empty(), "Card certificate cannot be empty.");

    validateCertificateFileSize(certificate.size());

    createCertificateFile(desfire, certificate.size());
    writeCertificate(desfire, certificate);

    const ByteVector readBack = readCertificateFile(desfire, config::CERTIFICATE_FILE_NO,
        certificate.size(), logicalaccess::CM_PLAIN);
    require(readBack == certificate, "Certificate read-back differs from certificate written to card.");

    output::printPass("Card Cert.B read-back matches exactly.");
    output::printPlain("=============== CERTIFICATE FILE ===============");
    output::printPlain("File :");
    output::printPlain(" FileNo = " + byteToHex(config::CERTIFICATE_FILE_NO));
    output::printPlain(" Size   = " + std::to_string(readBack.size()));
    output::printPlain(" Data   = " + hexDump(readBack));
    output::printPlain("================================================");
}

// ==============================================
// Card certificate
// ==============================================

ByteVector createCardCertificate(const TestCA &ca, const ByteVector &cardPublicKey)
{
    output::printInfo("Creating Cert.B.");
    assertP256PublicKey(cardPublicKey, "Card public key");

    const ByteVector certificate = ca.issueCertificate(cardPublicKey);
    validateCertificate(certificate, cardPublicKey, ca, "Card Cert.B");
    output::printPass("Card Cert.B created.");

    return certificate;
}

// ============================================================================
// Test application lifecycle
// ============================================================================

class DUOXTestApplicationSession
{
  public:
    DUOXTestApplicationSession(std::shared_ptr<logicalaccess::DESFireCommands> desfire,
                               std::shared_ptr<logicalaccess::DUOXCommands> duox,
                               std::shared_ptr<logicalaccess::Chip> chip)
        : desfire_(std::move(desfire))
        , duox_(std::move(duox))
        , chip_(std::move(chip))
    {
        validateDependencies();
        prepare();
    }

    ~DUOXTestApplicationSession()
    {
        cleanupNoThrow();
    }

    DUOXTestApplicationSession(const DUOXTestApplicationSession &)            = delete;
    DUOXTestApplicationSession &operator=(const DUOXTestApplicationSession &) = delete;
    DUOXTestApplicationSession(DUOXTestApplicationSession &&)                 = delete;
    DUOXTestApplicationSession &operator=(DUOXTestApplicationSession &&)      = delete;

    const std::shared_ptr<logicalaccess::DESFireCommands> &desfire() const noexcept
    {
        return desfire_;
    }

    const std::shared_ptr<logicalaccess::DUOXCommands> &duox() const noexcept
    {
        return duox_;
    }

    const std::shared_ptr<logicalaccess::Chip> &chip() const noexcept
    {
        return chip_;
    }

  private:
    void validateDependencies() const
    {
        requireNotNull(desfire_, "DESFireCommands");
        requireNotNull(duox_, "DUOXCommands");
        requireNotNull(chip_, "Chip");
        assertTestApplicationSafety();
    }

    void prepare()
    {
        // The caller has already erased/prepared the PICC. This class owns only the dedicated application lifecycle
        selectPICC(desfire_);
        authenticatePICC(duox_);
        output::printInfo("Creating test application 0x" + formatAid(config::TEST_AID));
        duox_->createApplication(config::TEST_AID, logicalaccess::KS_DEFAULT, 2, logicalaccess::DF_KEY_AES);
        applicationCreated_ = true;
        output::printPass("Test application created.");
        selectTestApplication(desfire_, chip_);
        authenticateApplication(duox_, chip_);
    }

    void cleanupNoThrow() noexcept
    {
        if (!applicationCreated_)
            return;

        try
        {
            cleanup();
        }
        catch (const std::exception &exception)
        {
            output::printWarning(std::string("Test application cleanup failed : ") + exception.what());
        }
        catch (...)
        {
            output::printWarning("Test application cleanup failed with an unknown exception.");
        }
    }

    void cleanup()
    {
        assertTestApplicationSafety();
        output::printInfo("Cleaning up dedicated DUOX test application.");

        selectPICC(desfire_);
        authenticatePICC(duox_);

        if (!testApplicationExists(desfire_))
        {
            applicationCreated_ = false;
            output::printWarning("Dedicated test application was already absent.");
            return;
        }
        output::printInfo("Deleting ONLY dedicated DUOX test application.");

        desfire_->deleteApplication(config::TEST_AID);
        require(!testApplicationExists(desfire_), "Dedicated DUOX test application still exists after deletion.");
        applicationCreated_ = false;
        output::printPass("Dedicated DUOX test application deleted.");
    }

    std::shared_ptr<logicalaccess::DESFireCommands> desfire_;
    std::shared_ptr<logicalaccess::DUOXCommands> duox_;
    std::shared_ptr<logicalaccess::Chip> chip_;
    bool applicationCreated_ = false;
};

// ==============================================
// ISOGeneralAuthenticate
// ==============================================

void authenticateECCPart1(const std::shared_ptr<logicalaccess::DUOXCommands> &duox, ReaderCertificateMode mode)
{
    requireNotNull(duox, "DUOXCommands");

    output::printInfo("ISOGeneralAuthenticate Part 1.");
    output::printInfo(std::string("Reader certificate mode : ") + readerCertificateModeName(mode));

    // Card side : CA Root Key + Card ECC key + Card certificate file.
    // Mutual authentication is requested.

    duox->isoGeneralAuthenticatePart1(config::CA_ROOT_KEY_NO, 0, logicalaccess::CurveID::NIST_P256, true,
        certificateExpected(mode), config::CERTIFICATE_FILE_NO, config::CARD_ECC_KEY_NO);

    output::printPass("ISOGeneralAuthenticate Part 1 completed.");
}

void authenticateECCPart2(const std::shared_ptr<logicalaccess::DUOXCommands> &duox,
                          const ReaderIdentity &reader, ReaderCertificateMode mode)
{
    requireNotNull(duox, "DUOXCommands");
    output::printInfo("ISOGeneralAuthenticate Part 2.");
    validateReaderIdentity(reader);

    switch (mode)
    {
    case ReaderCertificateMode::WithCertA:
    {
        require(!reader.certificate.empty(), "WITH Cert.A selected, but reader certificate is empty.");
        output::printInfo("Part 2 mode : sending Priv.A + Cert.A.");
        output::printInfo("Cert.A size = " + std::to_string(reader.certificate.size()));
        duox->isoGeneralAuthenticatePart2(reader.privateKey, reader.certificate);
        output::printPass("ISOGeneralAuthenticate Part 2 completed WITH Cert.A.");

        return;
    }
    case ReaderCertificateMode::WithoutCertA:
    {
        output::printInfo("Part 2 mode : sending Priv.A WITHOUT Cert.A.");

        // The current DUOX API represents an absent station certificate with an empty ByteVector
        const ByteVector noCertificate;
        duox->isoGeneralAuthenticatePart2(reader.privateKey, noCertificate);
        output::printPass("ISOGeneralAuthenticate Part 2 completed WITHOUT Cert.A.");
        return;
    }
    }
    fail("Invalid ReaderCertificateMode.");
}

// ==============================================
// Full ISOGeneralAuthenticate test
// ==============================================

void runISOGeneralAuthenticateTest(
    const std::shared_ptr<logicalaccess::DESFireCommands> &desfire,
    const std::shared_ptr<logicalaccess::DUOXCommands> &duox,
    const std::shared_ptr<logicalaccess::Chip> &chip, ReaderCertificateMode mode)
{
    output::printSection(std::string("DUOX ISO GENERAL AUTHENTICATE - ") + readerCertificateModeName(mode));
    assertCurrentAid(chip, config::TEST_AID);

    // ------------------------------------------------------------
    // 1 - Create software CA
    // ------------------------------------------------------------

    output::printInfo("Creating software Test CA.");

    const TestCA ca = TestCA::create();

    output::printInfo("CA Pub = " + hexDump(ca.publicKey()));
    output::printPass("Software Test CA created.");

    // ------------------------------------------------------------
    // 2 - Generate card Priv.B / Pub.B
    // ------------------------------------------------------------

    const ByteVector cardPublicKey = generateCardKeyPair(duox);

    // ------------------------------------------------------------
    // 3 - Create and locally validate Cert.B
    // ------------------------------------------------------------

    const ByteVector cardCertificate = createCardCertificate(ca, cardPublicKey);

    // ------------------------------------------------------------
    // 4 - Install same CA public key into card
    // ------------------------------------------------------------

    installCARootKey(duox, ca);

    // ------------------------------------------------------------
    // 5 - Store and read back Cert.B
    // ------------------------------------------------------------

    storeCardCertificate(desfire, cardCertificate);

    // ------------------------------------------------------------
    // 6 - Create local reader Priv.A / Pub.A / Cert.A
    // ------------------------------------------------------------

    const ReaderIdentity reader = createReaderIdentity(ca);

    validateReaderIdentity(reader);
    output::printInfo("Cert.A size = " + std::to_string(reader.certificate.size()));
    output::printInfo("Pub.A = " + hexDump(reader.publicKey));

    // ------------------------------------------------------------
    // 7 - Start ECC authentication
    // ------------------------------------------------------------

    authenticateECCPart1(duox, mode);

    // ------------------------------------------------------------
    // 8 - Authenticate reader using local certificate
    // ------------------------------------------------------------

    authenticateECCPart2(duox, reader, mode);

    // ------------------------------------------------------------
    // 9 - Make sure authentication didn't move us to another AID
    // ------------------------------------------------------------

    assertCurrentAid(chip, config::TEST_AID);

    if (certificateExpected(mode))
    {
        output::printPass("DUOX certificate-based mutual authentication succeeded WITH Cert.A.");
    }
    else
    {
        output::printPass("DUOX authentication succeeded WITHOUT Cert.A.");
    }
}

// ==============================================
// Reader
// ==============================================

std::shared_ptr<logicalaccess::ReaderUnit> findReader(const std::shared_ptr<logicalaccess::ReaderProvider> &provider)
{
    requireNotNull(provider, "ReaderProvider");

    const auto readers = provider->getReaderList();
    for (const auto &reader : readers)
    {
        if (reader && reader->getName() == config::DESFIRE_READER_NAME)
            return reader;
    }

    std::ostringstream error;
    error << "Reader not found : " << config::DESFIRE_READER_NAME << "\nAvailable readers :";
    for (const auto &reader : readers)
    {
        if (reader)
            error << "\n  - " << reader->getName();
    }

    fail(error.str());
}

std::shared_ptr<logicalaccess::ReaderConfiguration> createReaderConfiguration()
{
    auto configuration = std::make_shared<logicalaccess::ReaderConfiguration>();
    auto *libraryManager = logicalaccess::LibraryManager::getInstance();

    require(libraryManager != nullptr, "LibraryManager unavailable.");

    const auto provider = libraryManager->getReaderProvider(config::READER_PROVIDER);
    requireNotNull(provider, "ReaderProvider");

    configuration->setReaderProvider(provider);
    configuration->setReaderUnit(findReader(provider));

    return configuration;
}

// ============================================================================
// Reader connection RAII
// ============================================================================

class ReaderConnection
{
  public:
    static ReaderConnection
    connect(const std::shared_ptr<logicalaccess::ReaderConfiguration> &configuration)
    {
        requireNotNull(configuration, "ReaderConfiguration");

        const auto reader = configuration->getReaderUnit();
        requireNotNull(reader, "ReaderUnit");

        output::printInfo("Reader : " + reader->getName());
        std::cout << "[INFO] Waiting " << config::CARD_INSERTION_TIMEOUT_MS / 1000 << " seconds for card insertion...\n";

        // The timestamp is not particularly useful by itself but it provides a consistent
        // reference point when comparing different test executions
        const auto startTime = std::time(nullptr);
        if (startTime != static_cast<std::time_t>(-1))
            std::cout << "[INFO] Time start : " << std::ctime(&startTime);

        reader->connectToReader();

        ReaderConnection connection(reader);
        if (!reader->waitInsertion(config::CARD_INSERTION_TIMEOUT_MS))
            fail("Card insertion timeout.");

        if (!reader->connect())
            fail("Unable to connect to card.");

        connection.cardConnected_ = true;

        const auto chip = reader->getSingleChip();
        if (!chip)
            fail("Unable to identify card.");

        connection.chip_ = chip;

        return connection;
    }

    ReaderConnection() = default;

    explicit ReaderConnection(std::shared_ptr<logicalaccess::ReaderUnit> reader)
        : reader_(std::move(reader))
    {
    }

    ~ReaderConnection()
    {
        disconnectNoThrow();
    }

    ReaderConnection(const ReaderConnection &)            = delete;
    ReaderConnection &operator=(const ReaderConnection &) = delete;

    ReaderConnection(ReaderConnection &&other) noexcept
        : reader_(std::move(other.reader_))
        , chip_(std::move(other.chip_))
        , cardConnected_(other.cardConnected_)
    {
        other.cardConnected_ = false;
    }

    ReaderConnection &operator=(ReaderConnection &&other) noexcept
    {
        if (this == &other)
            return *this;

        disconnectNoThrow();

        reader_        = std::move(other.reader_);
        chip_          = std::move(other.chip_);
        cardConnected_ = other.cardConnected_;

        other.cardConnected_ = false;

        return *this;
    }

    const std::shared_ptr<logicalaccess::ReaderUnit> &reader() const noexcept
    {
        return reader_;
    }

    const std::shared_ptr<logicalaccess::Chip> &chip() const noexcept
    {
        return chip_;
    }

    void disconnect()
    {
        if (!reader_)
            return;

        reader_->disconnect();
        cardConnected_ = false;
    }

  private:
    void disconnectNoThrow() noexcept
    {
        if (!reader_)
            return;

        try
        {
            reader_->disconnect();
        }
        catch (const std::exception &exception)
        {
            output::printWarning(std::string("Reader disconnect failed : ") + exception.what());
        }
        catch (...)
        {
            output::printWarning("Reader disconnect failed with an unknown exception.");
        }

        cardConnected_ = false;
    }

    std::shared_ptr<logicalaccess::ReaderUnit> reader_;
    std::shared_ptr<logicalaccess::Chip> chip_;
    bool cardConnected_ = false;
};

// ==============================================
// Test execution
// ==============================================

void runTest(const std::shared_ptr<logicalaccess::Chip> &chip)
{
    requireNotNull(chip, "Chip");
    if (chip->getCardType() != "DUOX")
    {
        output::printInfo("Inserted card is not DUOX. Nothing will be modified.");
        return;
    }

    output::printPass("DUOX card detected.");

    const auto desfire = getDESFireCommands(chip);
    const auto duox = getDUOXCommands(chip);

    // Establish deterministic PICC state
    prepareCard(desfire, duox);

    // The session owns the dedicated application and guarantees cleanup
    DUOXTestApplicationSession session(desfire, duox, chip);

    // current AID is now TEST_AID and application master key authenticated
    assertCurrentAid(chip, config::TEST_AID);

    runISOGeneralAuthenticateTest(session.desfire(), session.duox(), session.chip(), TEST_READER_CERTIFICATE_MODE);
}

} // namespace

// ==============================================
// Main
// ==============================================

int main()
{
    try
    {
        validateTestConfiguration();
        output::printSection("LLA DUOX FUNCTIONAL TEST SUITE");
        output::printPlain("Certificate / X.509 + ISOGeneralAuthenticate");
        output::printInfo("OpenSSL : " + std::string(OpenSSL_version(OPENSSL_VERSION)));
        output::printInfo("Test AID = 0x" + formatAid(config::TEST_AID));
        output::printInfo(std::string("Reader certificate mode = ") + readerCertificateModeName(TEST_READER_CERTIFICATE_MODE));

        output::printSection("CARD CONNECTION");

        const auto configuration = createReaderConfiguration();
        ReaderConnection connection = ReaderConnection::connect(configuration);
        const auto reader = connection.reader();
        const auto chip = connection.chip();

        requireNotNull(reader, "ReaderUnit after connection");
        requireNotNull(chip, "Chip after connection");

        const auto uid = reader->getNumber(chip);

        output::printInfo("Card type : " + chip->getCardType());
        output::printInfo("Card UID : " + logicalaccess::BufferHelper::getHex(uid));

        runTest(chip);

        // Explicit disconnect for the successful path even if ReaderConnection provides exception-safe fallback cleanup
        reader->disconnect();

        output::printPass("DUOX functional test suite completed.");

        return EXIT_SUCCESS;
    }
    catch (const std::exception &exception)
    {
        output::printError(exception.what());

        return EXIT_FAILURE;
    }
    catch (...)
    {
        output::printError("Unknown exception.");

        return EXIT_FAILURE;
    }
}
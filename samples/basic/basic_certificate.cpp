#include <logicalaccess/plugins/cards/duox/duoxCertificateValidator.hpp>
#include <logicalaccess/plugins/crypto/public_key.hpp>
#include <logicalaccess/plugins/crypto/x509V3Certificate.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

using namespace logicalaccess;

namespace
{

// ==============================================
// Test infrastructure
// ==============================================

class TestFailure : public std::runtime_error
{
  public:
    explicit TestFailure(const std::string &message) : std::runtime_error(message)
    {
    }
};

class TestSkipped : public std::runtime_error
{
  public:
    explicit TestSkipped(const std::string &message) : std::runtime_error(message)
    {
    }
};

void expect(bool condition, const std::string &message)
{
    if (!condition)
        throw TestFailure(message);
}

template <typename Exception = std::exception, typename Callable>
void expectThrows(Callable &&callable, const std::string &description)
{
    bool thrown = false;
    try
    {
        callable();
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    if (!thrown)
        throw TestFailure("Expected exception was not thrown : " + description);
}

std::string toHex(const ByteVector &data)
{
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');

    for (const std::uint8_t byte : data)
        stream << std::setw(2) << static_cast<unsigned int>(byte);

    return stream.str();
}

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

[[noreturn]] void opensslFailure(const std::string &operation)
{
    throw std::runtime_error(operation + " : " + opensslErrorString());
}

void require(bool condition, const std::string &message)
{
    if (!condition)
        opensslFailure(message);
}

void printSection(const std::string &name)
{
    std::cout << '\n';
    std::cout << "==============================================\n";
    std::cout << name << '\n';
    std::cout << "==============================================\n";
}

void printInfo(const std::string &name, const std::string &value)
{
    std::cout << " " << std::left << std::setw(32) << name << " : " << value << '\n';
}

void printInfo(const std::string &name, std::size_t value)
{
    printInfo(name, std::to_string(value));
}

void printInfo(const std::string &name, int value)
{
    printInfo(name, std::to_string(value));
}

void printInfo(const std::string &name, bool value)
{
    // Do not write printInfo(name, value ? "true" : "false")
    // Because const char* can select the bool overload again and cause infinite recursion
    printInfo(name, std::string(value ? "true" : "false"));
}

// ==============================================
// OpenSSL RAII
// ==============================================

// Keep the ownership wrappers local to the test suite
// The production classes own their OpenSSL objects themselves

struct EVPKeyDeleter
{
    void operator()(EVP_PKEY *key) const noexcept
    {
        if (key != nullptr)
            EVP_PKEY_free(key);
    }
};

struct EVPKeyContextDeleter
{
    void operator()(EVP_PKEY_CTX *context) const noexcept
    {
        if (context != nullptr)
            EVP_PKEY_CTX_free(context);
    }
};

struct X509Deleter
{
    void operator()(X509 *certificate) const noexcept
    {
        if (certificate != nullptr)
            X509_free(certificate);
    }
};

struct X509ExtensionDeleter
{
    void operator()(X509_EXTENSION *extension) const noexcept
    {
        if (extension != nullptr)
            X509_EXTENSION_free(extension);
    }
};

struct X509NameDeleter
{
    void operator()(X509_NAME *name) const noexcept
    {
        if (name != nullptr)
            X509_NAME_free(name);
    }
};

using EVPKeyPtr = std::unique_ptr<EVP_PKEY, EVPKeyDeleter>;
using EVPKeyContextPtr = std::unique_ptr<EVP_PKEY_CTX, EVPKeyContextDeleter>;
using X509Ptr = std::unique_ptr<X509, X509Deleter>;
using X509ExtensionPtr = std::unique_ptr<X509_EXTENSION, X509ExtensionDeleter>;
using X509NamePtr = std::unique_ptr<X509_NAME, X509NameDeleter>;

// ==============================================
// Certificate diagnostics
// ==============================================

void printCertificate(const ByteVector &der)
{
    if (der.empty())
        throw std::invalid_argument("printCertificate : certificate is empty");

    const unsigned char *cursor = der.data();
    X509Ptr certificate(d2i_X509(nullptr, &cursor, static_cast<long>(der.size())));

    if (!certificate)
        throw std::runtime_error("printCertificate : unable to parse DER certificate : " + opensslErrorString());
    if (cursor != der.data() + der.size())
        throw std::runtime_error("printCertificate : trailing data after certificate");

    std::cout << "\n---------- X.509 CERTIFICATE ----------\n";

    if (X509_print_fp(stdout, certificate.get()) != 1)
        throw std::runtime_error("printCertificate : X509_print_fp failed : " + opensslErrorString());

    std::cout << "---------------------------------------\n";
}

void saveDERCertificate(const ByteVector &der, const std::string &path)
{
    if (der.empty())
        throw std::invalid_argument("saveDERCertificate : certificate is empty");

    std::ofstream file(path, std::ios::binary | std::ios::trunc);

    if (!file)
        throw std::runtime_error("saveDERCertificate : unable to open : " + path);

    file.write(reinterpret_cast<const char *>(der.data()), static_cast<std::streamsize>(der.size()));

    if (!file)
        throw std::runtime_error("saveDERCertificate : failed writing : " + path);
}

// ==============================================
// OpenSSL helpers
// ==============================================

int getEVPPublicKeyCurveNID(EVP_PKEY *key)
{
    if (key == nullptr)
        throw std::invalid_argument("getEVPPublicKeyCurveNID : null key");

    char groupName[128]         = {};
    std::size_t groupNameLength = 0;

    if (EVP_PKEY_get_utf8_string_param(key, OSSL_PKEY_PARAM_GROUP_NAME, groupName,
                                       sizeof(groupName), &groupNameLength) != 1)
    {
        opensslFailure("EVP_PKEY_get_utf8_string_param(group-name)");
    }

    groupName[groupNameLength] = '\0';

    const int nid = OBJ_txt2nid(groupName);
    if (nid == NID_undef)
        throw std::runtime_error(std::string("Unknown EC group name : ") + groupName);

    return nid;
}

ByteVector getUncompressedPublicPoint(EVP_PKEY *key)
{
    require(key != nullptr, "null EC key");

    std::size_t required = 0;

    if (EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, nullptr, 0, &required) != 1)
        opensslFailure("EVP_PKEY_get_octet_string_param(size)");

    require(required != 0, "EC public point is empty");

    ByteVector result(required);
    std::size_t written = 0;

    if (EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, result.data(), result.size(), &written) != 1)
        opensslFailure("EVP_PKEY_get_octet_string_param");

    result.resize(written);

    return result;
}

// ==============================================
// Certificate generation
// ==============================================

struct GeneratedCertificate
{
    ByteVector der;
    std::shared_ptr<PublicKey> publicKey;
};

EVPKeyPtr generateECKey(int curveNID)
{
    EVPKeyContextPtr context(EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr));

    if (!context)
        opensslFailure("EVP_PKEY_CTX_new_id(EVP_PKEY_EC)");
    if (EVP_PKEY_keygen_init(context.get()) != 1)
        opensslFailure("EVP_PKEY_keygen_init");
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(context.get(), curveNID) != 1)
        opensslFailure("EVP_PKEY_CTX_set_ec_paramgen_curve_nid");

    EVP_PKEY *generated = nullptr;

    if (EVP_PKEY_keygen(context.get(), &generated) != 1)
        opensslFailure("EVP_PKEY_keygen");
    if (generated == nullptr)
        opensslFailure("EVP_PKEY_keygen returned null key");

    return EVPKeyPtr(generated);
}

bool canGenerateCurve(int curveNID)
{
    try
    {
        EVPKeyPtr key = generateECKey(curveNID);
        return static_cast<bool>(key);
    }
    catch (...)
    {
        ERR_clear_error();
        return false;
    }
}

X509_NAME *createName(X509 *certificate, const char *commonName)
{
    X509_NAME *name = X509_get_subject_name(certificate);

    if (name == nullptr)
        opensslFailure("X509_get_subject_name");

    if (X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char *>(commonName),
                                   -1, -1, 0) != 1)
    {
        opensslFailure("X509_NAME_add_entry_by_txt");
    }

    return name;
}

ByteVector serializeCertificate(X509 *certificate)
{
    const int length = i2d_X509(certificate, nullptr);
    if (length <= 0)
        opensslFailure("i2d_X509(size)");

    ByteVector result(static_cast<std::size_t>(length));
    unsigned char *cursor = result.data();

    const int encodedLength = i2d_X509(certificate, &cursor);
    if (encodedLength != length)
        throw std::runtime_error("i2d_X509 returned inconsistent length");

    return result;
}

GeneratedCertificate createCertificate(int curveNID, EVP_PKEY *subjectKey,
                                       EVP_PKEY *issuerKey, const char *subjectName,
                                       const char *issuerName, long serialNumber,
                                       bool caCertificate)
{
    (void)curveNID;

    if (subjectKey == nullptr)
        throw std::invalid_argument("createCertificate : subject key is null");
    if (issuerKey == nullptr)
        throw std::invalid_argument("createCertificate : issuer key is null");

    X509Ptr certificate(X509_new());

    if (!certificate)
        opensslFailure("X509_new");
    // X.509 v3 => internal version value 2
    if (X509_set_version(certificate.get(), 2) != 1)
        opensslFailure("X509_set_version");

    ASN1_INTEGER *serial = X509_get_serialNumber(certificate.get());

    if (serial == nullptr)
        opensslFailure("X509_get_serialNumber");
    if (ASN1_INTEGER_set(serial, serialNumber) != 1)
        opensslFailure("ASN1_INTEGER_set");
    if (X509_gmtime_adj(X509_getm_notBefore(certificate.get()), 0) == nullptr)
        opensslFailure("X509_gmtime_adj(notBefore)");
    if (X509_gmtime_adj(X509_getm_notAfter(certificate.get()), 60L * 60L * 24L * 365L) == nullptr)
        opensslFailure("X509_gmtime_adj(notAfter)");

    X509_NAME *subject = createName(certificate.get(), subjectName);
    if (X509_set_subject_name(certificate.get(), subject) != 1)
        opensslFailure("X509_set_subject_name");

    X509NamePtr issuer(X509_NAME_new());
    if (!issuer)
        opensslFailure("X509_NAME_new");
    if (X509_NAME_add_entry_by_txt(issuer.get(), "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char *>(issuerName),
                                   -1, -1, 0) != 1)
    {
        opensslFailure("X509_NAME_add_entry_by_txt(issuer)");
    }
    if (X509_set_issuer_name(certificate.get(), issuer.get()) != 1)
        opensslFailure("X509_set_issuer_name");
    if (X509_set_pubkey(certificate.get(), subjectKey) != 1)
        opensslFailure("X509_set_pubkey");

    // X.509v3 extensions
    X509V3_CTX extensionContext;
    X509V3_set_ctx_nodb(&extensionContext);
    X509V3_set_ctx(&extensionContext, certificate.get(), certificate.get(), nullptr, nullptr, 0);

    const char *basicConstraintsValue = caCertificate ? "critical,CA:TRUE" : "critical,CA:FALSE";

    X509ExtensionPtr basicConstraints(X509V3_EXT_conf_nid(nullptr, &extensionContext, NID_basic_constraints,
                            const_cast<char *>(basicConstraintsValue)));

    if (!basicConstraints)
        opensslFailure("X509V3_EXT_conf_nid(basicConstraints)");
    if (X509_add_ext(certificate.get(), basicConstraints.get(), -1) != 1)
        opensslFailure("X509_add_ext(basicConstraints)");

    const char *keyUsageValue = caCertificate ? "critical,keyCertSign,cRLSign" : "critical,digitalSignature,keyAgreement";

    X509ExtensionPtr keyUsage(X509V3_EXT_conf_nid(nullptr, &extensionContext,
        NID_key_usage, const_cast<char *>(keyUsageValue)));

    if (!keyUsage)
        opensslFailure("X509V3_EXT_conf_nid(keyUsage)");
    if (X509_add_ext(certificate.get(), keyUsage.get(), -1) != 1)
        opensslFailure("X509_add_ext(keyUsage)");
    // Sign using ECDSA/SHA-256
    if (X509_sign(certificate.get(), issuerKey, EVP_sha256()) <= 0)
        opensslFailure("X509_sign");

    GeneratedCertificate result;
    result.der = serializeCertificate(certificate.get());

    // Returns a fresh EVP_PKEY reference; ownership is transferred to PublicKey
    EVP_PKEY *publicKeyReference = X509_get_pubkey(certificate.get());
    if (publicKeyReference == nullptr)
        opensslFailure("X509_get_pubkey");
    result.publicKey = std::make_shared<PublicKey>(publicKeyReference);

    return result;
}

struct CertificateFixture
{
    EVPKeyPtr key;
    GeneratedCertificate certificate;
};

CertificateFixture createSelfSignedCertificate(int curveNID)
{
    CertificateFixture result;
    result.key = generateECKey(curveNID);
    result.certificate = createCertificate(curveNID, result.key.get(), result.key.get(),
                          "LLA Self Signed Test", "LLA Self Signed Test", 1, false);

    return result;
}

struct ChainFixture
{
    EVPKeyPtr rootKey;
    EVPKeyPtr leafKey;

    GeneratedCertificate root;
    GeneratedCertificate leaf;
};

ChainFixture createRootAndLeafChain(int curveNID)
{
    ChainFixture result;
    result.rootKey = generateECKey(curveNID);
    result.leafKey = generateECKey(curveNID);
    result.root = createCertificate(curveNID, result.rootKey.get(), result.rootKey.get(),
                                    "LLA DUOX Test Root", "LLA DUOX Test Root", 1, true);
    result.leaf = createCertificate(curveNID, result.leafKey.get(), result.rootKey.get(),
                          "LLA DUOX Test Reader", "LLA DUOX Test Root", 2, false);
    return result;
}

// ==============================================
// X509V3Certificate tests
// ==============================================

void testEmptyCertificate()
{
    printSection("X509V3Certificate : empty input");
    expectThrows<std::invalid_argument>(
        [] { X509V3Certificate certificate(ByteVector{}); },
        "constructing certificate from empty ByteVector");

    X509V3Certificate certificate;

    expect(!certificate.isValid(), "default certificate must be invalid");

    std::cout << " Empty input correctly rejected.\n";
    std::cout << " Default object correctly remains invalid.\n";
}

void testEmptyStringCertificate()
{
    printSection("X509V3Certificate : empty string");

    const std::string empty;

    expectThrows<std::invalid_argument>(
        [&] { X509V3Certificate certificate(empty); },
        "constructing certificate from empty std::string");

    std::cout << " Empty binary string correctly rejected.\n";
}

void testInvalidCertificate()
{
    printSection("X509V3Certificate : malformed DER");

    const ByteVector invalid = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    expectThrows([&] { X509V3Certificate certificate(invalid); }, "constructing certificate from malformed DER");

    std::cout << " Malformed DER correctly rejected.\n";
}

void testTrailingData()
{
    printSection("X509V3Certificate : DER trailing data rejection");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    ByteVector malformed = fixture.certificate.der;
    malformed.push_back(0x00);

    expectThrows<std::invalid_argument>([&] { X509V3Certificate certificate(malformed); },
                                        "valid certificate followed by trailing byte");

    std::cout << " Trailing data correctly rejected.\n";
}

void testValidCertificate()
{
    printSection("X509V3Certificate : complete valid certificate");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate certificate(fixture.certificate.der);
    expect(certificate.isValid(), "certificate must be valid");
    expect(certificate.getDERData() == fixture.certificate.der, "getDERData() must preserve exact DER input");
    expect(certificate.getX509() != nullptr, "getX509() must return underlying X509");
    expect(certificate.getVersion() == 2, "certificate must be X.509 v3");
    expect(certificate.getPublicKeyType() == EVP_PKEY_EC, "certificate public key must be EC");
    expect(certificate.getPublicKeyCurveNID() == NID_X9_62_prime256v1, "certificate must use NIST P-256");
    expect(certificate.getSignatureAlgorithmNID() == NID_ecdsa_with_SHA256, "certificate must use ECDSA with SHA-256");

    const auto publicKey = certificate.getKey();
    expect(publicKey != nullptr, "getKey() must return a public key");
    expect(publicKey->getPublicKey() != nullptr, "returned PublicKey must contain an EVP_PKEY");

    const ByteVector serial = certificate.getSerialNumber();
    expect(!serial.empty(), "serial number must not be empty");
    expect(serial.size() <= 4, "test certificate serial must fit DUOX four-byte limit");

    const ByteVector signature = certificate.getSignatureValue();
    expect(!signature.empty(), "certificate signature must not be empty");

    const ByteVector tbs = certificate.getTBSCertificateDER();
    expect(!tbs.empty(), "TBSCertificate DER must not be empty");

    const ByteVector spki = certificate.getSubjectPublicKeyInfoDER();
    expect(!spki.empty(), "SubjectPublicKeyInfo DER must not be empty");

    printInfo("DER size", certificate.getDERData().size());
    printInfo("X.509 version", static_cast<int>(certificate.getVersion()));
    printInfo("Public key type NID", certificate.getPublicKeyType());
    printInfo("Curve NID", certificate.getPublicKeyCurveNID());
    printInfo("Signature algorithm NID", certificate.getSignatureAlgorithmNID());
    printInfo("Serial size", serial.size());
    printInfo("TBS size", tbs.size());
    printInfo("SPKI size", spki.size());
    printInfo("Signature size", signature.size());
    printInfo("Serial", toHex(serial));

    std::cout << " Certificate representation validated successfully.\n";
}

void testMultipleCertificateInstances()
{
    printSection("X509V3Certificate : independent certificate instances");

    const CertificateFixture firstFixture = createSelfSignedCertificate(NID_X9_62_prime256v1);
    const CertificateFixture secondFixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate first(firstFixture.certificate.der);
    X509V3Certificate second(secondFixture.certificate.der);

    expect(first.isValid(), "first certificate must be valid");
    expect(second.isValid(), "second certificate must be valid");
    expect(first.getDERData() == firstFixture.certificate.der, "first certificate must retain its own DER");
    expect(second.getDERData() == secondFixture.certificate.der, "second certificate must retain its own DER");
    expect(first.getX509() != nullptr, "first certificate must own an X509 object");
    expect(second.getX509() != nullptr, "second certificate must own an X509 object");
    expect(first.getX509() != second.getX509(), "certificate instances must own distinct X509 objects");
    expect(first.getKey() != nullptr, "first certificate must have a public key");
    expect(second.getKey() != nullptr, "second certificate must have a public key");
    expect(first.getKey()->getPublicKey() != second.getKey()->getPublicKey(),
           "certificate instances must own distinct EVP_PKEY objects");

    // Replacing one instance must not modify the other
    first.setDERData(secondFixture.certificate.der);
    expect(first.isValid(), "first certificate must remain valid after replacement");
    expect(second.isValid(), "second certificate must remain valid after first replacement");
    expect(second.getDERData() == secondFixture.certificate.der, "second certificate must remain unchanged");

    // Invalidate replacement of first. Previous valid state must remain intact while second remains completely independent
    expectThrows([&] { first.setDERData(ByteVector{0x01, 0x02, 0x03}); }, "invalid replacement of first certificate");
    expect(first.isValid(), "failed replacement must preserve first certificate");
    expect(second.isValid(), "failed replacement of first must not affect second");
    expect(second.getDERData() == secondFixture.certificate.der, "second certificate DER must remain unchanged");

    std::cout << " Multiple certificate instances remain fully independent.\n";
}

// ==============================================
// Move semantics
// ==============================================

void testMoveConstruction()
{
    printSection("X509V3Certificate : move construction");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate original(fixture.certificate.der);

    X509 *originalX509 = original.getX509();
    expect(originalX509 != nullptr, "source certificate must initially own X509");

    X509V3Certificate moved(std::move(original));
    expect(moved.isValid(), "move destination must remain valid");
    expect(moved.getDERData() == fixture.certificate.der, "move destination must preserve DER");
    expect(moved.getX509() == originalX509, "move must transfer X509 ownership");
    expect(!original.isValid(), "moved-from object must be invalid");
    expect(original.getX509() == nullptr, "moved-from object must not retain X509");

    std::cout << " Move construction preserves ownership correctly.\n";
}

void testMoveAssignment()
{
    printSection("X509V3Certificate : move assignment");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate source(fixture.certificate.der);
    X509V3Certificate destination;
    destination = std::move(source);
    expect(destination.isValid(), "move-assignment destination must be valid");
    expect(destination.getDERData() == fixture.certificate.der, "move-assignment destination must preserve DER");
    expect(!source.isValid(), "move-assignment source must be invalid");
    expect(source.getX509() == nullptr, "move-assignment source must not retain X509");

    std::cout << " Move assignment preserves ownership correctly.\n";
}

// ==============================================
// DER input / replacement
// ==============================================

void testSetDERData()
{
    printSection("X509V3Certificate : setDERData");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate certificate;
    expect(!certificate.isValid(), "default certificate must initially be invalid");

    certificate.setDERData(fixture.certificate.der);
    expect(certificate.isValid(), "setDERData must initialize certificate");
    expect(certificate.getDERData() == fixture.certificate.der, "setDERData must preserve DER");

    std::cout << " setDERData successfully initialized certificate.\n";
}

void testSetDERDataString()
{
    printSection("X509V3Certificate : binary std::string input");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    const std::string binaryDER(reinterpret_cast<const char *>(fixture.certificate.der.data()),
        fixture.certificate.der.size());

    X509V3Certificate certificate(binaryDER);
    expect(certificate.isValid(), "binary std::string certificate must parse");
    expect(certificate.getDERData() == fixture.certificate.der, "binary string input must preserve exact bytes");

    std::cout << " Binary std::string DER handling succeeded.\n";
}

void testTransactionalReplacement()
{
    printSection("X509V3Certificate : transactional replacement");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate certificate(fixture.certificate.der);

    const ByteVector originalDER = certificate.getDERData();
    const ByteVector malformed = {0x01, 0x02, 0x03};

    expectThrows([&] { certificate.setDERData(malformed); }, "replacing valid certificate with malformed DER");
    expect(certificate.isValid(), "failed replacement must preserve old certificate");
    expect(certificate.getDERData() == originalDER, "failed replacement must preserve original DER");
    expect(certificate.getVersion() == 2, "old certificate must remain fully usable after failure");

    std::cout << " Failed replacement correctly preserved prior state.\n";
}

void testLargeMalformedInput()
{
    printSection("X509V3Certificate : malformed large input");

    const ByteVector malformed(4096, 0xAA);

    expectThrows([&] { X509V3Certificate certificate(malformed); }, "4096-byte malformed certificate");

    std::cout << " Large malformed DER correctly rejected.\n";
}

// ==============================================
// PublicKey ownership tests
// ==============================================

void testPublicKeyOwnership()
{
    printSection("PublicKey : ownership and replacement");

    EVPKeyPtr firstKey = generateECKey(NID_X9_62_prime256v1);
    EVPKeyPtr secondKey = generateECKey(NID_X9_62_prime256v1);

    EVP_PKEY *firstRaw  = firstKey.release();
    EVP_PKEY *secondRaw = secondKey.release();

    PublicKey publicKey(firstRaw);
    expect(publicKey.getPublicKey() == firstRaw, "PublicKey must take ownership of supplied EVP_PKEY");

    publicKey.setPublicKey(secondRaw);
    expect(publicKey.getPublicKey() == secondRaw, "setPublicKey must replace owned EVP_PKEY");

    publicKey.setPublicKey(nullptr);
    expect(publicKey.getPublicKey() == nullptr, "setPublicKey(nullptr) must clear the key");

    std::cout << " PublicKey ownership and replacement validated.\n";
}

void testPublicKeyInstancesAreIndependent()
{
    printSection("PublicKey : independent instances");

    EVPKeyPtr firstRaw = generateECKey(NID_X9_62_prime256v1);
    EVPKeyPtr secondRaw = generateECKey(NID_X9_62_prime256v1);

    PublicKey first(firstRaw.release());
    PublicKey second(secondRaw.release());

    expect(first.getPublicKey() != nullptr, "first PublicKey must contain EVP_PKEY");
    expect(second.getPublicKey() != nullptr, "second PublicKey must contain EVP_PKEY");
    expect(first.getPublicKey() != second.getPublicKey(),
           "different PublicKey instances must own different EVP_PKEY objects");

    EVP_PKEY *secondKey = second.getPublicKey();
    first.setPublicKey(nullptr);

    expect(first.getPublicKey() == nullptr, "clearing first PublicKey must clear only first instance");
    expect(second.getPublicKey() == secondKey, "clearing first PublicKey must not affect second instance");

    std::cout << " PublicKey instances remain independent.\n";
}

// ==============================================
// X509V3Certificate : cryptographic verification
// ==============================================

void testCertificateVerification()
{
    printSection("X509V3Certificate : certificate signature verification");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate certificate(fixture.certificate.der);

    const auto certificateKey = certificate.getKey();
    expect(certificate.verify(certificateKey), "self-signed certificate must verify with its own public key");

    std::cout << " Self-signature verification succeeded.\n";
}

void testCertificateVerificationWithWrongKey()
{
    printSection("X509V3Certificate : wrong issuer key rejection");

    const CertificateFixture certificateFixture = createSelfSignedCertificate(NID_X9_62_prime256v1);
    const CertificateFixture wrongKeyFixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate certificate(certificateFixture.certificate.der);

    const auto wrongKey = wrongKeyFixture.certificate.publicKey;
    expect(!certificate.verify(wrongKey), "certificate must not verify with unrelated public key");

    std::cout << " Unrelated issuer key correctly rejected.\n";
}

void testCertificateVerificationNullKey()
{
    printSection("X509V3Certificate : null issuer key handling");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate certificate(fixture.certificate.der);
    expectThrows<std::invalid_argument>([&] { certificate.verify(nullptr); }, "certificate.verify(nullptr)");

    std::cout << " Null issuer key correctly rejected.\n";
}

// ==============================================
// Exact DER / ASN.1 extraction
// ==============================================

void testDERComponentExactEncoding()
{
    printSection("X509V3Certificate : exact DER component encoding");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate certificate(fixture.certificate.der);
    X509 *x509 = certificate.getX509();
    expect(x509 != nullptr, "underlying X509 must not be null");

    // TBSCertificate
    const int expectedTbsLength = i2d_re_X509_tbs(x509, nullptr);
    expect(expectedTbsLength > 0, "i2d_re_X509_tbs must return a positive length");

    ByteVector expectedTbs(static_cast<std::size_t>(expectedTbsLength));
    unsigned char *tbsCursor = expectedTbs.data();

    const int encodedTbsLength = i2d_re_X509_tbs(x509, &tbsCursor);
    expect(encodedTbsLength == expectedTbsLength, "i2d_re_X509_tbs returned inconsistent length");
    expect(certificate.getTBSCertificateDER() == expectedTbs,
           "getTBSCertificateDER must exactly match OpenSSL DER encoding");

    // SubjectPublicKeyInfo
    X509_PUBKEY *publicKey = X509_get_X509_PUBKEY(x509);
    expect(publicKey != nullptr, "certificate must contain SubjectPublicKeyInfo");

    const int expectedSpkiLength = i2d_X509_PUBKEY(publicKey, nullptr);
    expect(expectedSpkiLength > 0, "i2d_X509_PUBKEY must return a positive length");

    ByteVector expectedSpki(static_cast<std::size_t>(expectedSpkiLength));
    unsigned char *spkiCursor = expectedSpki.data();

    const int encodedSpkiLength = i2d_X509_PUBKEY(publicKey, &spkiCursor);
    expect(encodedSpkiLength == expectedSpkiLength, "i2d_X509_PUBKEY returned inconsistent length");
    expect(certificate.getSubjectPublicKeyInfoDER() == expectedSpki,
           "getSubjectPublicKeyInfoDER must exactly match OpenSSL DER encoding");

    // SignatureValue
    const ASN1_BIT_STRING *signature     = nullptr;
    const X509_ALGOR *signatureAlgorithm = nullptr;

    X509_get0_signature(&signature, &signatureAlgorithm, x509);
    expect(signature != nullptr, "certificate signature BIT STRING must not be null");
    expect(signature->data != nullptr, "certificate signature data must not be null");
    expect(signature->length > 0, "certificate signature must not be empty");

    const ByteVector expectedSignature(signature->data, signature->data + signature->length);
    expect(certificate.getSignatureValue() == expectedSignature,
           "getSignatureValue must exactly match X509 signature BIT STRING");

    std::cout << " TBS, SPKI and signature DER extraction validated exactly.\n";
}

void testSerialNumberEncoding()
{
    printSection("X509V3Certificate : serial number encoding");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate certificate(fixture.certificate.der);
    X509 *x509 = certificate.getX509();
    expect(x509 != nullptr, "underlying X509 must not be null");

    const ASN1_INTEGER *serial = X509_get0_serialNumber(x509);
    expect(serial != nullptr, "certificate serial number must not be null");
    expect(serial->data != nullptr, "certificate serial number data must not be null");
    expect(serial->length > 0, "certificate serial number must not be empty");

    const ByteVector expected(serial->data, serial->data + serial->length);
    expect(certificate.getSerialNumber() == expected, "getSerialNumber must return the ASN.1 INTEGER value bytes");

    std::cout << " Serial number extraction matches OpenSSL exactly.\n";
}

// ==============================================
// DUOX validator tests
// ==============================================

void testDUOXParse()
{
    printSection("DUOXCertificateValidator : parse");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);
    const duox::ValidatedCertificate result = duox::DUOXCertificateValidator::parse(fixture.certificate.der);

    expect(result.certificate.isValid(), "validated certificate must contain valid X509V3Certificate");
    expect(result.publicKey != nullptr, "validated certificate must expose public key");
    expect(result.publicKey->getPublicKey() != nullptr, "validated certificate public key must contain EVP_PKEY");
    expect(result.curve == CurveID::NIST_P256, "parsed certificate curve must be NIST P-256");
    expect(result.serialNumber == ByteVector{0x01}, "generated certificate serial must be 1");
    expect(!result.hasAccessRightsExtension, "test certificate must not have DUOX access-right extension");
    expect(result.accessRights.arType == 0, "default access-right arType must be zero");
    expect(result.accessRights.accessRight == 0, "default access-right value must be zero");

    printInfo("Validated DER size", result.certificate.getDERData().size());
    printInfo("Curve", result.curve == CurveID::NIST_P256 ? std::string("NIST_P256") : std::string("other"));
    printInfo("Serial", toHex(result.serialNumber));
    printInfo("Has access-right extension", result.hasAccessRightsExtension);

    std::cout << " DUOX single-certificate parsing succeeded.\n";
}

void testDUOXStructureValidation()
{
    printSection("DUOXCertificateValidator : structure validation");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);
    X509V3Certificate certificate(fixture.certificate.der);

    duox::DUOXCertificateValidator::validateStructure(certificate);
    duox::DUOXCertificateValidator::validateForCurve(certificate, CurveID::NIST_P256);

    std::cout << " DUOX structural validation succeeded.\n";
    std::cout << " P-256 curve validation succeeded.\n";
}

void testDUOXWrongCurveValidation()
{
    printSection("DUOXCertificateValidator : wrong curve rejection");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);
    X509V3Certificate certificate(fixture.certificate.der);

    expectThrows(
        [&]
        {
            duox::DUOXCertificateValidator::validateForCurve(certificate, CurveID::BRAINPOOL_P256R1);
        },
        "P-256 certificate validated as Brainpool P-256r1");

    std::cout << " Wrong curve correctly rejected.\n";
}

void testDUOXWrongIssuerSignature()
{
    printSection("DUOXCertificateValidator : wrong issuer signature");

    const CertificateFixture certificateFixture = createSelfSignedCertificate(NID_X9_62_prime256v1);
    const CertificateFixture wrongIssuer = createSelfSignedCertificate(NID_X9_62_prime256v1);

    X509V3Certificate certificate(certificateFixture.certificate.der);

    expectThrows(
        [&]
        {
            duox::DUOXCertificateValidator::validateSignature( certificate, wrongIssuer.certificate.publicKey);
        },
        "certificate validated against unrelated issuer key");

    std::cout << " Wrong issuer correctly rejected.\n";
}

void testDUOXNullIssuer()
{
    printSection("DUOXCertificateValidator : null issuer");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);
    X509V3Certificate certificate(fixture.certificate.der);

    expectThrows<std::invalid_argument>(
        [&] { duox::DUOXCertificateValidator::validateSignature(certificate, nullptr); },
        "validateSignature with null issuer");

    std::cout << " Null issuer correctly rejected.\n";
}

void testDUOXInvalidCertificate()
{
    printSection("DUOXCertificateValidator : malformed certificate");

    const ByteVector malformed = {0x30, 0x01, 0x00};

    expectThrows([&] { duox::DUOXCertificateValidator::parse(malformed); }, "parsing malformed DUOX certificate");

    std::cout << " Malformed certificate correctly rejected.\n";
}

void testDUOXEmptyCertificate()
{
    printSection("DUOXCertificateValidator : empty certificate");

    expectThrows<std::invalid_argument>(
        [] { duox::DUOXCertificateValidator::parse(ByteVector{}); },
        "parsing empty DUOX certificate");

    std::cout << " Empty certificate correctly rejected.\n";
}

// ==============================================
// Certificate-chain tests
// ==============================================

void testSingleCertificateChain()
{
    printSection("DUOXCertificateValidator : one-certificate chain");

    const ChainFixture fixture = createRootAndLeafChain(NID_X9_62_prime256v1);
    const std::vector<ByteVector> certificates = {fixture.root.der};
    const duox::CertificateAccessRights caRights;

    const auto result = duox::DUOXCertificateValidator::validateChain(
        certificates, fixture.root.publicKey, CurveID::NIST_P256, caRights);

    expect(result.certificateCount == 1, "single-certificate chain must contain one certificate");
    expect(result.leaf.certificate.isValid(), "chain leaf must be valid");
    expect(result.leaf.publicKey != nullptr, "chain leaf must expose public key");
    expect(result.leaf.curve == CurveID::NIST_P256, "chain leaf must use P-256");

    printInfo("Certificate count", result.certificateCount);
    printInfo("Leaf DER size", result.leaf.certificate.getDERData().size());

    std::cout << " One-certificate trust-anchor validation succeeded.\n";
}

void testTwoCertificateChain()
{
    printSection("DUOXCertificateValidator : two-certificate chain");

    const ChainFixture fixture = createRootAndLeafChain(NID_X9_62_prime256v1);

    /*
     * Chain ordering :
     *     certificates[0] = leaf
     *     certificates[1] = root
     */
    const std::vector<ByteVector> certificates = {fixture.leaf.der, fixture.root.der};
    const duox::CertificateAccessRights caRights;

    const auto result = duox::DUOXCertificateValidator::validateChain(
        certificates, fixture.root.publicKey, CurveID::NIST_P256, caRights);

    expect(result.certificateCount == 2, "two-certificate chain must contain two certificates");
    expect(result.leaf.certificate.isValid(), "leaf certificate must be valid");
    expect(result.leaf.curve == CurveID::NIST_P256, "leaf must use P-256");
    expect(result.leaf.serialNumber == ByteVector{0x02}, "leaf serial number must be 2");
    expect(result.leaf.publicKey != nullptr, "leaf public key must be available");

    printInfo("Certificate count", result.certificateCount);
    printInfo("Leaf serial", toHex(result.leaf.serialNumber));
    printInfo("Leaf public key type", result.leaf.certificate.getPublicKeyType());

    std::cout << " Two-certificate chain validation succeeded.\n";
}

void testChainWrongRoot()
{
    printSection("DUOXCertificateValidator : wrong CA root rejection");

    const ChainFixture fixture = createRootAndLeafChain(NID_X9_62_prime256v1);
    const CertificateFixture wrongRoot = createSelfSignedCertificate(NID_X9_62_prime256v1);
    const std::vector<ByteVector> certificates = {fixture.leaf.der, fixture.root.der};
    const duox::CertificateAccessRights caRights;

    expectThrows(
        [&]
        {
            duox::DUOXCertificateValidator::validateChain(certificates, wrongRoot.certificate.publicKey,
                                                          CurveID::NIST_P256, caRights);
        },
        "chain validated using unrelated CA root key");

    std::cout << " Wrong CA root correctly rejected.\n";
}

void testChainWrongCurve()
{
    printSection("DUOXCertificateValidator : chain curve mismatch");

    const ChainFixture fixture = createRootAndLeafChain(NID_X9_62_prime256v1);
    const std::vector<ByteVector> certificates = {fixture.leaf.der, fixture.root.der};
    const duox::CertificateAccessRights caRights;

    expectThrows(
        [&]
        {
            duox::DUOXCertificateValidator::validateChain(
                certificates, fixture.root.publicKey, CurveID::BRAINPOOL_P256R1, caRights);
        },
        "P-256 chain validated as Brainpool");

    std::cout << " Chain curve mismatch correctly rejected.\n";
}

void testChainEmpty()
{
    printSection("DUOXCertificateValidator : empty chain");

    const CertificateFixture root = createSelfSignedCertificate(NID_X9_62_prime256v1);
    const std::vector<ByteVector> certificates;
    const duox::CertificateAccessRights caRights;

    expectThrows(
        [&]
        {
            duox::DUOXCertificateValidator::validateChain(
                certificates, root.certificate.publicKey, CurveID::NIST_P256, caRights);
        },
        "empty certificate chain");

    std::cout << " Empty chain correctly rejected.\n";
}

void testChainTooLong()
{
    printSection("DUOXCertificateValidator : excessive chain length");

    const ChainFixture fixture = createRootAndLeafChain(NID_X9_62_prime256v1);
    const std::vector<ByteVector> certificates = {fixture.leaf.der, fixture.root.der, fixture.root.der};
    const duox::CertificateAccessRights caRights;

    expectThrows(
        [&]
        {
            duox::DUOXCertificateValidator::validateChain(
                certificates, fixture.root.publicKey, CurveID::NIST_P256, caRights);
        },
        "three-certificate DUOX chain");

    std::cout << " Excessive chain length correctly rejected.\n";
}

void testChainMalformedIntermediate()
{
    printSection("DUOXCertificateValidator : malformed chain member");

    const ChainFixture fixture = createRootAndLeafChain(NID_X9_62_prime256v1);
    const std::vector<ByteVector> certificates = {fixture.leaf.der, ByteVector{0x01, 0x02, 0x03}};
    const duox::CertificateAccessRights caRights;

    expectThrows(
        [&]
        {
            duox::DUOXCertificateValidator::validateChain(
                certificates, fixture.root.publicKey, CurveID::NIST_P256, caRights);
        },
        "malformed second certificate in chain");

    std::cout << " Malformed chain member correctly rejected.\n";
}

// ==============================================
// Access rights
// ==============================================

void testEffectiveAccessRightsZero()
{
    printSection("DUOXCertificateValidator : effective access rights");

    duox::CertificateAccessRights certificateRights;
    duox::CertificateAccessRights caRootRights;

    certificateRights.arType      = 0;
    certificateRights.accessRight = 0;

    caRootRights.arType      = 0;
    caRootRights.accessRight = 0;

    const auto effective = duox::DUOXCertificateValidator::calculateEffectiveAccessRights(certificateRights, caRootRights);
    expect(effective.arType == 0, "zero certificate arType must produce zero effective arType");
    expect(effective.accessRight == 0, "zero certificate accessRight must produce zero effective accessRight");

    std::cout << " Zero-right calculation is deterministic.\n";
}

// ==============================================
// EC curve abstraction tests
// ==============================================

void testP256CurveAbstraction()
{
    printSection("DUOX / OpenSSL : P-256 curve abstraction");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);
    X509V3Certificate certificate(fixture.certificate.der);
    expect(certificate.getPublicKeyType() == EVP_PKEY_EC, "P-256 certificate must contain an EC public key");
    expect(certificate.getPublicKeyCurveNID() == NID_X9_62_prime256v1, "P-256 certificate must report prime256v1");

    duox::DUOXCertificateValidator::validateForCurve(certificate, CurveID::NIST_P256);

    const auto parsed = duox::DUOXCertificateValidator::parse(fixture.certificate.der);
    expect(parsed.curve == CurveID::NIST_P256, "P-256 must map to CurveID::NIST_P256");

    std::cout << " P-256 curve abstraction validated.\n";
}

void testBrainpoolSupport()
{
    printSection("DUOX / OpenSSL : Brainpool P-256r1 support");

    if (!canGenerateCurve(NID_brainpoolP256r1))
        throw TestSkipped("OpenSSL runtime cannot generate Brainpool P-256r1");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_brainpoolP256r1);
    X509V3Certificate certificate(fixture.certificate.der);
    expect(certificate.getPublicKeyType() == EVP_PKEY_EC, "Brainpool certificate must contain EC key");
    expect(certificate.getPublicKeyCurveNID() == NID_brainpoolP256r1,
        "Brainpool certificate must report Brainpool P-256r1");

    duox::DUOXCertificateValidator::validateStructure(certificate);
    duox::DUOXCertificateValidator::validateForCurve(certificate, CurveID::BRAINPOOL_P256R1);

    const auto parsed = duox::DUOXCertificateValidator::parse(fixture.certificate.der);
    expect(parsed.curve == CurveID::BRAINPOOL_P256R1, "parsed Brainpool certificate must report Brainpool curve");
    expect(parsed.publicKey != nullptr, "Brainpool parsed public key must not be null");

    EVP_PKEY *evpPublicKey = parsed.publicKey->getPublicKey();
    expect(evpPublicKey != nullptr, "Brainpool PublicKey must contain EVP_PKEY");

    const ByteVector publicPoint = getUncompressedPublicPoint(evpPublicKey);
    expect(publicPoint.size() == 65, "Brainpool P-256r1 uncompressed point must be 65 bytes");
    expect(publicPoint[0] == 0x04, "Brainpool public key must use uncompressed point format");

    printInfo("Curve NID", certificate.getPublicKeyCurveNID());
    printInfo("Public point size", publicPoint.size());
    printInfo("Public point prefix", toHex(ByteVector{publicPoint[0]}));

    std::cout << " Brainpool P-256r1 support validated successfully.\n";
}

// ==============================================
// EC public-point encoding tests
// ==============================================

void testECUncompressedPublicPoint()
{
    printSection("EC public key : uncompressed point validation");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);
    X509V3Certificate certificate(fixture.certificate.der);

    const auto publicKey = certificate.getKey();
    expect(publicKey != nullptr, "certificate must expose a public key");

    EVP_PKEY *evpKey = publicKey->getPublicKey();
    expect(evpKey != nullptr, "PublicKey must contain EVP_PKEY");

    const ByteVector publicPoint = getUncompressedPublicPoint(evpKey);

    // P-256 : 0x04 || X (32 bytes) || Y (32 bytes)
    expect(publicPoint.size() == 65, "P-256 uncompressed public point must contain 65 bytes");
    expect(publicPoint.front() == 0x04, "EC public point must use uncompressed-point format");

    std::cout << " EC public point size/prefix validation succeeded.\n";
}

// ==============================================
// Diagnostics
// ==============================================

void testCertificateFixtureDiagnostics()
{
    printSection("Diagnostic : generated DUOX P-256 certificate");

    const CertificateFixture fixture = createSelfSignedCertificate(NID_X9_62_prime256v1);
    X509V3Certificate certificate(fixture.certificate.der);

    printInfo("DER length", certificate.getDERData().size());
    printInfo("Version", static_cast<int>(certificate.getVersion()));
    printInfo("Public key type", certificate.getPublicKeyType());
    printInfo("Curve", certificate.getPublicKeyCurveNID());
    printInfo("Signature algorithm", certificate.getSignatureAlgorithmNID());
    printInfo("Serial", toHex(certificate.getSerialNumber()));
    printInfo("TBS length", certificate.getTBSCertificateDER().size());
    printInfo("SPKI length", certificate.getSubjectPublicKeyInfoDER().size());
    printInfo("Signature length", certificate.getSignatureValue().size());

    const ByteVector &der = certificate.getDERData();
    const std::size_t prefixLength = std::min<std::size_t>(der.size(), 16);
    const ByteVector prefix(der.begin(), der.begin() + prefixLength);

    printInfo("DER prefix", toHex(prefix));

    std::cout << "\n Full OpenSSL certificate dump follows :\n";

    printCertificate(certificate.getDERData());
}

// ==============================================
// Test runner
// ==============================================

enum class TestStatus
{
    Passed,
    Failed,
    Skipped
};

struct TestResult
{
    std::string name;
    TestStatus status;
    std::string message;
};

template <typename Callable>
TestResult runTest(const std::string &name, Callable &&callable)
{
    std::cout << "\n[ RUN ] " << name;

    try
    {
        callable();
        std::cout << "[ OK ] " << name << '\n';

        return {name, TestStatus::Passed, {}};
    }
    catch (const TestSkipped &exception)
    {
        std::cout << "[ SKIPPED ] " << name << "\n" << exception.what() << '\n';

        return {name, TestStatus::Skipped, exception.what()};
    }
    catch (const TestFailure &exception)
    {
        std::cout << "[ FAILED ] " << name << "\n" << exception.what() << '\n';

        return {name, TestStatus::Failed, exception.what()};
    }
    catch (const std::exception &exception)
    {
        std::cout << "[ FAILED ] " << name << "\n" << "Unexpected exception : " << exception.what() << '\n';

        return {name, TestStatus::Failed, exception.what()};
    }
    catch (...)
    {
        std::cout << "[ FAILED ] " << name << "\n" << "Unknown exception" << '\n';

        return {name, TestStatus::Failed, "unknown exception"};
    }
}

} // namespace

int main()
{
    std::cout << "==============================================\n"
              << "LLA DUOX / X.509 CERTIFICATE TEST SUITE\n"
              << "==============================================\n";

    std::cout << "OpenSSL : " << OpenSSL_version(OPENSSL_VERSION) << '\n';

    std::vector<TestResult> results;

#define RUN_TEST(function) results.push_back(runTest(#function, function))

    // X509V3Certificate : input / lifecycle / representation
    RUN_TEST(testEmptyCertificate);
    RUN_TEST(testEmptyStringCertificate);
    RUN_TEST(testInvalidCertificate);
    RUN_TEST(testTrailingData);
    RUN_TEST(testValidCertificate);
    RUN_TEST(testMultipleCertificateInstances);
    RUN_TEST(testMoveConstruction);
    RUN_TEST(testMoveAssignment);
    RUN_TEST(testSetDERData);
    RUN_TEST(testSetDERDataString);
    RUN_TEST(testTransactionalReplacement);
    RUN_TEST(testLargeMalformedInput);

    // PublicKey ownership
    RUN_TEST(testPublicKeyOwnership);
    RUN_TEST(testPublicKeyInstancesAreIndependent);

    // X509V3Certificate : cryptographic verification
    RUN_TEST(testCertificateVerification);
    RUN_TEST(testCertificateVerificationWithWrongKey);
    RUN_TEST(testCertificateVerificationNullKey);

    // X509V3Certificate : exact DER / ASN.1 extraction
    RUN_TEST(testDERComponentExactEncoding);
    RUN_TEST(testSerialNumberEncoding);

    // DUOXCertificateValidator : single certificate
    RUN_TEST(testDUOXParse);
    RUN_TEST(testDUOXStructureValidation);
    RUN_TEST(testDUOXWrongCurveValidation);
    RUN_TEST(testDUOXWrongIssuerSignature);
    RUN_TEST(testDUOXNullIssuer);
    RUN_TEST(testDUOXInvalidCertificate);
    RUN_TEST(testDUOXEmptyCertificate);

    // DUOXCertificateValidator : certificate chain
    RUN_TEST(testSingleCertificateChain);
    RUN_TEST(testTwoCertificateChain);
    RUN_TEST(testChainWrongRoot);
    RUN_TEST(testChainWrongCurve);
    RUN_TEST(testChainEmpty);
    RUN_TEST(testChainTooLong);
    RUN_TEST(testChainMalformedIntermediate);

    // DUOXCertificateValidator : access rights
    RUN_TEST(testEffectiveAccessRightsZero);

    // EC curve abstraction
    RUN_TEST(testP256CurveAbstraction);
    RUN_TEST(testBrainpoolSupport);

    // EC public-point encoding
    RUN_TEST(testECUncompressedPublicPoint);

    // Diagnostics
    RUN_TEST(testCertificateFixtureDiagnostics);

#undef RUN_TEST

    std::size_t passed  = 0U;
    std::size_t failed  = 0U;
    std::size_t skipped = 0U;

    for (const TestResult &result : results)
    {
        switch (result.status)
        {
        case TestStatus::Passed: ++passed; break;
        case TestStatus::Failed: ++failed; break;
        case TestStatus::Skipped: ++skipped; break;
        }
    }

    std::cout << "\n"
              << "==============================================\n"
              << "TEST SUMMARY\n"
              << "==============================================\n";

    printInfo("Total", results.size());
    printInfo("Passed", passed);
    printInfo("Failed", failed);
    printInfo("Skipped", skipped);

    if (failed != 0)
    {
        std::cout << "\nFailed tests :\n";

        for (const TestResult &result : results)
        {
            if (result.status == TestStatus::Failed)
                std::cout << "  - " << result.name << " : " << result.message << '\n';
        }
        std::cout << "\nTEST SUITE RESULT : FAILURE\n";
        return EXIT_FAILURE;
    }
    std::cout << "\nTEST SUITE RESULT : SUCCESS\n";
    return EXIT_SUCCESS;
}
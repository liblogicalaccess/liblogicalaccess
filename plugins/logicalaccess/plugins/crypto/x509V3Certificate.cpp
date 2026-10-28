#include <logicalaccess/plugins/crypto/x509V3Certificate.hpp>

#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>

namespace logicalaccess
{

namespace
{

using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using BIGNUM_ptr = std::unique_ptr<BIGNUM, decltype(&BN_free)>;

// \brief Consume the complete OpenSSL error queue and return a readable representation of all errors currently present
std::string opensslErrorString()
{
    std::string result;
    bool first = true;

    while (const unsigned long error = ERR_get_error())
    {
        char buffer[256] = {};
        ERR_error_string_n(error, buffer, sizeof(buffer));

        if (!first)
            result += " | ";

        result += buffer;
        first = false;
    }

    return first ? "No OpenSSL error available" : result;
}

// \brief Create a runtime_error containing the current OpenSSL error queue.
std::runtime_error makeOpenSSLError(const char *operation)
{
    return std::runtime_error(std::string(operation) + " : " + opensslErrorString());
}

// \brief Ensure that an X509 certificate has been initialized.
void ensureCertificateInitialized(const X509 *certificate, const char *operation)
{
    if (certificate == nullptr)
        throw std::runtime_error(std::string(operation) + " : certificate is not initialized");
}

} // namespace

X509V3Certificate::X509V3Certificate()
    : _certificate(nullptr, X509_free)
{
}

X509V3Certificate::X509V3Certificate(const ByteVector &derData)
    : _certificate(nullptr, X509_free)
{
    setDERData(derData);
}

X509V3Certificate::X509V3Certificate(const std::string &derData)
    : _certificate(nullptr, X509_free)
{
    setDERData(derData);
}

void X509V3Certificate::setDERData(const ByteVector &derData)
{
    if (derData.empty())
        throw std::invalid_argument("x509V3Certificate::setDERData : certificate is empty");

    // Parse before modifying the current object. If parsing or allocation fails, the existing object remains unchanged
    X509Ptr parsed = parseDER(derData.data(), derData.size());

    // Copy the caller-provided DER representation before committing the new certificate
    // Throwing will leave the current certificate untouched
    std::vector<std::uint8_t> newDER(derData.begin(), derData.end());

    _derData     = std::move(newDER);
    _certificate = std::move(parsed);
}

void X509V3Certificate::setDERData(const std::string &derData)
{
    if (derData.empty())
        throw std::invalid_argument("x509V3Certificate::setDERData : certificate is empty");

    // derData is intentionally treated as binary data
    const auto *data = reinterpret_cast<const std::uint8_t *>(derData.data());

    X509Ptr parsed = parseDER(data, derData.size());

    std::vector<std::uint8_t> newDER(data, data + derData.size());

    // Commit only after potentially throwing operations
    _derData     = std::move(newDER);
    _certificate = std::move(parsed);
}

X509V3Certificate::X509Ptr X509V3Certificate::parseDER(const std::uint8_t *data, std::size_t size)
{
    if (data == nullptr)
        throw std::invalid_argument("x509V3Certificate::parseDER : null certificate data");

    if (size == 0)
        throw std::invalid_argument("x509V3Certificate::parseDER : empty certificate");

    // d2i_X509() accepts a long length. This prevent narrowing
    if (size > static_cast<std::size_t>(std::numeric_limits<long>::max()))
        throw std::invalid_argument("x509V3Certificate::parseDER : certificate is too large");

    // Need a pointer to keep the original data pointer as d2i_X509() advances the input pointer
    const unsigned char *cursor = reinterpret_cast<const unsigned char *>(data);
    X509 *rawCertificate = d2i_X509(nullptr, &cursor, static_cast<long>(size));

    if (rawCertificate == nullptr)
        throw makeOpenSSLError("x509V3Certificate::parseDER : d2i_X509 failed");

    // Transfer ownership so every exceptional exit automatically releases the parsed X509 object
    X509Ptr parsed{rawCertificate, X509_free};

    // A DER certificate must consume the complete supplied buffer
    const unsigned char *end = reinterpret_cast<const unsigned char *>(data) + size;
    if (cursor != end)
        throw std::invalid_argument("x509V3Certificate::parseDER : trailing data after certificate");

    return parsed;
}

const std::vector<std::uint8_t> &X509V3Certificate::getDERData() const noexcept
{
    return _derData;
}

std::shared_ptr<PublicKey> X509V3Certificate::getKey() const
{
    ensureCertificateInitialized(_certificate.get(), "x509V3Certificate::getKey");

    EVP_PKEY *key = X509_get_pubkey(_certificate.get());
    if (key == nullptr)
        throw makeOpenSSLError("x509V3Certificate::getKey : X509_get_pubkey failed");

    return std::make_shared<PublicKey>(key);
}

bool X509V3Certificate::verify(const std::shared_ptr<PublicKey> &issuerKey) const
{
    ensureCertificateInitialized(_certificate.get(), "x509V3Certificate::verify");

    if (!issuerKey)
        throw std::invalid_argument("x509V3Certificate::verify : issuer key is null");

    EVP_PKEY *key = issuerKey->getPublicKey();

    if (key == nullptr)
        throw std::invalid_argument("x509V3Certificate::verify : issuer key is invalid");

    const int result = X509_verify(_certificate.get(), key);

    if (result == 1)
        return true;
    if (result == 0)
        return false;

    throw makeOpenSSLError("x509V3Certificate::verify : X509_verify failed");
}

X509 *X509V3Certificate::getX509() noexcept
{
    return _certificate.get();
}

const X509 *X509V3Certificate::getX509() const noexcept
{
    return _certificate.get();
}

EVP_PKEY *X509V3Certificate::getEVPPublicKey() const
{
    ensureCertificateInitialized(_certificate.get(), "x509V3Certificate::getEVPPublicKey");

    EVP_PKEY *key = X509_get_pubkey(_certificate.get());
    if (key == nullptr)
        throw makeOpenSSLError("x509V3Certificate::getEVPPublicKey : X509_get_pubkey failed");

    return key;
}

ByteVector X509V3Certificate::getSubjectPublicKeyInfoDER() const
{
    ensureCertificateInitialized(_certificate.get(), "x509V3Certificate::getSubjectPublicKeyInfoDER");

    X509_PUBKEY *publicKey = X509_get_X509_PUBKEY(_certificate.get());

    if (publicKey == nullptr)
        throw makeOpenSSLError("x509V3Certificate::getSubjectPublicKeyInfoDER : X509_get_X509_PUBKEY failed");

    const int length = i2d_X509_PUBKEY(publicKey, nullptr);

    if (length <= 0)
        throw makeOpenSSLError("x509V3Certificate::getSubjectPublicKeyInfoDER : i2d_X509_PUBKEY failed");

    ByteVector result(static_cast<std::size_t>(length));

    unsigned char *cursor = result.data();
    const int encodedLength = i2d_X509_PUBKEY(publicKey, &cursor);

    if (encodedLength != length)
        throw std::runtime_error("x509V3Certificate::getSubjectPublicKeyInfoDER : unexpected DER encoding length");
    // Verify that exactly the allocated buffer was consumed
    if (cursor != result.data() + result.size())
        throw std::runtime_error("x509V3Certificate::getSubjectPublicKeyInfoDER : unexpected DER cursor position");

    return result;
}

std::int64_t X509V3Certificate::getVersion() const
{
    ensureCertificateInitialized(_certificate.get(), "x509V3Certificate::getVersion");

    return static_cast<std::int64_t>(X509_get_version(_certificate.get()));
}

ByteVector X509V3Certificate::getSerialNumber() const
{
    ensureCertificateInitialized(_certificate.get(), "x509V3Certificate::getSerialNumber");

    const ASN1_INTEGER *serial = X509_get_serialNumber(_certificate.get());
    if (serial == nullptr)
        throw makeOpenSSLError("x509V3Certificate::getSerialNumber : X509_get_serialNumber failed");

    BIGNUM *rawBN = ASN1_INTEGER_to_BN(serial, nullptr);
    if (rawBN == nullptr)
        throw makeOpenSSLError("x509V3Certificate::getSerialNumber : ASN1_INTEGER_to_BN failed");

    BIGNUM_ptr bn{rawBN, BN_free};
    if (BN_is_negative(bn.get()) != 0)
        throw std::runtime_error("x509V3Certificate::getSerialNumber : negative serial number");

    const int byteLength = BN_num_bytes(bn.get());
    if (byteLength < 0)
        throw std::runtime_error("x509V3Certificate::getSerialNumber : invalid serial number");

    ByteVector result(static_cast<std::size_t>(byteLength));

    if (byteLength == 0)
        return result;

    const int written = BN_bn2bin(bn.get(), result.data());
    if (written != byteLength)
        throw std::runtime_error("x509V3Certificate::getSerialNumber : failed to encode serial number");

    return result;
}

int X509V3Certificate::getSignatureAlgorithmNID() const
{
    ensureCertificateInitialized(_certificate.get(), "x509V3Certificate::getSignatureAlgorithmNID");

    const int nid = X509_get_signature_nid(_certificate.get());
    if (nid == NID_undef)
        throw std::runtime_error("x509V3Certificate::getSignatureAlgorithmNID : unknown signature algorithm");

    return nid;
}

int X509V3Certificate::getPublicKeyType() const
{
    ensureCertificateInitialized(_certificate.get(), "x509V3Certificate::getPublicKeyType");

    EVP_PKEY *rawKey = X509_get_pubkey(_certificate.get());
    if (rawKey == nullptr)
        throw makeOpenSSLError("x509V3Certificate::getPublicKeyType : X509_get_pubkey failed");

    EVP_PKEY_ptr key{rawKey, EVP_PKEY_free};
    const int type = EVP_PKEY_base_id(key.get());

    if (type == EVP_PKEY_NONE)
        throw std::runtime_error("x509V3Certificate::getPublicKeyType : unknown public key type");

    return type;
}

int X509V3Certificate::getPublicKeyCurveNID() const
{
    ensureCertificateInitialized(_certificate.get(), "x509V3Certificate::getPublicKeyCurveNID");

    EVP_PKEY *rawKey = X509_get_pubkey(_certificate.get());
    if (rawKey == nullptr)
        throw makeOpenSSLError("x509V3Certificate::getPublicKeyCurveNID : X509_get_pubkey failed");

    EVP_PKEY_ptr key{rawKey, EVP_PKEY_free};

    if (EVP_PKEY_base_id(key.get()) != EVP_PKEY_EC)
        return NID_undef;

    char groupName[128]         = {};
    std::size_t groupNameLength = 0;

    const int result = EVP_PKEY_get_group_name(key.get(), groupName, sizeof(groupName), &groupNameLength);

    if (result != 1)
        throw makeOpenSSLError("x509V3Certificate::getPublicKeyCurveNID : EVP_PKEY_get_group_name failed");
    if (groupNameLength == 0 || groupNameLength >= sizeof(groupName))
        throw std::runtime_error("x509V3Certificate::getPublicKeyCurveNID : invalid EC group name length");

    // Maps the textual object name to its NID
    const int curveNID = OBJ_txt2nid(groupName);
    if (curveNID == NID_undef)
        throw std::runtime_error("x509V3Certificate::getPublicKeyCurveNID : unknown EC curve");

    return curveNID;
}

ByteVector X509V3Certificate::getSignatureValue() const
{
    ensureCertificateInitialized(_certificate.get(), "x509V3Certificate::getSignatureValue");

    const ASN1_BIT_STRING *signature = nullptr;
    X509_get0_signature(&signature, nullptr, _certificate.get());

    if (signature == nullptr)
        throw std::runtime_error("x509V3Certificate::getSignatureValue : certificate has no signature");
    if (signature->length < 0)
        throw std::runtime_error("x509V3Certificate::getSignatureValue : invalid signature length");
    if (signature->length == 0)
        return {};
    if (signature->data == nullptr)
        throw std::runtime_error("x509V3Certificate::getSignatureValue : invalid signature data");

    return ByteVector(signature->data, signature->data + static_cast<std::size_t>(signature->length));
}

ByteVector X509V3Certificate::getTBSCertificateDER() const
{
    ensureCertificateInitialized(_certificate.get(), "x509V3Certificate::getTBSCertificateDER");

    // Re encode the TBS structure through OpenSSL
    const int length = i2d_re_X509_tbs(_certificate.get(), nullptr);

    if (length <= 0)
        throw makeOpenSSLError("x509V3Certificate::getTBSCertificateDER : i2d_re_X509_tbs failed");

    ByteVector result(static_cast<std::size_t>(length));

    unsigned char *cursor = result.data();
    const int encodedLength = i2d_re_X509_tbs(_certificate.get(), &cursor);

    if (encodedLength != length)
        throw std::runtime_error("x509V3Certificate::getTBSCertificateDER : unexpected DER encoding length");
    if (cursor != result.data() + result.size())
        throw std::runtime_error("x509V3Certificate::getTBSCertificateDER : unexpected DER cursor position");

    return result;
}

bool X509V3Certificate::isValid() const noexcept
{
    return _certificate != nullptr;
}

} // namespace logicalaccess
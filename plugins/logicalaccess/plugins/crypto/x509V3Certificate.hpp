#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/plugins/crypto/openssl.hpp>
#include <logicalaccess/plugins/crypto/public_key.hpp>

#include <openssl/evp.h>
#include <openssl/types.h>
#include <openssl/x509.h>

namespace logicalaccess
{

/**
 * \brief X.509 v3 certificate encoded in DER
 *
 * This class owns the underlying OpenSSL X509 object and keeps the exact DER encoding supplied by the caller
 *
 * Certificate representation and cryptographic primitives are kept here.
 * DUOX-specific certificate policy belongs to the DUOX authentication layer
 */
class LLA_CRYPTO_API X509V3Certificate
{
  public:
    /*
     * \brief Construct an empty certificate
     */
    X509V3Certificate();
    /*
     * \brief Construct a certificate from DER-encoded data
     */
    explicit X509V3Certificate(const ByteVector &derData);
    /*
     * \brief Construct a certificate from DER-encoded binary data
     */
    explicit X509V3Certificate(const std::string &derData);

    X509V3Certificate(const X509V3Certificate &)            = delete;
    X509V3Certificate &operator=(const X509V3Certificate &) = delete;

    X509V3Certificate(X509V3Certificate &&) noexcept            = default;
    X509V3Certificate &operator=(X509V3Certificate &&) noexcept = default;

    ~X509V3Certificate() = default;

    /**
     * \brief Replace the certificate with DER encoded data
     */
    void setDERData(const ByteVector &derData);

    /**
     * \brief Replace the certificate with DER encoded binary data
     *
     * This overload intentionally treats the string as binary data.
     * It mustn't be interpreted as PEM/text
     */
    void setDERData(const std::string &derData);

    /**
     * \brief Return the exact DER bytes supplied to setDERData()
     */
    const std::vector<std::uint8_t> &getDERData() const noexcept;

    /**
     * \brief Return the certificate public key
     *
     * A new PublicKey wrapper is returned and the returned PublicKey owns the extracted EVP_PKEY
     */
    std::shared_ptr<PublicKey> getKey() const;

    /**
     * \brief Verify this certificate's signature using the supplied public key
     *
     * This verifies : Certificate.Signature over Certificate.TBSCertificate
     *
     * It does not perform certificate-chain validation or certificate-policy validation
     *
     * \return true if the signature is valid
     */
    bool verify(const std::shared_ptr<PublicKey> &issuerKey) const;

    /**
     * \brief Return the OpenSSL certificate object
     *
     * The returned pointer remains owned by this object and must not be freed
     */
    X509 *getX509() noexcept;
    const X509 *getX509() const noexcept;

    /**
     * \brief Return the certificate's public key as an OpenSSL EVP_PKEY
     * 
     * X509_get_pubkey() creates a new EVP_PKEY object
     *
     * Ownership of the returned pointer is transferred to the caller.
     * The caller is responsible for releasing it with EVP_PKEY_free()
     * 
     * \return A newly allocated EVP_PKEY owned by the caller.
     */
    EVP_PKEY *getEVPPublicKey() const;

    /**
     * \brief Return the DER encoded SubjectPublicKeyInfo
     */
    ByteVector getSubjectPublicKeyInfoDER() const;

    /**
     * \brief Return X.509 version
     *
     * X.509 v3 is represented by value 2
     */
    std::int64_t getVersion() const;

    /**
     * \brief Return the serial number as a non-negative integer
     *
     * DUOX-specific serial-number length validation is deliberately left to the DUOX certificate validator
     */
    ByteVector getSerialNumber() const;

    /**
     * \brief Return the signature algorithm NID
     */
    int getSignatureAlgorithmNID() const;

    /**
     * \brief Return the certificate public-key type
     * 
     * The returned value is an OpenSSL EVP_PKEY type identifier
     */
    int getPublicKeyType() const;

    /**
     * \brief Return the certificate public-key curve NID
     *
     * Returns NID_undef if the certificate public key is not an EC key
     */
    int getPublicKeyCurveNID() const;

    /**
     * \brief Return the certificate signatureValue BIT STRING contents
     *
     * The returned bytes exclude the BIT STRING tag and length
     *
     * For ECDSA-signed certificates, these bytes contain the DER encoding of : SEQUENCE { r INTEGER, s INTEGER }
     */
    ByteVector getSignatureValue() const;

    /**
     * \brief Return the DER encoding of the TBSCertificate structure
     * 
     * The TBSCertificate is encoded by OpenSSL and the returned value includes the TBSCertificate ASN.1 tag and length
     */
    ByteVector getTBSCertificateDER() const;

    /**
     * \brief Check whether a parsed X.509 certificate is present
     */
    bool isValid() const noexcept;

  private:
    using X509Ptr = std::unique_ptr<X509, decltype(&X509_free)>;

    /**
     * \brief Parse DER data into an OpenSSL X509 object
     *
     * This function does not modify the current object.
     * The caller is responsible for committing the successfully parsed certificate and associated DER representation
     */
    static X509Ptr parseDER(const std::uint8_t *data, std::size_t size);

    X509Ptr _certificate{nullptr, X509_free};
    std::vector<std::uint8_t> _derData;
};

} // namespace logicalaccess
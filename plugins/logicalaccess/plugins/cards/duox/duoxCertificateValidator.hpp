#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <logicalaccess/plugins/cards/duox/duoxevcharging.hpp>
#include <logicalaccess/plugins/cards/duox/duoxecc.hpp>
#include <logicalaccess/plugins/cards/desfire/lla_cards_desfire_api.hpp>
#include <logicalaccess/plugins/crypto/public_key.hpp>
#include <logicalaccess/plugins/crypto/x509V3Certificate.hpp>

namespace logicalaccess
{

/**
 * \brief Parsed EV Charging certificate.
 *
 * This structure contains untrusted data until certificate verification succeeds
 */
struct EVChargingCertificate
{
    /**
     * \brief Raw certificate.
     *
     * Including outer 7F21 object
     */
    ByteVector rawData;

    /**
     * \brief Certificate serial number / UID encoding.
     * 
     * Extracted from TLV tag 0x93.
     */
    ByteVector uid;

    /**
     * \brief Certificate Authority identifier.
     *
     * Extracted from TLV tag 0x42.
     */
    ByteVector caId;

    /**
     * \brief Subject identifier.
     *
     * Extracted from TLV tag 0x5F20.
     */
    ByteVector subjectIdentifier;

    /**
     * \brief Key usage information.
     *
     * Extracted from TLV tag 0x95.
     */
    ByteVector keyUsage;

    /**
     * \brief Certificate effective date.
     *
     * Extracted from TLV tag 0x5F25.
     * YYYYMMDD, four bytes.
     */
    ByteVector effectiveDate;

    /**
     * \brief Certificate expiration date.
     *
     * Extracted from TLV tag 0x5F24.
     * YYYYMMDD, four bytes.
     */
    ByteVector expirationDate;

    /**
     * \brief CA Security Domain Image Number.
     *
     * Extracted from TLV tag 0x45.
     */
    std::uint8_t caSecurityDomainImageNumber = 0U;

    /**
     * \brief Raw public key data object.
     *
     * Extracted from TLV tag 0x7F49.
     */
    ByteVector publicKeyData;

    /**
     * \brief Uncompressed ECC public key.
     *
     * Extracted from TLV tag 0xB0
     *
     * Expected representation : 0x04 || X || Y
     */
    ByteVector publicKey;

    /**
     * \brief ECC key parameter reference.
     *
     * Extracted from TLV tag 0xF0
     */
    duox_ev_charging::EVChargingKeyParameterReference keyParameterReference =
        duox_ev_charging::EVChargingKeyParameterReference::BrainpoolP256r1;

    /**
     * \brief Raw ECDSA certificate signature.
     *
     * Extracted from TLV tag 0x5F37.
     *
     * Representation : r || s
     */
    ByteVector signature;

    /**
     * \brief Exact byte sequence covered by the certificate signature.
     *
     * Includes the encoded TLVs from tag 0x93 through tag 0x7F49.
     */
    ByteVector signedData;

    /**
     * \brief Set only after successful verification.
     */
    bool signatureVerified = false;
};

/**
 * \brief Calendar date used by EV Charging certificates.
 *
 * Certificate date encoding is YYYYMMDD in packed BCD :
 *
 * 20 26 01 02 = 2026-01-02
 */
struct EVChargingDate
{
    std::uint16_t year = 0U;
    std::uint8_t month = 0U;
    std::uint8_t day   = 0U;

    bool operator==(const EVChargingDate &other) const
    {
        return year == other.year && month == other.month && day == other.day;
    }

    bool operator<(const EVChargingDate &other) const
    {
        if (year != other.year)
            return year < other.year;

        if (month != other.month)
            return month < other.month;

        return day < other.day;
    }
};

namespace duox
{

/**
 * \brief Access rights carried by a certificate or granted by a CA root
 *
 * The exact interpretation of accessRight is DUOX policy-specific.
 * This structure contains only the raw access-right information and does not represent any derived policy
 */
struct CertificateAccessRights
{
    std::uint8_t arType;
    ByteVector arValue;

    CertificateAccessRights()
        : arType(0U)
        , arValue()
    {
    }
};

/**
 * Configuration associated with a ManageCARootKey entry.
 *
 * Caller must supply the configuration loaded from the targeted CA root key entry.
 * The public key is the external trust anchor.
 *
 * trustedIssuer is the exact issuer representation. An empty value disables the issuer-name restriction.
 *
 * This structure does not contain CRL data.
 * When certificate revocation is enabled, revocation must be enforced by the authentication layer.
 */
struct CARootKeyValidationParameters
{
    CurveID curve = CurveID::NIST_P256;
    CertificateAccessRights accessRights;
    // Empty means that no trusted issuer-name check is configured
    ByteVector trustedIssuer;
    // CRL revocation configuration from ManageCARootKey
    bool crlEnabled          = false;
    std::uint8_t crlFileNo   = 0U;
    std::uint32_t crlFileAid = 0U;
};

/**
 * \brief Cryptographically validated certificate
 *
 * Contains properties belonging to the certificate itself :
 * - the parsed X.509 certificate;
 * - its ECC public key;
 * - its DUOX supported curve;
 * - its serial number;
 * - its optional access-right extension
 *
 * No policy derived from a CA root key is stored here.
 */
struct ValidatedCertificate
{
    X509V3Certificate certificate;
    std::shared_ptr<PublicKey> publicKey;

    CurveID curve = CurveID::NIST_P256;
    ByteVector serialNumber;

    bool hasAccessRightsExtension = false;
    CertificateAccessRights accessRights;
};

/**
 * \brief Result of certificate-chain validation
 *
 * The supplied certificate chain is ordered from leaf to issuer :
 *     certificates[0] = leaf
 *     certificates[1] = intermediate/root-signed certificate, if present
 *
 * The CA root certificate itself is never supplied as part of the chain.
 * The targeted CA root public key is the external trust anchor.
 *
 * DUOX Cert.A currently supports at most :
 * leaf or leaf -> intermediate -> CA root trust anchor
 */
struct CertificateChainValidationResult
{
    ValidatedCertificate leaf;

    // Access rights granted by the certificate chain after applying the restrictions imposed by the targeted CA Root Key
    CertificateAccessRights effectiveAccessRights;

    // Number of certificates successfully validated. Currently this is either 1 or 2
    std::size_t certificateCount = 0U;
};

class LLA_CARDS_DESFIRE_API DUOXCertificateValidator
{
  public:
    /**
     * \brief Parse and structurally validate a single certificate
     *
     * The returned certificate contains its validated public key, curve, serial number and certificate-specific attributes
     *
     * The certificate's issuer signature is not verified by this function
     */
    static ValidatedCertificate parse(const ByteVector &certificateDER);

    /**
     * \brief Validate a certificate chain
     *
     * The certificates must be ordered from leaf to issuer :
     * [leaf] or [leaf, intermediate]
     *
     * The CA root remains an external trust anchor represented by caRootKey
     *
     * \param certificates Certificate DER objects ordered leaf-to-root
     * \param caRootKey External CA root public-key trust anchor
     * \param expectedCurve Curve required by the authentication context
     * \param caRootAccessRights Rights granted by the targeted CA root
     *
     * \return Validated leaf and effective access rights
     */
    static CertificateChainValidationResult validateChain(const std::vector<ByteVector> &certificates,
                  const std::shared_ptr<PublicKey> &caRootKey, const CurveID expectedCurve,
                  const CertificateAccessRights &caRootAccessRights);

    // Extended overload using metadata stored with ManageCARootKey
    static CertificateChainValidationResult validateChain(const std::vector<ByteVector> &certificates,
                  const std::shared_ptr<PublicKey> &caRootKey, const CARootKeyValidationParameters &rootParameters);

    /**
     * \brief Validate the certificate's DUOX-specific X.509 structure
     * This method does not verify the issuer signature
     */
    static void validateStructure(const X509V3Certificate &certificate);

    // \brief Validate that the certificate uses the expected ECC curve.
    static void validateForCurve(const X509V3Certificate &certificate, const CurveID expectedCurve);

    // \brief Verify the certificate signature with an issuer public key.
    static void validateSignature(const X509V3Certificate &certificate, const std::shared_ptr<PublicKey> &issuerKey);

    /**
     * \brief Calculate rights effective under a CA root
     *
     * Certificate rights may only restrict the rights granted by the targeted CA root - they must never expand them
     */
    static CertificateAccessRights calculateEffectiveAccessRights(const CertificateAccessRights &certificateRights,
        const CertificateAccessRights &caRootRights);

    // TODO : CRL retrieval and parsing; for now this is provided by the caller
    // This callback must return true only when the certificate is revoked.
    using RevocationCheck = bool (*)(std::uint32_t aid, std::uint8_t fileNo, const ByteVector &serialNumber);

    static void setRevocationCheck(RevocationCheck callback);
};

} // namespace duox

} // namespace logicalaccess
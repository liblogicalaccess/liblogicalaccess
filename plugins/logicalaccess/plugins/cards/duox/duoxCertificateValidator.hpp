#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <logicalaccess/plugins/cards/duox/duoxecc.hpp>
#include <logicalaccess/plugins/cards/desfire/lla_cards_desfire_api.hpp>
#include <logicalaccess/plugins/crypto/public_key.hpp>
#include <logicalaccess/plugins/crypto/x509V3Certificate.hpp>

namespace logicalaccess
{

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
    std::uint8_t accessRight;

    CertificateAccessRights()
        : arType(0)
        , accessRight(0)
    {
    }
};

/**
 * \brief Cryptographically validated certificate
 *
 * Contains properties belonging to the certificate itself :
 *
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

    CurveID curve;
    ByteVector serialNumber;

    bool hasAccessRightsExtension = false;
    CertificateAccessRights accessRights;
};

/**
 * \brief Result of certificate-chain validation
 *
 * The supplied certificate chain is ordered from leaf to issuer :
 *
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
};

} // namespace duox

} // namespace logicalaccess
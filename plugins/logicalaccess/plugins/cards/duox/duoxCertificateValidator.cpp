#include <logicalaccess/plugins/cards/duox/duoxCertificateValidator.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>

namespace logicalaccess
{

namespace duox
{

namespace
{

using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

constexpr std::size_t SERIAL_NUMBER_MAX_SIZE       = 4U;
constexpr std::size_t UNCOMPRESSED_P256_POINT_SIZE = 65U;
constexpr std::uint8_t UNCOMPRESSED_POINT_PREFIX   = 0x04U;

void ensureCertificateInitialized(const X509V3Certificate &certificate)
{
    if (!certificate.isValid())
        throw std::invalid_argument("Certificate is not initialized.");
}

void ensureIssuerKeyInitialized(const std::shared_ptr<PublicKey> &issuerKey)
{
    if (!issuerKey)
        throw std::invalid_argument("Issuer public key is null.");

    if (issuerKey->getPublicKey() == nullptr)
        throw std::invalid_argument("Issuer public key is invalid.");
}

CurveID curveFromNID(int curveNID)
{
    switch (curveNID)
    {
    case NID_X9_62_prime256v1: return CurveID::NIST_P256;

    case NID_brainpoolP256r1: return CurveID::BRAINPOOL_P256R1;

    default: throw std::runtime_error("Unsupported ECC curve.");
    }
}

void validateSerialNumber(const X509V3Certificate &certificate)
{
    const ByteVector serialNumber = certificate.getSerialNumber();

    if (serialNumber.empty())
        throw std::runtime_error("Certificate serial number is empty.");

    if (serialNumber.size() > SERIAL_NUMBER_MAX_SIZE)
        throw std::runtime_error("Certificate serial number exceeds four bytes.");
}

EVP_PKEY_ptr getCertificatePublicKey(const X509V3Certificate &certificate)
{
    EVP_PKEY_ptr publicKey(certificate.getEVPPublicKey(), &EVP_PKEY_free);

    if (!publicKey)
        throw std::runtime_error("Certificate does not contain a public key.");

    if (EVP_PKEY_base_id(publicKey.get()) != EVP_PKEY_EC)
        throw std::runtime_error("Certificate public key is not ECC.");

    return publicKey;
}

ByteVector getUncompressedPublicKey(const X509V3Certificate &certificate)
{
    EVP_PKEY_ptr publicKey = getCertificatePublicKey(certificate);
    std::size_t publicKeySize = 0U;

    if (EVP_PKEY_get_octet_string_param(publicKey.get(), OSSL_PKEY_PARAM_PUB_KEY, nullptr, 0U, &publicKeySize) != 1)
        throw std::runtime_error("Unable to determine EC public-key size.");

    ByteVector encodedPublicKey(publicKeySize);

    if (EVP_PKEY_get_octet_string_param(publicKey.get(), OSSL_PKEY_PARAM_PUB_KEY,
                                        encodedPublicKey.data(), encodedPublicKey.size(),
                                        &publicKeySize) != 1)
    {
        throw std::runtime_error("Unable to extract EC public key.");
    }

    encodedPublicKey.resize(publicKeySize);

    if (encodedPublicKey.size() != UNCOMPRESSED_P256_POINT_SIZE || encodedPublicKey.front() != UNCOMPRESSED_POINT_PREFIX)
        throw std::runtime_error("EC public key is not a 65-byte uncompressed point.");

    return encodedPublicKey;
}

void validatePublicKeyPoint(const X509V3Certificate &certificate)
{
    // DUOX requires the EC public key to be encoded as : 0x04 || X || Y with 32-byte X and Y coordinates
    (void)getUncompressedPublicKey(certificate);
}

std::shared_ptr<PublicKey> extractPublicKey(const X509V3Certificate &certificate)
{
    EVP_PKEY_ptr publicKey = getCertificatePublicKey(certificate);

    // PublicKey takes ownership of the EVP_PKEY returned by X509_get_pubkey()/X509V3Certificate
    return std::make_shared<PublicKey>(publicKey.release());
}

bool certificateNamesMatch(const X509V3Certificate &certificate, const X509V3Certificate &issuer)
{
    const X509 *certificateX509 = certificate.getX509();
    const X509 *issuerX509      = issuer.getX509();

    if (certificateX509 == nullptr || issuerX509 == nullptr)
        throw std::runtime_error("Invalid certificate object.");

    const X509_NAME *certificateIssuerName = X509_get_issuer_name(certificateX509);
    const X509_NAME *issuerSubjectName = X509_get_subject_name(issuerX509);

    if (certificateIssuerName == nullptr)
        throw std::runtime_error("Certificate has no issuer name.");

    if (issuerSubjectName == nullptr)
        throw std::runtime_error("Issuer certificate has no subject name.");

    return X509_NAME_cmp(certificateIssuerName, issuerSubjectName) == 0;
}

void validateCertificateIssuerRelationship(const X509V3Certificate &certificate, const X509V3Certificate &issuer)
{
    if (!certificateNamesMatch(certificate, issuer))
        throw std::runtime_error("Certificate issuer does not match issuer certificate subject.");
}

void validateCertificateCount(const std::vector<ByteVector> &certificates)
{
    if (certificates.empty())
        throw std::invalid_argument("DUOXCertificateValidator::validateChain : certificate chain is empty.");

    if (certificates.size() > 2U)
    {
        throw std::invalid_argument("DUOXCertificateValidator::validateChain : certificate chain "
            "contains more than two certificates.");
    }

    for (std::size_t index = 0U; index < certificates.size(); ++index)
    {
        if (certificates[index].empty())
        {
            throw std::invalid_argument("DUOXCertificateValidator::validateChain : certificate at index " +
                std::to_string(index) + " is empty.");
        }
    }
}

void validateCertificateCurves(const std::vector<ValidatedCertificate> &certificates, const CurveID expectedCurve)
{
    for (const ValidatedCertificate &certificate : certificates)
        DUOXCertificateValidator::validateForCurve(certificate.certificate, expectedCurve);
}

} // namespace

ValidatedCertificate DUOXCertificateValidator::parse(const ByteVector &certificateDER)
{
    if (certificateDER.empty())
        throw std::invalid_argument("DUOXCertificateValidator::parse : certificate is empty");

    // Construct the certificate locally and only publish it in the result after every validation step has succeeded
    X509V3Certificate certificate(certificateDER);
    validateStructure(certificate);

    ValidatedCertificate result;
    result.curve = curveFromNID(certificate.getPublicKeyCurveNID());
    result.publicKey = extractPublicKey(certificate);
    if (!result.publicKey)
        throw std::runtime_error("DUOXCertificateValidator::parse : failed to create certificate public-key wrapper.");
    result.serialNumber = certificate.getSerialNumber();
    if (result.serialNumber.empty())
        throw std::runtime_error("DUOXCertificateValidator::parse : certificate serial number is empty.");
    if (result.serialNumber.size() > SERIAL_NUMBER_MAX_SIZE)
        throw std::runtime_error("DUOXCertificateValidator::parse : certificate serial number exceeds four bytes.");

    // Access-right extensions are deliberately not inferred from generic X.509 extensions
    // Once the DUOX extension format is defined, parsing it belongs here
    result.hasAccessRightsExtension = false;
    result.accessRights             = CertificateAccessRights();

    result.certificate = std::move(certificate);

    return result;
}

CertificateChainValidationResult DUOXCertificateValidator::validateChain(const std::vector<ByteVector> &certificates,
    const std::shared_ptr<PublicKey> &caRootKey, const CurveID expectedCurve,
    const CertificateAccessRights &caRootAccessRights)
{
    validateCertificateCount(certificates);
    ensureIssuerKeyInitialized(caRootKey);

    std::vector<ValidatedCertificate> validatedCertificates;
    validatedCertificates.reserve(certificates.size());

    for (const ByteVector &certificateDER : certificates)
        validatedCertificates.emplace_back(parse(certificateDER));

    validateCertificateCurves(validatedCertificates, expectedCurve);

    /*
     * The CA root key is always the external trust anchor
     * One certificate :  leaf -> CA root key
     * Two certificates : leaf -> intermediate -> CA root key
     */
    if (validatedCertificates.size() == 1U)
    {
        validateSignature(validatedCertificates.front().certificate, caRootKey);
    }
    else
    {
        ValidatedCertificate &leaf = validatedCertificates[0];
        ValidatedCertificate &intermediate = validatedCertificates[1];
        validateCertificateIssuerRelationship(leaf.certificate, intermediate.certificate);
        // Intermediate is directly trusted by the external CA root trust anchor
        validateSignature(intermediate.certificate, caRootKey);
        // Leaf is signed by the intermediate certificate
        validateSignature(leaf.certificate, intermediate.publicKey);
    }

    CertificateAccessRights effectiveAccessRights = caRootAccessRights;
    const ValidatedCertificate &leaf = validatedCertificates.front();
    if (leaf.hasAccessRightsExtension)
        effectiveAccessRights = calculateEffectiveAccessRights(leaf.accessRights, caRootAccessRights);

    CertificateChainValidationResult result;
    result.leaf                  = std::move(validatedCertificates.front());
    result.effectiveAccessRights = effectiveAccessRights;
    result.certificateCount      = certificates.size();

    return result;
}

void DUOXCertificateValidator::validateStructure(const X509V3Certificate &certificate)
{
    ensureCertificateInitialized(certificate);

    // X.509 version encoding : 0 = v1, 1 = v2, 2 = v3
    if (certificate.getVersion() != 2)
        throw std::runtime_error("Certificate is not X.509 v3.");

    validateSerialNumber(certificate);

    // DUOX certificates use ECDSA with SHA-256
    if (certificate.getSignatureAlgorithmNID() != NID_ecdsa_with_SHA256)
        throw std::runtime_error("Certificate signature algorithm is not ECDSA with SHA-256.");

    // DUOX certificates must contain an ECC public key
    if (certificate.getPublicKeyType() != EVP_PKEY_EC)
        throw std::runtime_error("Certificate public key is not ECC.");

    // Ensure that the certificate uses one of the ECC curves supported by DUOX
    (void)curveFromNID(certificate.getPublicKeyCurveNID());

    // Ensure that the actual public point uses the DUOX-required uncompressed representation
    validatePublicKeyPoint(certificate);

    // A structurally valid X.509 certificate must contain a signature value
    if (certificate.getSignatureValue().empty())
        throw std::runtime_error("Certificate signature is empty.");
}

void DUOXCertificateValidator::validateForCurve(const X509V3Certificate &certificate, const CurveID expectedCurve)
{
    ensureCertificateInitialized(certificate);

    const int curveNID = certificate.getPublicKeyCurveNID();

    switch (expectedCurve)
    {
    case CurveID::NIST_P256:
        if (curveNID != NID_X9_62_prime256v1)
            throw std::runtime_error("DUOX certificate does not use NIST P-256.");
        return;

    case CurveID::BRAINPOOL_P256R1:
        if (curveNID != NID_brainpoolP256r1)
            throw std::runtime_error("DUOX certificate does not use Brainpool P-256r1.");
        return;

    default:
        throw std::invalid_argument("DUOXCertificateValidator::validateForCurve : unsupported DUOX curve.");
    }
}

void DUOXCertificateValidator::validateSignature(const X509V3Certificate &certificate,
    const std::shared_ptr<PublicKey> &issuerKey)
{
    ensureCertificateInitialized(certificate);
    ensureIssuerKeyInitialized(issuerKey);

    /*
     * Verifies the certificate signature over the TBSCertificate using the supplied issuer public key
     * It doesn't perform :
     * - chain construction
     * - trust-anchor discovery
     * - certificate-policy validation
     * - access-right evaluation
     * Those responsibilities remain in validateChain()
     */
    if (!certificate.verify(issuerKey))
        throw std::runtime_error("Certificate signature verification failed.");
}

CertificateAccessRights DUOXCertificateValidator::calculateEffectiveAccessRights(
    const CertificateAccessRights &certificateRights,
    const CertificateAccessRights &caRootRights)
{
    // Certificate rights may restrict CA-root rights but may never elevate them
    if (certificateRights.arType != caRootRights.arType)
        throw std::runtime_error("Certificate access-right type does not match targeted CA root access-right type.");

    CertificateAccessRights result;
    result.arType = caRootRights.arType;
    result.accessRight = static_cast<std::uint8_t>(certificateRights.accessRight & caRootRights.accessRight);

    return result;
}

} // namespace duox

} // namespace logicalaccess
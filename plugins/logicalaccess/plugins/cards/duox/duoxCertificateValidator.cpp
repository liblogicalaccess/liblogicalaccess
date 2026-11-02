#include <logicalaccess/plugins/cards/duox/duoxCertificateValidator.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <openssl/asn1.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/obj_mac.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

namespace logicalaccess
{

namespace duox
{

namespace
{

using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

constexpr const char *DUOX_AR_EXTENSION_OID = "1.3.6.1.4.1.28137.64.1";

constexpr std::size_t SERIAL_NUMBER_MAX_SIZE       = 4U;
constexpr std::size_t UNCOMPRESSED_P256_POINT_SIZE = 65U;
constexpr std::uint8_t UNCOMPRESSED_POINT_PREFIX   = 0x04U;

DUOXCertificateValidator::RevocationCheck g_revocationCheck = nullptr;

struct RawAccessRightsExtension
{
    bool present = false;
    ByteVector value;
};

RawAccessRightsExtension extractAccessRightsExtension(const X509V3Certificate &certificate)
{
    const X509 *x509 = certificate.getX509();

    if (x509 == nullptr)
        throw std::runtime_error("Invalid certificate object.");

    RawAccessRightsExtension result;
    const int extensionCount = X509_get_ext_count(x509);

    for (int index = 0; index < extensionCount; ++index)
    {
        X509_EXTENSION *extension = X509_get_ext(x509, index);

        if (extension == nullptr)
            throw std::runtime_error("Unable to retrieve certificate extension.");

        ASN1_OBJECT *object = X509_EXTENSION_get_object(extension);

        if (object == nullptr)
            throw std::runtime_error("Certificate extension has no OID.");

        char oid[128]       = {};
        const int oidLength = OBJ_obj2txt(oid, sizeof(oid), object, 1);

        if (oidLength <= 0 || static_cast<std::size_t>(oidLength) >= sizeof(oid))
        {
            throw std::runtime_error("Invalid or oversized certificate extension OID.");
        }

        if (std::string(oid) != DUOX_AR_EXTENSION_OID)
        {
            if (X509_EXTENSION_get_critical(extension) != 0)
            {
                throw std::runtime_error(
                    "Certificate contains an unsupported critical extension.");
            }
            // Unknown non-critical extensions are ignored by DUOX
            continue;
        }

        if (result.present)
        {
            throw std::runtime_error(
                "Certificate contains duplicate DUOX access-right extensions.");
        }

        ASN1_OCTET_STRING *data = X509_EXTENSION_get_data(extension);

        if (data == nullptr || data->length <= 0 || data->data == nullptr)
        {
            throw std::runtime_error("DUOX access-right extension is empty.");
        }

        result.present = true;
        result.value.assign(data->data, data->data + data->length);
    }

    return result;
}

CertificateAccessRights parseAccessRightsPayload(const ByteVector &payload)
{
    if (payload.empty())
        throw std::runtime_error("DUOX access-right extension payload is empty.");

    CertificateAccessRights result;
    result.arType = payload.front();
    result.arValue.assign(payload.begin() + 1, payload.end());

    switch (result.arType & 0x7FU)
    {
    case 0x01U: // Application rights specified by AID
    case 0x02U: // Application rights specified by DFName
    case 0x03U: // Delegated application management
        break;

    default: throw std::runtime_error("Unsupported DUOX access-right type.");
    }

    return result;
}

void validateAccessRightsPayload(const CertificateAccessRights &rights)
{
    const std::uint8_t type = rights.arType & 0x7FU;
    const ByteVector &value = rights.arValue;

    const auto validateACMap = [&value](std::size_t offset)
    {
        if (offset > value.size() || value.size() - offset < 2U)
            throw std::runtime_error("Truncated DUOX ACMap.");

        // ACMap is transmitted least-significant byte first.
        // Bits 15-14 are reserved and must be zero.
        if ((value[offset + 1U] & 0xC0U) != 0U)
            throw std::runtime_error("DUOX ACMap has non-zero reserved bits.");
    };

    switch (type)
    {
    case 0x01U: // AID-based application rights
        if (value.size() != 5U && value.size() != 10U)
            throw std::runtime_error("Invalid DUOX AID access-right payload length.");

        validateACMap(3U);

        if (value.size() == 10U)
            validateACMap(8U);

        break;

    case 0x02U: // DFName-based application rights
    {
        if (value.empty())
            throw std::runtime_error("Missing DUOX DFName length.");

        const std::size_t nameLength = value[0];

        if (nameLength < 1U || nameLength > 16U)
            throw std::runtime_error("Invalid DUOX DFName length.");

        if (value.size() != 1U + nameLength + 2U)
            throw std::runtime_error("Invalid DUOX DFName access-right payload length.");

        validateACMap(1U + nameLength);
        break;
    }

    case 0x03U: // Delegated application management
    {
        if (value.size() < 3U)
        {
            throw std::runtime_error("Truncated DUOX delegated-management payload.");
        }

        const std::size_t uidLength = value[0];

        if (uidLength != 0U && uidLength != 7U && uidLength != 10U)
            throw std::runtime_error("Invalid DUOX UID length.");

        const std::size_t operationOffset = 1U + uidLength;

        if (value.size() < operationOffset + 2U)
            throw std::runtime_error("Truncated DUOX delegated-management operation.");

        const std::uint8_t operation    = value[operationOffset];
        const std::size_t payloadLength = value[operationOffset + 1U];

        if (operation != 0U && operation != 1U)
            throw std::runtime_error("Unsupported DUOX delegated-management operation.");

        if (operation == 0U)
        {
            if (payloadLength < 10U || payloadLength > 33U)
            {
                throw std::runtime_error("Invalid DUOX application-creation payload length.");
            }
        }
        else if (payloadLength != 3U)
        {
            throw std::runtime_error("Invalid DUOX application-deletion payload length.");
        }

        if (value.size() != operationOffset + 2U + payloadLength)
            throw std::runtime_error("DUOX delegated-management payload length mismatch.");

        break;
    }

    default: throw std::runtime_error("Unsupported DUOX access-right type.");
    }
}

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

void validateCertificateValidity(const X509V3Certificate &certificate)
{
    const X509 *x509 = certificate.getX509();

    if (x509 == nullptr)
        throw std::runtime_error("Invalid certificate object.");

    const ASN1_TIME *notBefore = X509_get0_notBefore(x509);
    const ASN1_TIME *notAfter  = X509_get0_notAfter(x509);

    if (notBefore == nullptr || notAfter == nullptr)
        throw std::runtime_error("Certificate is missing its validity period.");

    if (ASN1_TIME_check(notBefore) != 1 || ASN1_TIME_check(notAfter) != 1)
        throw std::runtime_error("Certificate contains an invalid validity time.");

    const int beforeResult = X509_cmp_current_time(notBefore);
    const int afterResult  = X509_cmp_current_time(notAfter);

    // A zero result means the comparison failed
    if (beforeResult == 0 || afterResult == 0)
        throw std::runtime_error("Unable to evaluate certificate validity period.");

    if (beforeResult > 0)
        throw std::runtime_error("Certificate is not yet valid.");

    if (afterResult < 0)
        throw std::runtime_error("Certificate has expired.");
}

void validateIntermediateCA(const X509V3Certificate &certificate)
{
    const X509 *x509 = certificate.getX509();

    if (x509 == nullptr)
        throw std::runtime_error("Invalid intermediate certificate.");

    BASIC_CONSTRAINTS *rawConstraints = static_cast<BASIC_CONSTRAINTS *>(
        X509_get_ext_d2i(x509, NID_basic_constraints, nullptr, nullptr));

    std::unique_ptr<BASIC_CONSTRAINTS, decltype(&BASIC_CONSTRAINTS_free)> constraints(
        rawConstraints, BASIC_CONSTRAINTS_free);

    if (!constraints)
        throw std::runtime_error(
            "Intermediate certificate has no valid Basic Constraints.");

    if (constraints->ca == 0)
        throw std::runtime_error("Intermediate certificate is not a CA.");

    ASN1_BIT_STRING *rawUsage = static_cast<ASN1_BIT_STRING *>(X509_get_ext_d2i(x509, NID_key_usage, nullptr, nullptr));

    std::unique_ptr<ASN1_BIT_STRING, decltype(&ASN1_BIT_STRING_free)> usage(rawUsage, ASN1_BIT_STRING_free);

    if (!usage)
        throw std::runtime_error("Intermediate certificate has no valid Key Usage extension.");

    if (ASN1_BIT_STRING_get_bit(usage.get(), 5) != 1)
        throw std::runtime_error("Intermediate certificate lacks keyCertSign usage.");

    // TODO check later : the supplied chain has exactly one intermediate, so pathLenConstraint must permit zero subordinate
    // CA certificates below it. The leaf does not count as a CA.

    // If pathLenConstraint is present, it must allow the subordinate CA certificates in this chain.
    if (constraints->pathlen != nullptr)
    {
        const long pathLength = ASN1_INTEGER_get(constraints->pathlen);

        // This accepts only a leaf beneath the intermediate; it does not count the leaf as a CA
        if (pathLength < 0)
            throw std::runtime_error("Invalid intermediate CA path length.");
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
    EVP_PKEY_ptr publicKey    = getCertificatePublicKey(certificate);
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

std::shared_ptr<PublicKey> extractPublicKey(const X509V3Certificate &certificate)
{
    EVP_PKEY_ptr publicKey = getCertificatePublicKey(certificate);

    // PublicKey takes ownership of the EVP_PKEY returned by X509_get_pubkey()/X509V3Certificate
    return std::make_shared<PublicKey>(publicKey.release());
}

void validateIssuerName(const X509V3Certificate &certificate, const ByteVector &trustedIssuer)
{
    if (trustedIssuer.empty())
        return;

    const X509 *x509 = certificate.getX509();

    if (x509 == nullptr)
        throw std::runtime_error("Invalid certificate object.");

    const X509_NAME *issuerName = X509_get_issuer_name(x509);

    if (issuerName == nullptr)
        throw std::runtime_error("Certificate has no issuer name.");

    // TODO : ManageCARootKey's Issuer bytes are the DER encoding of an X.509 Name;
    //        If the protocol defines Issuer as a different representation, replace this comparison accordingly
    const int encodedLength = i2d_X509_NAME(const_cast<X509_NAME *>(issuerName), nullptr);

    if (encodedLength <= 0)
        throw std::runtime_error("Unable to encode certificate issuer name.");

    ByteVector encodedIssuer(static_cast<std::size_t>(encodedLength));
    unsigned char *cursor = encodedIssuer.data();

    if (i2d_X509_NAME(const_cast<X509_NAME *>(issuerName), &cursor) != encodedLength)
        throw std::runtime_error("Unable to encode certificate issuer name.");

    if (encodedIssuer != trustedIssuer)
        throw std::runtime_error("Certificate issuer does not match the configured trusted issuer.");
}

bool certificateNamesMatch(const X509V3Certificate &certificate, const X509V3Certificate &issuer)
{
    const X509 *certificateX509 = certificate.getX509();
    const X509 *issuerX509      = issuer.getX509();

    if (certificateX509 == nullptr || issuerX509 == nullptr)
        throw std::runtime_error("Invalid certificate object.");

    const X509_NAME *certificateIssuerName = X509_get_issuer_name(certificateX509);
    const X509_NAME *issuerSubjectName     = X509_get_subject_name(issuerX509);

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
        throw std::invalid_argument("DUOXCertificateValidator::validateChain : certificate chain "
            "contains more than two certificates.");

    for (const ByteVector &der : certificates)
    {
        if (der.empty())
            throw std::invalid_argument("Empty certificate in DUOX chain.");
    }
}

} // namespace

void DUOXCertificateValidator::setRevocationCheck(RevocationCheck callback)
{
    g_revocationCheck = callback;
}

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

    const RawAccessRightsExtension extension = extractAccessRightsExtension(certificate);

    result.hasAccessRightsExtension = extension.present;

    if (extension.present)
    {
        result.accessRights = parseAccessRightsPayload(extension.value);
        validateAccessRightsPayload(result.accessRights);
    }

    result.certificate = std::move(certificate);

    return result;
}

CertificateChainValidationResult DUOXCertificateValidator::validateChain(const std::vector<ByteVector> &certificates,
                                        const std::shared_ptr<PublicKey> &caRootKey,
                                        CurveID expectedCurve,
                                        const CertificateAccessRights &caRootAccessRights)
{
    CARootKeyValidationParameters parameters;
    parameters.curve        = expectedCurve;
    parameters.accessRights = caRootAccessRights;

    return validateChain(certificates, caRootKey, parameters);
}

CertificateChainValidationResult DUOXCertificateValidator::validateChain(
    const std::vector<ByteVector> &certificates,
    const std::shared_ptr<PublicKey> &caRootKey,
    const CARootKeyValidationParameters &rootParameters)
{
    validateCertificateCount(certificates);
    ensureIssuerKeyInitialized(caRootKey);

    std::vector<ValidatedCertificate> validatedCertificates;
    validatedCertificates.reserve(certificates.size());

    for (const ByteVector &der : certificates)
    {
        validatedCertificates.emplace_back(parse(der));
        validateForCurve(validatedCertificates.back().certificate, rootParameters.curve);
    }

    // The leaf's issuer must match the trusted issuer configured for the
    // targeted CA root key, when such a name has been configured.
    validateIssuerName(validatedCertificates.front().certificate, rootParameters.trustedIssuer);

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
        ValidatedCertificate &leaf         = validatedCertificates[0];
        ValidatedCertificate &intermediate = validatedCertificates[1];
        validateCertificateIssuerRelationship(leaf.certificate, intermediate.certificate);
        
        // TODO (assumption) the intermediate certificate is expected to be issued directly by the configured CA root public key.
        // The CA root itself is an external trust anchor, not a certificate included in the supplied chain.
        validateStructure(validatedCertificates[1].certificate);

        validateIntermediateCA(intermediate.certificate);

        validateIssuerName(intermediate.certificate, rootParameters.trustedIssuer);
        // Intermediate is directly trusted by the external CA root trust anchor
        validateSignature(intermediate.certificate, caRootKey);
        // Leaf is signed by the intermediate certificate
        validateSignature(leaf.certificate, intermediate.publicKey);

    }

    if (rootParameters.crlEnabled)
    {
        if (g_revocationCheck == nullptr)
        {
            // TODO (missing implementation) connect this callback to the file API
            // Read the configured CRL file from crlFileAid and crlFileNo
            // parse and validate the CRL signature and validity period, and check each certificate serial number
            throw std::runtime_error("Certificate revocation is enabled but no CRL checker is configured.");
        }

        for (const ValidatedCertificate &certificate : validatedCertificates)
        {
            if (g_revocationCheck(rootParameters.crlFileAid, rootParameters.crlFileNo,
                                  certificate.serialNumber))
            {
                throw std::runtime_error("DUOX certificate has been revoked.");
            }
        }
    }

    CertificateAccessRights effectiveAccessRights = rootParameters.accessRights;
    const ValidatedCertificate &leaf = validatedCertificates.front();
    if (leaf.hasAccessRightsExtension)
        effectiveAccessRights = calculateEffectiveAccessRights(leaf.accessRights, rootParameters.accessRights);

    CertificateChainValidationResult result;
    result.leaf                  = std::move(validatedCertificates.front());
    result.effectiveAccessRights = std::move(effectiveAccessRights);
    result.certificateCount      = certificates.size();

    return result;
}

void DUOXCertificateValidator::validateStructure(const X509V3Certificate &certificate)
{
    ensureCertificateInitialized(certificate);
    validateCertificateValidity(certificate);

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
    // DUOX requires the EC public key to be encoded as : 0x04 || X || Y with 32-byte X and Y coordinates
    (void)getUncompressedPublicKey(certificate);

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
    const std::uint8_t certificateType = certificateRights.arType & 0x7FU;
    const std::uint8_t rootType        = caRootRights.arType & 0x7FU;

    // Certificate rights may restrict CA-root rights but may never elevate them
    if (certificateType != rootType)
        throw std::runtime_error("Certificate access-right type does not match targeted CA root access-right type.");

    const ByteVector &certificateValue = certificateRights.arValue;
    const ByteVector &rootValue        = caRootRights.arValue;

    if (certificateValue.size() != rootValue.size())
        throw std::runtime_error("Certificate and CA root access-right payload lengths differ.");

    // Validate both payloads before calculating effective permissions
    validateAccessRightsPayload(certificateRights);
    validateAccessRightsPayload(caRootRights);

    CertificateAccessRights result;
    result.arType  = caRootRights.arType;
    result.arValue = rootValue;

    const auto intersectACMap = [&certificateValue, &rootValue, &result](std::size_t offset)
    {
        if (offset > certificateValue.size() || certificateValue.size() - offset < 2U)
            throw std::runtime_error("Truncated ACMap in access-rights payload.");

        // ACMap is assumed to be little-endian, as in the existing DUOX payload validator
        const std::uint16_t certificateMap =
            static_cast<std::uint16_t>(certificateValue[offset]) |
            (static_cast<std::uint16_t>(certificateValue[offset + 1U]) << 8U);

        const std::uint16_t rootMap =
            static_cast<std::uint16_t>(rootValue[offset]) |
            (static_cast<std::uint16_t>(rootValue[offset + 1U]) << 8U);

        // A certificate must never grant permissions absent from the CA root's corresponding access map
        const std::uint16_t effectiveMap = certificateMap & rootMap;

        result.arValue[offset] = static_cast<std::uint8_t>(effectiveMap & 0xFFU);
        result.arValue[offset + 1U] = static_cast<std::uint8_t>((effectiveMap >> 8U) & 0xFFU);
    };

    switch (certificateType)
    {
    case 0x01U: // AID-based application rights
        if (certificateValue.size() != 5U && certificateValue.size() != 10U)
            throw std::runtime_error("Invalid AID-based access-right payload length.");

        // The identifiers must match before their permissions can be intersected
        if (!std::equal(certificateValue.begin(), certificateValue.begin() + 3, rootValue.begin()))
            throw std::runtime_error("AID access-right identifiers do not match.");

        intersectACMap(3U);

        if (certificateValue.size() == 10U)
        {
            if (!std::equal(certificateValue.begin() + 5, certificateValue.begin() + 8, rootValue.begin() + 5))
            {
                throw std::runtime_error("Second AID access-right identifiers do not match.");
            }
            intersectACMap(8U);
        }

        break;

    case 0x02U: // DFName-based application rights
    {
        const std::size_t nameLength = certificateValue[0];

        if (certificateValue[0] != rootValue[0] ||
            !std::equal(certificateValue.begin() + 1, certificateValue.begin() + 1 + nameLength, rootValue.begin() + 1))
        {
            throw std::runtime_error("DFName access-right identifiers do not match.");
        }

        intersectACMap(1U + nameLength);
        break;
    }

    case 0x03U:
        // TODO delegated application management rights to intersected for delegated application-management operations
        throw std::runtime_error("Effective delegated-management rights require the authoritative DUOX policy.");

    default: throw std::runtime_error("Unsupported DUOX access-right type.");
    }

    return result;
}

} // namespace duox

} // namespace logicalaccess
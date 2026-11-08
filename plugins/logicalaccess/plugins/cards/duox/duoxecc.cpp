#include <logicalaccess/plugins/cards/duox/duoxecc.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>

#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/param_build.h>

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>

namespace logicalaccess
{

namespace
{
// Number of bytes used by one affine coordinate for DUOX P-256 curves
constexpr std::size_t DUOX_P256_COORDINATE_SIZE = 32U;

// ==============================================
// Generic helpers
// ==============================================
[[noreturn]] void fail(const std::string &message)
{
    throw LibLogicalAccessException(message);
}

template <typename T>
void require(const T &condition, const std::string &message)
{
    if (!static_cast<bool>(condition))
        fail(message);
}

template <typename T>
void requireNotNull(const T *value, const char *message)
{
    if (value == nullptr)
        fail(message);
}

void requireOpenSSL(bool success, const std::string &message)
{
    require(success, message);
}

// ==============================================
// OpenSSL resource management
// ==============================================

// Give every resource deterministic cleanup, including when an exception is thrown
using ECGroupPtr = std::unique_ptr<EC_GROUP, decltype(&EC_GROUP_free)>;
using ECPointPtr = std::unique_ptr<EC_POINT, decltype(&EC_POINT_free)>;
using BNContextPtr = std::unique_ptr<BN_CTX, decltype(&BN_CTX_free)>;
using BIGNUMPtr = std::unique_ptr<BIGNUM, decltype(&BN_free)>;

using EVP_PKEYPtr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using EVP_MD_CTXPtr = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;
using EVP_PKEY_CTXPtr = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
using ECDSASigPtr = std::unique_ptr<ECDSA_SIG, decltype(&ECDSA_SIG_free)>;

// ==============================================
// Curve utilities
// ==============================================

// \brief Return the OpenSSL NID corresponding to a DUOX curve
int getOpenSSLCurveNID(CurveID curve)
{
    switch (curve)
    {
    case CurveID::NIST_P256: return NID_X9_62_prime256v1;

    case CurveID::BRAINPOOL_P256R1: return NID_brainpoolP256r1;

    default: fail("Unsupported DUOX elliptic curve");
    }
}

// \brief Return the OpenSSL curve name corresponding to a DUOX curve
const char *getOpenSSLCurveName(CurveID curve)
{
    switch (curve)
    {
    case CurveID::NIST_P256: return "prime256v1";

    case CurveID::BRAINPOOL_P256R1: return "brainpoolP256r1";

    default: fail("Unsupported DUOX elliptic curve");
    }
}

/**
 * \brief Create an OpenSSL EC group for a DUOX curve
 *
 * Ownership of the returned object belongs to the caller
 */
ECGroupPtr createECGroup(CurveID curve)
{
    const int nid = getOpenSSLCurveNID(curve);

    ECGroupPtr group(EC_GROUP_new_by_curve_name(nid), &EC_GROUP_free);

    requireNotNull(group.get(), "Failed to create OpenSSL EC group for DUOX curve.");

    return group;
}

// ==============================================
// BIGNUM utilities
// ==============================================

// \brief Check whether a ByteVector can safely be passed to OpenSSL functions that accept an int length
bool isValidBignumInput(const ByteVector &value)
{
    return value.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max());
}

// \brief Convert a big-endian byte vector into an OpenSSL BIGNUM
BIGNUMPtr coordinateToBignum(const ByteVector &coordinate)
{
    require(isValidBignumInput(coordinate), "DUOX EC coordinate is too large for OpenSSL BIGNUM conversion");

    BIGNUMPtr value(BN_bin2bn(coordinate.data(), static_cast<int>(coordinate.size()), nullptr), &BN_free);

    requireNotNull(value.get(), "Failed to convert DUOX EC coordinate to OpenSSL BIGNUM.");

    return value;
}

// ==============================================
// EC point utilities
// ==============================================

/**
 * \brief Create an OpenSSL EC public key from a DUOX affine point
 *
 * The point is validated before being imported
 */
ECPointPtr createECPoint(const EC_GROUP *group, const ECPoint &point, BN_CTX *bnContext)
{
    requireNotNull(group, "Invalid OpenSSL EC group.");
    requireNotNull(bnContext, "Invalid OpenSSL BN context.");

    ECPointPtr ecPoint(EC_POINT_new(group), &EC_POINT_free);

    requireNotNull(ecPoint.get(), "Failed to allocate OpenSSL EC point.");

    const BIGNUMPtr x = coordinateToBignum(point.x);
    const BIGNUMPtr y = coordinateToBignum(point.y);

    requireOpenSSL(EC_POINT_set_affine_coordinates(group, ecPoint.get(), x.get(), y.get(), bnContext) == 1,
                   "Invalid DUOX EC public point");

    return ecPoint;
}

// ==============================================
// Private key validation
// ==============================================

/**
 * \brief Validate a DUOX private scalar
 *
 * A private scalar must satisfy : 1 <= privateKey < curve order
 */
void validatePrivateKey(const EC_GROUP *group, const ByteVector &privateKey, BN_CTX *bnContext)
{
    requireNotNull(group, "Invalid OpenSSL EC group.");
    requireNotNull(bnContext, "Invalid OpenSSL BN context.");

    require(!privateKey.empty(), "DUOX EC private key is empty");

    require(isValidBignumInput(privateKey), "DUOX EC private key is too large for OpenSSL BIGNUM conversion");

    const BIGNUMPtr privateKeyBN = coordinateToBignum(privateKey);
    BIGNUMPtr order(BN_new(), &BN_free);

    requireNotNull(order.get(), "Failed to allocate OpenSSL EC order BIGNUM.");

    requireOpenSSL(EC_GROUP_get_order(group, order.get(), bnContext) == 1, "Failed to retrieve OpenSSL EC group order");

    require(!BN_is_zero(privateKeyBN.get()) && !BN_is_negative(privateKeyBN.get()),
            "DUOX EC private key must be greater than zero");

    require(BN_cmp(privateKeyBN.get(), order.get()) < 0, "DUOX EC private key is outside the valid scalar range");
}

// ==============================================
// EVP private key construction
// ==============================================

// \brief Create an OpenSSL EVP private key from a DUOX private scalar
EVP_PKEYPtr createECPrivateKey(CurveID curve, const ByteVector &privateKey)
{
    require(!privateKey.empty(), "DUOX EC private key is empty");
    require(isValidBignumInput(privateKey), "DUOX EC private key is too large");

    const int nid = getOpenSSLCurveNID(curve);
    ECGroupPtr group = createECGroup(curve);
    BNContextPtr bnContext(BN_CTX_new(), &BN_CTX_free);

    requireNotNull(bnContext.get(), "Failed to allocate OpenSSL BN context.");

    validatePrivateKey(group.get(), privateKey, bnContext.get());

    const BIGNUMPtr privateKeyBN = coordinateToBignum(privateKey);

    // Derive the public point
    ECPointPtr publicPoint(EC_POINT_new(group.get()), &EC_POINT_free);

    requireNotNull(publicPoint.get(), "Failed to allocate OpenSSL EC public point.");

    requireOpenSSL(EC_POINT_mul(group.get(), publicPoint.get(), privateKeyBN.get(), nullptr, nullptr, bnContext.get()) == 1,
                   "Failed to derive DUOX EC public key");

    // Extract public coordinates
    const std::size_t coordinateSize = getCoordinateSize(curve);

    BIGNUMPtr publicX(BN_new(), &BN_free);
    BIGNUMPtr publicY(BN_new(), &BN_free);

    requireNotNull(publicX.get(), "Failed to allocate OpenSSL EC public X coordinate.");
    requireNotNull(publicY.get(), "Failed to allocate OpenSSL EC public Y coordinate.");

    requireOpenSSL(EC_POINT_get_affine_coordinates(group.get(), publicPoint.get(),
                                                   publicX.get(), publicY.get(),
                                                   bnContext.get()) == 1,
                   "Failed to extract OpenSSL EC public coordinates");

    // Encode the public point as 0x04 || X || Y

    ByteVector encodedPublicKey(1U + (2U * coordinateSize));

    encodedPublicKey[0] = 0x04U;

    requireOpenSSL(BN_bn2binpad(publicX.get(), encodedPublicKey.data() + 1U,
                                static_cast<int>(coordinateSize)) == static_cast<int>(coordinateSize),
                   "Failed to encode DUOX EC public X coordinate");

    requireOpenSSL(BN_bn2binpad(publicY.get(), encodedPublicKey.data() + 1U + coordinateSize,
                                static_cast<int>(coordinateSize)) == static_cast<int>(coordinateSize),
                   "Failed to encode DUOX EC public Y coordinate");

    // Create the EVP key context

    EVP_PKEY_CTXPtr keyContext(EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr), &EVP_PKEY_CTX_free);

    requireNotNull(keyContext.get(), "Failed to allocate OpenSSL EC key context.");

    requireOpenSSL(EVP_PKEY_fromdata_init(keyContext.get()) > 0, "Failed to initialize OpenSSL EC key import");

    const char *curveName = OBJ_nid2sn(nid);

    requireNotNull(curveName, "Failed to determine OpenSSL DUOX curve name.");
    
    // Build key parameters

    OSSL_PARAM_BLD *paramBuilder = OSSL_PARAM_BLD_new();

    requireNotNull(paramBuilder, "Failed to allocate OpenSSL EC parameter builder.");

    struct ParamBuilderGuard
    {
        OSSL_PARAM_BLD *builder;

        ~ParamBuilderGuard()
        {
            if (builder != nullptr)
                OSSL_PARAM_BLD_free(builder);
        }
    } paramBuilderGuard{paramBuilder};

    requireOpenSSL(OSSL_PARAM_BLD_push_utf8_string(paramBuilder, OSSL_PKEY_PARAM_GROUP_NAME, curveName, 0) == 1,
                   "Failed to configure OpenSSL DUOX EC curve");

    requireOpenSSL(OSSL_PARAM_BLD_push_BN(paramBuilder, OSSL_PKEY_PARAM_PRIV_KEY, privateKeyBN.get()) == 1,
                   "Failed to configure OpenSSL DUOX EC private key");

    requireOpenSSL(OSSL_PARAM_BLD_push_octet_string(paramBuilder, OSSL_PKEY_PARAM_PUB_KEY,
                                                    encodedPublicKey.data(),
                                                    encodedPublicKey.size()) == 1,
                   "Failed to configure OpenSSL DUOX EC public key");

    OSSL_PARAM *params = OSSL_PARAM_BLD_to_param(paramBuilder);

    requireNotNull(params, "Failed to build OpenSSL DUOX EC parameters.");

    struct ParamGuard
    {
        OSSL_PARAM *params;

        ~ParamGuard()
        {
            if (params != nullptr)
                OSSL_PARAM_free(params);
        }
    } paramGuard{params};

    EVP_PKEY *rawKey = nullptr;

    requireOpenSSL(EVP_PKEY_fromdata(keyContext.get(), &rawKey, EVP_PKEY_KEYPAIR, params) > 0,
        "Failed to construct OpenSSL DUOX EC private key");

    EVP_PKEYPtr pkey(rawKey, &EVP_PKEY_free);

    requireNotNull(pkey.get(), "OpenSSL returned a null DUOX EC private key.");

    return pkey;
}

// ==============================================
// ECDSA utilities
// ==============================================

// \brief Validate one fixed-width ECDSA signature component
void validateSignatureComponent(const ByteVector &value, const char *name)
{
    require(value.size() == DUOX_P256_COORDINATE_SIZE,
            std::string("DUOX ECDSA ") + name + " must contain exactly 32 bytes.");
}

// \brief Check whether an EVP key uses the requested DUOX curve
bool keyUsesCurve(EVP_PKEY *key, CurveID curve)
{
    if (key == nullptr)
        return false;

    if (EVP_PKEY_base_id(key) != EVP_PKEY_EC)
        return false;

    char groupName[128]         = {};
    std::size_t groupNameLength = 0;

    if (EVP_PKEY_get_group_name(key, groupName, sizeof(groupName), &groupNameLength) != 1)
        return false;

    return std::string(groupName) == getOpenSSLCurveName(curve);
}

} // namespace

// ==============================================
// Public curve utilities
// ==============================================

std::size_t getCoordinateSize(CurveID curve)
{
    switch (curve)
    {
    case CurveID::NIST_P256:
    case CurveID::BRAINPOOL_P256R1: return DUOX_P256_COORDINATE_SIZE;

    default: fail("Unsupported DUOX elliptic curve");
    }
}

// ==============================================
// EC point encoding and decoding
// ==============================================

ECPoint decodeECPoint(CurveID curve, const ByteVector &encoded)
{
    const std::size_t coordinateSize = getCoordinateSize(curve);
    const std::size_t expectedSize   = 1U + (2U * coordinateSize);

    require(encoded.size() == expectedSize, "Invalid DUOX EC point length.");
    require(encoded.front() == 0x04U, "DUOX EC point is not an uncompressed point.");

    ECPoint point;
    const auto xBegin = encoded.begin() + 1;
    const auto xEnd   = xBegin + coordinateSize;
    const auto yEnd   = encoded.end();

    point.x.assign(xBegin, xEnd);
    point.y.assign(xEnd, yEnd);

    return point;
}

ByteVector encodeECPoint(CurveID curve, const ECPoint &point)
{
    const std::size_t coordinateSize = getCoordinateSize(curve);

    require(point.x.size() == coordinateSize, "Invalid DUOX EC point X-coordinate length");
    require(point.y.size() == coordinateSize, "Invalid DUOX EC point Y-coordinate length");

    ByteVector encoded;
    encoded.reserve(1U + (2U * coordinateSize));

    encoded.push_back(0x04U);
    encoded.insert(encoded.end(), point.x.begin(), point.x.end());
    encoded.insert(encoded.end(), point.y.begin(), point.y.end());

    return encoded;
}

// ==============================================
// EC point validation
// ==============================================

bool validateECPoint(CurveID curve, const ECPoint &point)
{
    const std::size_t coordinateSize = getCoordinateSize(curve);

    if (point.x.size() != coordinateSize || point.y.size() != coordinateSize)
        return false;

    ECGroupPtr group = createECGroup(curve);
    BNContextPtr bnContext(BN_CTX_new(), &BN_CTX_free);

    requireNotNull(bnContext.get(), "Failed to allocate OpenSSL BN context.");

    ECPointPtr ecPoint(EC_POINT_new(group.get()), &EC_POINT_free);

    requireNotNull(ecPoint.get(), "Failed to allocate OpenSSL EC point.");

    const BIGNUMPtr x = coordinateToBignum(point.x);
    const BIGNUMPtr y = coordinateToBignum(point.y);

    // Import the affine coordinates into the selected curve (OpenSSL requires BIGNUM representations)
    if (EC_POINT_set_affine_coordinates(group.get(), ecPoint.get(), x.get(), y.get(), bnContext.get()) != 1)
        return false;

    // Actual point on curve check required for public-key validation
    if (EC_POINT_is_on_curve(group.get(), ecPoint.get(), bnContext.get()) != 1)
        return false;

    // Point at infinity is not a valid DUOX public point
    if (EC_POINT_is_at_infinity(group.get(), ecPoint.get()) == 1)
        return false;

    return true;
}

// ==============================================
// ECDH shared secret derivation
// ==============================================

ByteVector deriveECDHSharedSecret(CurveID curve, const ByteVector &privateKey, const ECPoint &peerPublicPoint)
{
    const std::size_t coordinateSize = getCoordinateSize(curve);

    require(privateKey.size() == coordinateSize, "Invalid private key size");

    // Validate the peer public point before scalar multiplication
    require(validateECPoint(curve, peerPublicPoint), "Invalid DUOX EC peer public point");

    ECGroupPtr group = createECGroup(curve);
    BNContextPtr bnContext(BN_CTX_new(), &BN_CTX_free);

    requireNotNull(bnContext.get(), "Failed to allocate OpenSSL BN context.");

    // Validate the private scalar against the actual order of the selected curve
    validatePrivateKey(group.get(), privateKey, bnContext.get()); // dA

    // Convert the private scalar into a BIGNUM
    const BIGNUMPtr privateKeyBN = coordinateToBignum(privateKey);

    // Retrieve the ECC domain-parameter cofactor h. NIST ECC CDH computes P = h * dA * QB
    // TODO h always equals 1 with current curves => keep it for later versions if cofactor doesn't equal 1
    BIGNUMPtr cofactor(BN_new(), &BN_free);

    requireNotNull(cofactor.get(), "Failed to allocate OpenSSL EC cofactor.");

    requireOpenSSL(EC_GROUP_get_cofactor(group.get(), cofactor.get(), bnContext.get()) == 1,
                   "Failed to retrieve OpenSSL EC group cofactor");

    // Compute scalar = h * dA
    // EC_POINT_mul() performs scalar multiplication modulo the group order as appropriate for the selected group
    BIGNUMPtr scalar(BN_new(), &BN_free);

    requireNotNull(scalar.get(), "Failed to allocate OpenSSL ECC CDH scalar.");

    requireOpenSSL(BN_mul(scalar.get(), cofactor.get(), privateKeyBN.get(), bnContext.get()) == 1,
        "Failed to compute ECC CDH scalar");

    // Reconstruct the peer public point in OpenSSL representation
    ECPointPtr peerPoint = createECPoint(group.get(), peerPublicPoint, bnContext.get()); // QB

    // Allocate the resulting shared point
    ECPointPtr sharedPoint(EC_POINT_new(group.get()), &EC_POINT_free);

    requireNotNull(sharedPoint.get(), "Failed to allocate OpenSSL ECDH shared point.");

    // ECDH scalar multiplication, ECC CDH : P = h * dA * QB
    requireOpenSSL(EC_POINT_mul(group.get(), sharedPoint.get(), nullptr, peerPoint.get(),
                                scalar.get(), bnContext.get()) == 1,
                   "Failed to perform OpenSSL ECDH scalar multiplication");

    // ECDH must never produce the point at infinity
    require(EC_POINT_is_at_infinity(group.get(), sharedPoint.get()) != 1, "ECDH resulted in the point at infinity");

    // Extract the affine X-coordinate of the shared point (uses fixed-width 32-byte coordinates) z = xP
    BIGNUMPtr sharedX(BN_new(), &BN_free);

    requireNotNull(sharedX.get(), "Failed to allocate OpenSSL ECDH shared X coordinate.");

    requireOpenSSL(EC_POINT_get_affine_coordinates(group.get(), sharedPoint.get(),
                                                   sharedX.get(), nullptr,
                                                   bnContext.get()) == 1,
                   "Failed to extract OpenSSL ECC CDH X-coordinate");

    // Convert the field element z = xP to the fixed width byte representation Z (32 bytes big-endian with leading zeroes)
    ByteVector sharedSecret(coordinateSize);

    requireOpenSSL(BN_bn2binpad(sharedX.get(), sharedSecret.data(),
                                static_cast<int>(coordinateSize)) == static_cast<int>(coordinateSize),
                   "Failed to encode DUOX ECDH shared secret");

    return sharedSecret;
}

// ==============================================
// ECDSA signing
// ==============================================

ECDSASignature signECDSA(CurveID curve, const ByteVector &privateKey, const ByteVector &message)
{
    const std::size_t coordinateSize = getCoordinateSize(curve);

    require(coordinateSize == DUOX_P256_COORDINATE_SIZE, "Unexpected DUOX ECDSA coordinate size");

    require(privateKey.size() == coordinateSize, "Invalid private key size");

    EVP_PKEYPtr pkey = createECPrivateKey(curve, privateKey);
    EVP_MD_CTXPtr mdContext(EVP_MD_CTX_new(), &EVP_MD_CTX_free);
    
    requireNotNull(mdContext.get(), "Failed to allocate OpenSSL ECDSA digest context.");

    requireOpenSSL(EVP_DigestSignInit(mdContext.get(), nullptr, EVP_sha256(), nullptr, pkey.get()) == 1,
                   "Failed to initialize DUOX ECDSA SHA-256 signing");

    requireOpenSSL(EVP_DigestSignUpdate(mdContext.get(), message.data(), message.size()) == 1,
                   "Failed to update DUOX ECDSA SHA-256 signing");

    std::size_t signatureSize = 0;

    requireOpenSSL(EVP_DigestSignFinal(mdContext.get(), nullptr, &signatureSize) == 1,
                   "Failed to determine DUOX ECDSA signature size");

    ByteVector derSignature(signatureSize);

    requireOpenSSL(EVP_DigestSignFinal(mdContext.get(), derSignature.data(), &signatureSize) == 1,
                   "Failed to generate DUOX ECDSA signature");

    derSignature.resize(signatureSize);

    return decodeECDSASignatureDER(derSignature);
}

// ==============================================
// ECDSA verification
// ==============================================

bool verifyECDSA(CurveID curve, const PublicKey &publicKey, const ByteVector &message, const ECDSASignature &signature)
{
    validateSignatureComponent(signature.r, "r");
    validateSignatureComponent(signature.s, "s");

    EVP_PKEY *key = publicKey.getPublicKey();

    requireNotNull(key, "DUOX ECDSA public key must not be null.");

    require(keyUsesCurve(key, curve), "DUOX ECDSA public key uses the wrong curve");

    // Extract the encoded public key
    std::size_t publicKeySize = 0;

    requireOpenSSL(EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, nullptr, 0, &publicKeySize) == 1,
                   "Unable to determine DUOX ECDSA public-key size");

    ByteVector encodedPublicKey(publicKeySize);

    requireOpenSSL(EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, encodedPublicKey.data(),
                       encodedPublicKey.size(), &publicKeySize) == 1,
                   "Unable to extract DUOX ECDSA public key");

    encodedPublicKey.resize(publicKeySize);

    // Validate public EC point

    const ECPoint point = decodeECPoint(curve, encodedPublicKey);

    require(validateECPoint(curve, point), "DUOX ECDSA public key is not a valid EC point");

    const ByteVector derSignature = encodeECDSASignatureDER(signature);
    EVP_MD_CTXPtr mdContext(EVP_MD_CTX_new(), &EVP_MD_CTX_free);

    requireNotNull(mdContext.get(), "Failed to allocate OpenSSL ECDSA verification context.");

    requireOpenSSL(EVP_DigestVerifyInit(mdContext.get(), nullptr, EVP_sha256(), nullptr, key) == 1,
        "Failed to initialize DUOX ECDSA SHA-256 verification");

    requireOpenSSL(EVP_DigestVerifyUpdate(mdContext.get(), message.data(), message.size()) == 1,
        "Failed to update DUOX ECDSA SHA-256 verification");

    const int result = EVP_DigestVerifyFinal(mdContext.get(), derSignature.data(), derSignature.size());

    if (result == 1)
        return true;

    if (result == 0)
        return false;

    fail("OpenSSL ECDSA signature verification failed");
}

// ==============================================
// ECDSA DER encoding
// ==============================================

ByteVector encodeECDSASignatureDER(const ECDSASignature &signature)
{
    validateSignatureComponent(signature.r, "r");
    validateSignatureComponent(signature.s, "s");

    ECDSASigPtr ecdsaSignature(ECDSA_SIG_new(), &ECDSA_SIG_free);

    requireNotNull(ecdsaSignature.get(), "Failed to allocate OpenSSL ECDSA signature.");

    BIGNUMPtr r = coordinateToBignum(signature.r);
    BIGNUMPtr s = coordinateToBignum(signature.s);

    // ECDSA_SIG_set0() takes ownership of r and s on success
    requireOpenSSL(ECDSA_SIG_set0(ecdsaSignature.get(), r.get(), s.get()) == 1,
                   "Failed to initialize OpenSSL ECDSA signature");

    r.release();
    s.release();

    const int length = i2d_ECDSA_SIG(ecdsaSignature.get(), nullptr);

    require(length > 0, "Failed to determine DER ECDSA signature length");

    ByteVector encoded(static_cast<std::size_t>(length));

    unsigned char *cursor = encoded.data();
    const int encodedLength = i2d_ECDSA_SIG(ecdsaSignature.get(), &cursor);

    require(encodedLength == length, "Unexpected DER ECDSA signature length");

    return encoded;
}

// ==============================================
// ECDSA DER decoding
// ==============================================

ECDSASignature decodeECDSASignatureDER(const ByteVector &encoded)
{
    require(!encoded.empty(), "DUOX ECDSA DER signature is empty");

    require(encoded.size() <= static_cast<std::size_t>(std::numeric_limits<long>::max()),
            "DUOX ECDSA DER signature is too large");

    const unsigned char *cursor = encoded.data();

    ECDSA_SIG *parsed = d2i_ECDSA_SIG(nullptr, &cursor, static_cast<long>(encoded.size()));

    requireNotNull(parsed, "Failed to parse DER-encoded OpenSSL ECDSA signature.");

    ECDSASigPtr signature(parsed, &ECDSA_SIG_free);
    const unsigned char *end = encoded.data() + encoded.size();

    require(cursor == end, "Trailing data after DER ECDSA signature");

    const BIGNUM *r = nullptr;
    const BIGNUM *s = nullptr;

    ECDSA_SIG_get0(signature.get(), &r, &s);

    requireNotNull(r, "DER ECDSA signature has a null r component.");
    requireNotNull(s, "DER ECDSA signature has a null s component.");

    require(BN_is_negative(r) == 0 && BN_is_negative(s) == 0, "DER ECDSA signature contains a negative value");

    const int rLength = BN_num_bytes(r);
    const int sLength = BN_num_bytes(s);

    require(rLength >= 0 && sLength >= 0, "Invalid DER-encoded ECDSA integer length");

    require(rLength <= static_cast<int>(DUOX_P256_COORDINATE_SIZE) &&
                sLength <= static_cast<int>(DUOX_P256_COORDINATE_SIZE),
            "DUOX ECDSA signature integer exceeds 32 bytes");

    ECDSASignature result;
    result.r.resize(DUOX_P256_COORDINATE_SIZE, 0);
    result.s.resize(DUOX_P256_COORDINATE_SIZE, 0);

    if (rLength != 0)
    {
        requireOpenSSL(BN_bn2binpad(r, result.r.data(),
                       static_cast<int>(DUOX_P256_COORDINATE_SIZE)) == static_cast<int>(DUOX_P256_COORDINATE_SIZE),
                       "Failed to decode ECDSA r value");
    }

    if (sLength != 0)
    {
        requireOpenSSL(BN_bn2binpad(s, result.s.data(),
                       static_cast<int>(DUOX_P256_COORDINATE_SIZE)) == static_cast<int>(DUOX_P256_COORDINATE_SIZE),
                       "Failed to decode ECDSA s value");
    }

    // ECDSA r and s are required to be non zero
    const bool rIsZero = std::all_of(result.r.begin(), result.r.end(), [](std::uint8_t byte) { return byte == 0; });
    const bool sIsZero = std::all_of(result.s.begin(), result.s.end(), [](std::uint8_t byte) { return byte == 0; });

    require(!rIsZero && !sIsZero, "DUOX ECDSA signature contains zero r or s");

    return result;
}

SessionKeys deriveSessionKeys(const ECPoint &ephemeralPublicA, const ECPoint &ephemeralPublicB,
                                      const ByteVector &sharedSecret)
{
    constexpr std::size_t DUOX_COORDINATE_SIZE    = 32U;
    constexpr std::size_t DUOX_SHARED_SECRET_SIZE = 32U;
    constexpr std::size_t DUOX_KDF_SALT_SIZE      = 16U;
    constexpr std::size_t DUOX_SESSION_KEY_SIZE   = 16U;

    // DUOX uses 256-bit affine coordinates
    require(ephemeralPublicA.x.size() == DUOX_COORDINATE_SIZE && ephemeralPublicA.y.size() == DUOX_COORDINATE_SIZE,
            "DUOX ephemeral public key A must contain 32-byte coordinates");

    require(ephemeralPublicB.x.size() == DUOX_COORDINATE_SIZE && ephemeralPublicB.y.size() == DUOX_COORDINATE_SIZE,
            "DUOX ephemeral public key B must contain 32-byte coordinates");

    require(sharedSecret.size() == DUOX_SHARED_SECRET_SIZE, "DUOX shared secret must contain exactly 32 bytes");

    // Step 1 : Derive the Key Derivation Key
    ByteVector salt;
    salt.reserve(DUOX_KDF_SALT_SIZE);

    salt.insert(salt.end(), ephemeralPublicA.x.end() - 8, ephemeralPublicA.x.end());
    salt.insert(salt.end(), ephemeralPublicB.x.end() - 8, ephemeralPublicB.x.end());

    require(salt.size() == DUOX_KDF_SALT_SIZE, "DUOX KDF salt must contain exactly 16 bytes");

    const ByteVector kdk = openssl::CMACCrypto::cmac(salt, "aes", sharedSecret);

    require(kdk.size() == DUOX_SESSION_KEY_SIZE, "DUOX KDK must contain exactly 16 bytes");

    /*
     * Step 2 : Build the common counter mode input
     *
     * Counter = 1, encoded as a 16-bit big-endian integer
     * Output length = 128 bits, encoded as a 16-bit big-endian integer
     * Context = 10 zero bytes
     */
    ByteVector kdfInput;
    kdfInput.reserve(14U);

    kdfInput.push_back(0x00);
    kdfInput.push_back(0x01);
    kdfInput.push_back(0x00);
    kdfInput.push_back(0x80);
    kdfInput.insert(kdfInput.end(), 10U, 0x00);

    require(kdfInput.size() == 14U, "Invalid DUOX KDF input size");

    // Step 3 : Derive the encryption session key
    ByteVector encInput;
    encInput.reserve(2U + kdfInput.size());

    encInput.push_back(0xB4);
    encInput.push_back(0x4B);
    encInput.insert(encInput.end(), kdfInput.begin(), kdfInput.end());

    const ByteVector encKey = openssl::CMACCrypto::cmac(kdk, "aes", encInput);

    require(encKey.size() == DUOX_SESSION_KEY_SIZE, "DUOX encryption session key must contain exactly 16 bytes");

    // Step 4 : Derive the MAC session key
    ByteVector macInput;
    macInput.reserve(2U + kdfInput.size());

    macInput.push_back(0x4B);
    macInput.push_back(0xB4);
    macInput.insert(macInput.end(), kdfInput.begin(), kdfInput.end());

    const ByteVector macKey = openssl::CMACCrypto::cmac(kdk, "aes", macInput);

    require(macKey.size() == DUOX_SESSION_KEY_SIZE, "DUOX MAC session key must contain exactly 16 bytes");

    return {encKey, macKey};
}

ECKeyPair generateECKeyPair(CurveID curve)
{
    const std::size_t coordinateSize = getCoordinateSize(curve);
    const char *curveName            = getOpenSSLCurveName(curve);

    EVP_PKEY_CTXPtr keyContext(EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr), &EVP_PKEY_CTX_free);

    requireNotNull(keyContext.get(), "Failed to allocate OpenSSL EC key generation context.");

    requireOpenSSL(EVP_PKEY_keygen_init(keyContext.get()) > 0, "Unable to initialize EC key generation");

    requireOpenSSL(EVP_PKEY_CTX_set_group_name(keyContext.get(), curveName) > 0, "Unable to configure EC curve");

    EVP_PKEY *rawKey = nullptr;

    const int generationResult = EVP_PKEY_generate(keyContext.get(), &rawKey);

    EVP_PKEYPtr key(rawKey, &EVP_PKEY_free);

    requireOpenSSL(generationResult > 0, "Failed to generate the OpenSSL EC key pair.");

    // Extract the private scalar as a BIGNUM
    BIGNUM *privateKeyRaw = nullptr;

    requireOpenSSL(EVP_PKEY_get_bn_param(key.get(), OSSL_PKEY_PARAM_PRIV_KEY, &privateKeyRaw) == 1,
        "Unable to extract EC private key");

    BIGNUMPtr privateKey(privateKeyRaw, &BN_free);

    ECKeyPair result;
    result.privateKey.resize(coordinateSize);

    const int written = BN_bn2binpad(privateKey.get(), result.privateKey.data(), static_cast<int>(coordinateSize));

    require(written == static_cast<int>(coordinateSize), "Unable to encode EC private key");

    // Extract the public key in uncompressed form : 0x04 || X || Y
    std::size_t publicKeySize = 0;

    requireOpenSSL(EVP_PKEY_get_octet_string_param(key.get(), OSSL_PKEY_PARAM_PUB_KEY, nullptr, 0, &publicKeySize) == 1,
                   "Unable to determine EC public key size");

    const std::size_t expectedPublicKeySize = 1U + (2U * coordinateSize);

    require(publicKeySize == expectedPublicKeySize, "Unexpected EC public key size");

    ByteVector encodedPublicKey(publicKeySize);

    requireOpenSSL(EVP_PKEY_get_octet_string_param(
                       key.get(), OSSL_PKEY_PARAM_PUB_KEY, encodedPublicKey.data(),
                       encodedPublicKey.size(), &publicKeySize) == 1,
                   "Unable to extract EC public key");

    require(publicKeySize == encodedPublicKey.size(), "Unexpected EC public key size");

    result.publicKey = decodeECPoint(curve, encodedPublicKey);

    return result;
}

} // namespace logicalaccess
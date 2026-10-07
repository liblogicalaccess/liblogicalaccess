#include "duoxecc.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/param_build.h>

#include <logicalaccess/plugins/crypto/cmac.hpp>

//TODO later in final version : replace std::runtime_error by EXCEPTION_ASSERT_WITH_LOG and THROW_EXCEPTION_WITH_LOG

namespace logicalaccess
{

namespace
{
// Number of bytes used by one affine coordinate for DUOX P-256 curves
constexpr std::size_t DUOX_P256_COORDINATE_SIZE = 32U;

// Give every resource deterministic cleanup, including when an exception is thrown
using ECGroupPtr = std::unique_ptr<EC_GROUP, decltype(&EC_GROUP_free)>;
using ECPointPtr = std::unique_ptr<EC_POINT, decltype(&EC_POINT_free)>;
using BNContextPtr = std::unique_ptr<BN_CTX, decltype(&BN_CTX_free)>;
using BIGNUMPtr = std::unique_ptr<BIGNUM, decltype(&BN_free)>;

using EVP_PKEYPtr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using EVP_MD_CTXPtr = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;
using EVP_PKEY_CTXPtr = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
using ECDSASigPtr = std::unique_ptr<ECDSA_SIG, decltype(&ECDSA_SIG_free)>;

// \brief Return the OpenSSL NID corresponding to a DUOX curve
int getOpenSSLCurveNID(CurveID curve)
{
    switch (curve)
    {
    case CurveID::NIST_P256: return NID_X9_62_prime256v1;

    case CurveID::BRAINPOOL_P256R1: return NID_brainpoolP256r1;

    default: throw std::invalid_argument("Unsupported DUOX elliptic curve");
    }
}

// \brief Return the OpenSSL curve name corresponding to a DUOX curve
const char *getOpenSSLCurveName(CurveID curve)
{
    switch (curve)
    {
    case CurveID::NIST_P256: return "prime256v1";

    case CurveID::BRAINPOOL_P256R1: return "brainpoolP256r1";

    default: throw std::invalid_argument("Unsupported DUOX elliptic curve");
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

    if (!group)
        throw std::runtime_error("Failed to create OpenSSL EC group for DUOX curve");

    return group;
}

// \brief Check whether a ByteVector can safely be passed to OpenSSL functions that accept an int length
bool isValidBignumInput(const ByteVector &value)
{
    return value.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max());
}

// \brief Convert a big-endian byte vector into an OpenSSL BIGNUM
BIGNUMPtr coordinateToBignum(const ByteVector &coordinate)
{
    if (!isValidBignumInput(coordinate))
        throw std::invalid_argument("DUOX EC coordinate is too large for OpenSSL BIGNUM conversion");

    BIGNUMPtr value(BN_bin2bn(coordinate.data(), static_cast<int>(coordinate.size()), nullptr), &BN_free);

    if (!value)
        throw std::runtime_error("Failed to convert DUOX EC coordinate to OpenSSL BIGNUM");

    return value;
}

/**
 * \brief Create an OpenSSL EC public key from a DUOX affine point
 *
 * The point is validated before being imported
 */
ECPointPtr createECPoint(const EC_GROUP *group, const ECPoint &point, BN_CTX *bn_ctx)
{
    if (group == nullptr || bn_ctx == nullptr)
        throw std::invalid_argument("Invalid OpenSSL EC parameters");

    ECPointPtr ecPoint(EC_POINT_new(group), &EC_POINT_free);

    if (!ecPoint)
        throw std::runtime_error("Failed to allocate OpenSSL EC point");

    const BIGNUMPtr x = coordinateToBignum(point.x);
    const BIGNUMPtr y = coordinateToBignum(point.y);

    if (EC_POINT_set_affine_coordinates(group, ecPoint.get(), x.get(), y.get(), bn_ctx) != 1)
        throw std::invalid_argument("Invalid DUOX EC public point");

    return ecPoint;
}

/**
 * \brief Validate a DUOX private scalar
 *
 * A private scalar must satisfy : 1 <= privateKey < curve order
 */
void validatePrivateKey(const EC_GROUP *group, const ByteVector &privateKey, BN_CTX *bnContext)
{
    if (group == nullptr || bnContext == nullptr)
        throw std::invalid_argument("Invalid OpenSSL EC parameters");

    if (privateKey.empty())
        throw std::invalid_argument("DUOX EC private key is empty");

    if (!isValidBignumInput(privateKey))
        throw std::invalid_argument("DUOX EC private key is too large for OpenSSL BIGNUM conversion");

    const BIGNUMPtr privateKeyBN = coordinateToBignum(privateKey);
    BIGNUMPtr order(BN_new(), &BN_free);

    if (!order)
        throw std::runtime_error("Failed to allocate OpenSSL EC order BIGNUM");

    if (EC_GROUP_get_order(group, order.get(), bnContext) != 1)
        throw std::runtime_error("Failed to retrieve OpenSSL EC group order");

    if (BN_is_zero(privateKeyBN.get()) || BN_is_negative(privateKeyBN.get()))
        throw std::invalid_argument("DUOX EC private key must be greater than zero");

    if (BN_cmp(privateKeyBN.get(), order.get()) >= 0)
        throw std::invalid_argument("DUOX EC private key is outside the valid scalar range");
}

// \brief Create an OpenSSL EVP private key from a DUOX private scalar
EVP_PKEYPtr createECPrivateKey(CurveID curve, const ByteVector &privateKey)
{
    if (privateKey.empty())
        throw std::invalid_argument("DUOX EC private key is empty");

    if (!isValidBignumInput(privateKey))
        throw std::invalid_argument("DUOX EC private key is too large");

    const int nid = getOpenSSLCurveNID(curve);
    ECGroupPtr group = createECGroup(curve);
    BNContextPtr bnContext(BN_CTX_new(), &BN_CTX_free);

    if (!bnContext)
        throw std::runtime_error("Failed to allocate OpenSSL BN context");

    validatePrivateKey(group.get(), privateKey, bnContext.get());

    const BIGNUMPtr privateKeyBN = coordinateToBignum(privateKey);

    // Derive the public point
    ECPointPtr publicPoint(EC_POINT_new(group.get()), &EC_POINT_free);

    if (!publicPoint)
        throw std::runtime_error("Failed to allocate OpenSSL EC public point");

    if (EC_POINT_mul(group.get(), publicPoint.get(), privateKeyBN.get(), nullptr, nullptr, bnContext.get()) != 1)
        throw std::runtime_error("Failed to derive DUOX EC public key");

    // Encode the public point
    const std::size_t coordinateSize = getCoordinateSize(curve);

    BIGNUMPtr publicX(BN_new(), &BN_free);
    BIGNUMPtr publicY(BN_new(), &BN_free);

    if (!publicX || !publicY)
        throw std::runtime_error("Failed to allocate OpenSSL EC public coordinates");

    if (EC_POINT_get_affine_coordinates(group.get(), publicPoint.get(), publicX.get(), publicY.get(),
        bnContext.get()) != 1)
    {
        throw std::runtime_error("Failed to extract OpenSSL EC public coordinates");
    }

    ByteVector encodedPublicKey(1U + (2U * coordinateSize));

    encodedPublicKey[0] = 0x04U;

    if (BN_bn2binpad(publicX.get(), encodedPublicKey.data() + 1U,
                     static_cast<int>(coordinateSize)) != static_cast<int>(coordinateSize))
    {
        throw std::runtime_error("Failed to encode DUOX EC public X coordinate");
    }

    if (BN_bn2binpad(publicY.get(), encodedPublicKey.data() + 1U + coordinateSize,
                     static_cast<int>(coordinateSize)) != static_cast<int>(coordinateSize))
    {
        throw std::runtime_error("Failed to encode DUOX EC public Y coordinate");
    }

    EVP_PKEY_CTXPtr keyContext(EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr), &EVP_PKEY_CTX_free);

    if (!keyContext)
        throw std::runtime_error("Failed to allocate OpenSSL EC key context");

    if (EVP_PKEY_fromdata_init(keyContext.get()) <= 0)
        throw std::runtime_error("Failed to initialize OpenSSL EC key import");

    const char *curveName = OBJ_nid2sn(nid);

    if (curveName == nullptr)
        throw std::runtime_error("Failed to determine OpenSSL DUOX curve name");

    OSSL_PARAM_BLD *paramBuilder = OSSL_PARAM_BLD_new();

    if (paramBuilder == nullptr)
        throw std::runtime_error("Failed to allocate OpenSSL EC parameter builder");

    struct ParamBuilderGuard
    {
        OSSL_PARAM_BLD *builder;

        ~ParamBuilderGuard()
        {
            if (builder != nullptr)
                OSSL_PARAM_BLD_free(builder);
        }
    } paramBuilderGuard{paramBuilder};

    if (OSSL_PARAM_BLD_push_utf8_string(paramBuilder, OSSL_PKEY_PARAM_GROUP_NAME, curveName, 0) != 1)
        throw std::runtime_error("Failed to configure OpenSSL DUOX EC curve");

    if (OSSL_PARAM_BLD_push_BN(paramBuilder, OSSL_PKEY_PARAM_PRIV_KEY, privateKeyBN.get()) != 1)
        throw std::runtime_error("Failed to configure OpenSSL DUOX EC private key");

    if (OSSL_PARAM_BLD_push_octet_string(paramBuilder, OSSL_PKEY_PARAM_PUB_KEY,
        encodedPublicKey.data(), encodedPublicKey.size()) != 1)
    {
        throw std::runtime_error("Failed to configure OpenSSL DUOX EC public key");
    }

    OSSL_PARAM *params = OSSL_PARAM_BLD_to_param(paramBuilder);

    if (params == nullptr)
        throw std::runtime_error("Failed to build OpenSSL DUOX EC parameters");

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

    if (EVP_PKEY_fromdata(keyContext.get(), &rawKey, EVP_PKEY_KEYPAIR, params) <= 0)
        throw std::runtime_error("Failed to construct OpenSSL DUOX EC private key");

    EVP_PKEYPtr pkey(rawKey, &EVP_PKEY_free);

    if (!pkey)
        throw std::runtime_error("OpenSSL returned a null DUOX EC private key");

    return pkey;
}

// \brief Validate one fixed-width ECDSA signature component
void validateSignatureComponent(const ByteVector &value, const char *name)
{
    if (value.size() != DUOX_P256_COORDINATE_SIZE)
        throw std::invalid_argument(std::string("DUOX ECDSA ") + name + " must contain exactly 32 bytes");
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

std::size_t getCoordinateSize(CurveID curve)
{
    switch (curve)
    {
    case CurveID::NIST_P256:
    case CurveID::BRAINPOOL_P256R1: return DUOX_P256_COORDINATE_SIZE;

    default: throw std::invalid_argument("Unsupported DUOX elliptic curve");
    }
}

ECPoint decodeECPoint(CurveID curve, const ByteVector &encoded)
{
    const std::size_t coordinateSize = getCoordinateSize(curve);
    const std::size_t expectedSize   = 1U + (2U * coordinateSize);

    if (encoded.size() != expectedSize)
        throw std::invalid_argument("Invalid DUOX EC point length.");

    if (encoded.front() != 0x04U)
        throw std::invalid_argument("DUOX EC point is not an uncompressed point.");

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

    if (point.x.size() != coordinateSize)
        throw std::invalid_argument("Invalid DUOX EC point X-coordinate length");

    if (point.y.size() != coordinateSize)
        throw std::invalid_argument("Invalid DUOX EC point Y-coordinate length");

    ByteVector encoded;
    encoded.reserve(1U + (2U * coordinateSize));

    encoded.push_back(0x04U);
    encoded.insert(encoded.end(), point.x.begin(), point.x.end());
    encoded.insert(encoded.end(), point.y.begin(), point.y.end());

    return encoded;
}

bool validateECPoint(CurveID curve, const ECPoint &point)
{
    const std::size_t coordinateSize = getCoordinateSize(curve);

    if (point.x.size() != coordinateSize || point.y.size() != coordinateSize)
        return false;

    ECGroupPtr group = createECGroup(curve);
    BNContextPtr bnContext(BN_CTX_new(), &BN_CTX_free);

    if (!bnContext)
        throw std::runtime_error("Failed to allocate OpenSSL BN context");

    ECPointPtr ecPoint(EC_POINT_new(group.get()), &EC_POINT_free);

    if (!ecPoint)
        throw std::runtime_error("Failed to allocate OpenSSL EC point");

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

ByteVector deriveECDHSharedSecret(CurveID curve, const ByteVector &privateKey, const ECPoint &peerPublicPoint)
{
    const std::size_t coordinateSize = getCoordinateSize(curve);

    if (privateKey.size() != coordinateSize)
        throw std::invalid_argument("Invalid private key size");

    /*
     * Validate the peer public point before performing any scalar multiplication :
     * Malformed or invalid peer points must never reach the ECDH operation
     */
    if (!validateECPoint(curve, peerPublicPoint))
        throw std::invalid_argument("Invalid DUOX EC peer public point");

    ECGroupPtr group = createECGroup(curve);
    BNContextPtr bnContext(BN_CTX_new(), &BN_CTX_free);

    if (!bnContext)
        throw std::runtime_error("Failed to allocate OpenSSL BN context");

    // Validate the private scalar against the actual order of the selected curve
    validatePrivateKey(group.get(), privateKey, bnContext.get()); // dA

    // Convert the private scalar into a BIGNUM
    const BIGNUMPtr privateKeyBN = coordinateToBignum(privateKey);

    // Retrieve the ECC domain-parameter cofactor h. NIST ECC CDH computes P = h * dA * QB
    // TODO h always equals 1 with current curves => keep it for later versions if cofactor doesn't equal 1
    BIGNUMPtr cofactor(BN_new(), &BN_free);

    if (!cofactor)
        throw std::runtime_error("Failed to allocate OpenSSL EC cofactor");

    if (EC_GROUP_get_cofactor(group.get(), cofactor.get(), bnContext.get()) != 1)
        throw std::runtime_error("Failed to retrieve OpenSSL EC group cofactor");

    // Compute scalar = h * dA
    // EC_POINT_mul() performs scalar multiplication modulo the group order as appropriate for the selected group
    BIGNUMPtr scalar(BN_new(), &BN_free);

    if (!scalar)
        throw std::runtime_error("Failed to allocate OpenSSL ECC CDH scalar");

    if (BN_mul(scalar.get(), cofactor.get(), privateKeyBN.get(), bnContext.get()) != 1)
        throw std::runtime_error("Failed to compute ECC CDH scalar");

    // Reconstruct the peer public point in OpenSSL representation
    ECPointPtr peerPoint = createECPoint(group.get(), peerPublicPoint, bnContext.get()); // QB

    // Allocate the resulting shared point
    ECPointPtr sharedPoint(EC_POINT_new(group.get()), &EC_POINT_free);

    if (!sharedPoint)
        throw std::runtime_error("Failed to allocate OpenSSL ECDH shared point");

    // ECDH scalar multiplication, ECC CDH : P = h * dA * QB
    if (EC_POINT_mul(group.get(), sharedPoint.get(), nullptr, peerPoint.get(), scalar.get(), bnContext.get()) != 1)
        throw std::runtime_error("OpenSSL ECDH scalar multiplication failed");

    // ECDH must never produce the point at infinity
    if (EC_POINT_is_at_infinity(group.get(), sharedPoint.get()) == 1)
        throw std::runtime_error("ECDH resulted in the point at infinity");

    // Extract the affine X-coordinate of the shared point (uses fixed-width 32-byte coordinates) z = xP
    BIGNUMPtr sharedX(BN_new(), &BN_free);

    if (!sharedX)
        throw std::runtime_error("Failed to allocate OpenSSL ECDH shared coordinate");

    if (EC_POINT_get_affine_coordinates(group.get(), sharedPoint.get(), sharedX.get(), nullptr, bnContext.get()) != 1)
        throw std::runtime_error("Failed to extract OpenSSL ECC CDH X-coordinate");

    // Convert the field element z = xP to the fixed width byte representation Z (32 bytes big-endian with leading zeroes)
    ByteVector sharedSecret(coordinateSize);

    if (BN_bn2binpad(sharedX.get(), sharedSecret.data(),
                     static_cast<int>(coordinateSize)) != static_cast<int>(coordinateSize))
    {
        throw std::runtime_error("Failed to encode DUOX ECDH shared secret");
    }

    return sharedSecret;
}

ECDSASignature signECDSA(CurveID curve, const ByteVector &privateKey, const ByteVector &message)
{
    const std::size_t coordinateSize = getCoordinateSize(curve);

    if (coordinateSize != 32U)
        throw std::runtime_error("Unexpected DUOX ECDSA coordinate size");

    if (privateKey.size() != coordinateSize)
        throw std::invalid_argument("Invalid private key size");

    EVP_PKEYPtr pkey = createECPrivateKey(curve, privateKey);
    EVP_MD_CTXPtr mdContext(EVP_MD_CTX_new(), &EVP_MD_CTX_free);

    if (!mdContext)
        throw std::runtime_error("Failed to allocate OpenSSL ECDSA digest context");

    if (EVP_DigestSignInit(mdContext.get(), nullptr, EVP_sha256(), nullptr, pkey.get()) != 1)
        throw std::runtime_error("Failed to initialize DUOX ECDSA SHA-256 signing");

    if (EVP_DigestSignUpdate(mdContext.get(), message.data(), message.size()) != 1)
        throw std::runtime_error("Failed to update DUOX ECDSA SHA-256 signing");

    std::size_t signatureSize = 0;

    if (EVP_DigestSignFinal(mdContext.get(), nullptr, &signatureSize) != 1)
        throw std::runtime_error("Failed to determine DUOX ECDSA signature size");

    ByteVector derSignature(signatureSize);

    if (EVP_DigestSignFinal(mdContext.get(), derSignature.data(), &signatureSize) != 1)
        throw std::runtime_error("Failed to generate DUOX ECDSA signature");

    derSignature.resize(signatureSize);

    return decodeECDSASignatureDER(derSignature);
}

bool verifyECDSA(CurveID curve, const PublicKey &publicKey, const ByteVector &message, const ECDSASignature &signature)
{
    validateSignatureComponent(signature.r, "r");
    validateSignatureComponent(signature.s, "s");

    EVP_PKEY *key = publicKey.getPublicKey();

    if (key == nullptr)
        throw std::invalid_argument("DUOX ECDSA public key is invalid");

    if (!keyUsesCurve(key, curve))
        throw std::invalid_argument("DUOX ECDSA public key uses the wrong curve");

    // Validate that the supplied EC public key is valid
    std::size_t publicKeySize = 0;

    if (EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, nullptr, 0, &publicKeySize) != 1)
        throw std::runtime_error("Unable to determine DUOX ECDSA public-key size");

    ByteVector encodedPublicKey(publicKeySize);

    if (EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, encodedPublicKey.data(), encodedPublicKey.size(),
                                        &publicKeySize) != 1)
    {
        throw std::runtime_error("Unable to extract DUOX ECDSA public key");
    }

    encodedPublicKey.resize(publicKeySize);

    const ECPoint point = decodeECPoint(curve, encodedPublicKey);

    if (!validateECPoint(curve, point))
        throw std::invalid_argument("DUOX ECDSA public key is not a valid EC point");

    const ByteVector derSignature = encodeECDSASignatureDER(signature);
    EVP_MD_CTXPtr mdCtx(EVP_MD_CTX_new(), &EVP_MD_CTX_free);

    if (!mdCtx)
        throw std::runtime_error("Failed to allocate OpenSSL ECDSA verification context");

    if (EVP_DigestVerifyInit(mdCtx.get(), nullptr, EVP_sha256(), nullptr, key) != 1)
        throw std::runtime_error("Failed to initialize DUOX ECDSA SHA-256 verification");

    if (EVP_DigestVerifyUpdate(mdCtx.get(), message.data(), message.size()) != 1)
        throw std::runtime_error("Failed to update DUOX ECDSA SHA-256 verification");

    const int result = EVP_DigestVerifyFinal(mdCtx.get(), derSignature.data(), derSignature.size());

    if (result == 1)
        return true;

    if (result == 0)
        return false;

    throw std::runtime_error("OpenSSL ECDSA signature verification failed");
}

ByteVector encodeECDSASignatureDER(const ECDSASignature &signature)
{
    validateSignatureComponent(signature.r, "r");
    validateSignatureComponent(signature.s, "s");

    ECDSASigPtr ecdsaSignature(ECDSA_SIG_new(), &ECDSA_SIG_free);

    if (!ecdsaSignature)
        throw std::runtime_error("Failed to allocate OpenSSL ECDSA signature");

    BIGNUMPtr r = coordinateToBignum(signature.r);
    BIGNUMPtr s = coordinateToBignum(signature.s);

    // ECDSA_SIG_set0() takes ownership of r and s on success
    if (ECDSA_SIG_set0(ecdsaSignature.get(), r.get(), s.get()) != 1)
        throw std::runtime_error("Failed to initialize OpenSSL ECDSA signature");

    r.release();
    s.release();

    const int length = i2d_ECDSA_SIG(ecdsaSignature.get(), nullptr);

    if (length <= 0)
        throw std::runtime_error("Failed to determine DER ECDSA signature length");

    ByteVector encoded(static_cast<std::size_t>(length));

    unsigned char *cursor = encoded.data();
    const int encodedLength = i2d_ECDSA_SIG(ecdsaSignature.get(), &cursor);

    if (encodedLength != length)
        throw std::runtime_error("Unexpected DER ECDSA signature length");

    return encoded;
}

ECDSASignature decodeECDSASignatureDER(const ByteVector &encoded)
{
    if (encoded.empty())
        throw std::invalid_argument("DUOX ECDSA DER signature is empty");

    if (encoded.size() > static_cast<std::size_t>(std::numeric_limits<long>::max()))
        throw std::invalid_argument("DUOX ECDSA DER signature is too large");

    const unsigned char *cursor = encoded.data();

    ECDSA_SIG *parsed = d2i_ECDSA_SIG(nullptr, &cursor, static_cast<long>(encoded.size()));

    if (parsed == nullptr)
        throw std::invalid_argument("Invalid DER ECDSA signature");

    ECDSASigPtr signature(parsed, &ECDSA_SIG_free);
    const unsigned char *end = encoded.data() + encoded.size();

    if (cursor != end)
        throw std::invalid_argument("Trailing data after DER ECDSA signature");

    const BIGNUM *r = nullptr;
    const BIGNUM *s = nullptr;

    ECDSA_SIG_get0(signature.get(), &r, &s);

    if (r == nullptr || s == nullptr)
        throw std::invalid_argument("DER ECDSA signature has missing r or s");

    if (BN_is_negative(r) != 0 || BN_is_negative(s) != 0)
        throw std::invalid_argument("DER ECDSA signature contains a negative value");

    const int rLength = BN_num_bytes(r);
    const int sLength = BN_num_bytes(s);

    if (rLength < 0 || sLength < 0)
        throw std::runtime_error("Invalid ECDSA signature integer length");

    if (rLength > 32 || sLength > 32)
        throw std::invalid_argument("DUOX ECDSA signature integer exceeds 32 bytes");

    ECDSASignature result;
    result.r.resize(DUOX_P256_COORDINATE_SIZE, 0);
    result.s.resize(DUOX_P256_COORDINATE_SIZE, 0);

    if (rLength != 0)
    {
        if (BN_bn2binpad(r, result.r.data(), 32) != 32)
            throw std::runtime_error("Failed to decode ECDSA r value");
    }

    if (sLength != 0)
    {
        if (BN_bn2binpad(s, result.s.data(), 32) != 32)
            throw std::runtime_error("Failed to decode ECDSA s value");
    }

    // ECDSA r and s are required to be non zero
    const bool rIsZero = std::all_of(result.r.begin(), result.r.end(), [](std::uint8_t byte) { return byte == 0; });
    const bool sIsZero = std::all_of(result.s.begin(), result.s.end(), [](std::uint8_t byte) { return byte == 0; });

    if (rIsZero || sIsZero)
        throw std::invalid_argument("DUOX ECDSA signature contains zero r or s");

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
    if (ephemeralPublicA.x.size() != DUOX_COORDINATE_SIZE || ephemeralPublicA.y.size() != DUOX_COORDINATE_SIZE)
        throw std::invalid_argument("DUOX ephemeral public key A must contain 32-byte coordinates");

    if (ephemeralPublicB.x.size() != DUOX_COORDINATE_SIZE || ephemeralPublicB.y.size() != DUOX_COORDINATE_SIZE)
        throw std::invalid_argument("DUOX ephemeral public key B must contain 32-byte coordinates");

    if (sharedSecret.size() != DUOX_SHARED_SECRET_SIZE)
        throw std::invalid_argument("DUOX shared secret must contain exactly 32 bytes");

    // Step 1 : Derive the Key Derivation Key
    ByteVector salt;
    salt.reserve(DUOX_KDF_SALT_SIZE);

    salt.insert(salt.end(), ephemeralPublicA.x.end() - 8, ephemeralPublicA.x.end());
    salt.insert(salt.end(), ephemeralPublicB.x.end() - 8, ephemeralPublicB.x.end());

    if (salt.size() != DUOX_KDF_SALT_SIZE)
        throw std::runtime_error("DUOX KDF salt must contain exactly 16 bytes");

    const ByteVector kdk = openssl::CMACCrypto::cmac(salt, "aes", sharedSecret);

    if (kdk.size() != DUOX_SESSION_KEY_SIZE)
        throw std::runtime_error("DUOX KDK must contain exactly 16 bytes");

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

    if (kdfInput.size() != 14U)
        throw std::runtime_error("Invalid DUOX KDF input size");

    // Step 3 : Derive the encryption session key
    ByteVector encInput;
    encInput.reserve(2U + kdfInput.size());

    encInput.push_back(0xB4);
    encInput.push_back(0x4B);
    encInput.insert(encInput.end(), kdfInput.begin(), kdfInput.end());

    const ByteVector encKey = openssl::CMACCrypto::cmac(kdk, "aes", encInput);

    if (encKey.size() != DUOX_SESSION_KEY_SIZE)
        throw std::runtime_error("DUOX encryption session key must contain exactly 16 bytes");

    // Step 4 : Derive the MAC session key
    ByteVector macInput;
    macInput.reserve(2U + kdfInput.size());

    macInput.push_back(0x4B);
    macInput.push_back(0xB4);
    macInput.insert(macInput.end(), kdfInput.begin(), kdfInput.end());

    const ByteVector macKey = openssl::CMACCrypto::cmac(kdk, "aes", macInput);

    if (macKey.size() != DUOX_SESSION_KEY_SIZE)
        throw std::runtime_error("DUOX MAC session key must contain exactly 16 bytes");

    return {encKey, macKey};
}

ECKeyPair generateECKeyPair(CurveID curve)
{
    const std::size_t coordinateSize = getCoordinateSize(curve);
    const char *curveName            = getOpenSSLCurveName(curve);

    EVP_PKEY_CTXPtr keyContext(EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr), &EVP_PKEY_CTX_free);

    if (!keyContext)
        throw std::runtime_error("Unable to create EC key generation context");

    if (EVP_PKEY_keygen_init(keyContext.get()) <= 0)
        throw std::runtime_error("Unable to initialize EC key generation");

    if (EVP_PKEY_CTX_set_group_name(keyContext.get(), curveName) <= 0)
        throw std::runtime_error("Unable to configure EC curve");

    EVP_PKEY *rawKey = nullptr;

    if (EVP_PKEY_generate(keyContext.get(), &rawKey) <= 0)
        throw std::runtime_error("Unable to generate EC key pair");

    EVP_PKEYPtr key(rawKey, &EVP_PKEY_free);

    // Extract the private scalar as a BIGNUM
    BIGNUM *privateKeyRaw = nullptr;

    if (EVP_PKEY_get_bn_param(key.get(), OSSL_PKEY_PARAM_PRIV_KEY, &privateKeyRaw) != 1)
        throw std::runtime_error("Unable to extract EC private key");

    BIGNUMPtr privateKey(privateKeyRaw, &BN_free);

    ECKeyPair result;
    result.privateKey.resize(coordinateSize);

    const int written = BN_bn2binpad(privateKey.get(), result.privateKey.data(), static_cast<int>(coordinateSize));
    if (written != static_cast<int>(coordinateSize))
        throw std::runtime_error("Unable to encode EC private key");

    // Extract the public key in uncompressed form : 0x04 || X || Y
    std::size_t publicKeySize = 0;

    if (EVP_PKEY_get_octet_string_param(key.get(), OSSL_PKEY_PARAM_PUB_KEY, nullptr, 0, &publicKeySize) != 1)
        throw std::runtime_error("Unable to determine EC public key size");

    const std::size_t expectedPublicKeySize = 1U + (2U * coordinateSize);

    if (publicKeySize != expectedPublicKeySize)
        throw std::runtime_error("Unexpected EC public key size");

    ByteVector encodedPublicKey(publicKeySize);

    if (EVP_PKEY_get_octet_string_param(key.get(), OSSL_PKEY_PARAM_PUB_KEY, encodedPublicKey.data(), encodedPublicKey.size(),
        &publicKeySize) != 1)
    {
        throw std::runtime_error("Unable to extract EC public key");
    }

    if (publicKeySize != encodedPublicKey.size())
        throw std::runtime_error("Unexpected EC public key size");

    result.publicKey = decodeECPoint(curve, encodedPublicKey);

    return result;
}

} // namespace logicalaccess
#include <logicalaccess/plugins/cards/duox/duoxecc.hpp>
#include <logicalaccess/plugins/crypto/public_key.hpp>

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <openssl/core_names.h>
#include <openssl/bn.h>
#include <openssl/param_build.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace logicalaccess;

namespace
{

/*
 * ==============================================
 * Test constants
 * ==============================================
 */

constexpr std::size_t DUOX_COORDINATE_SIZE  = 32U;
constexpr std::size_t DUOX_POINT_SIZE       = 65U;
constexpr std::size_t DUOX_SESSION_KEY_SIZE = 16U;

constexpr std::uint8_t UNCOMPRESSED_POINT_PREFIX = 0x04U;

/*
 * ==============================================
 * Test infrastructure
 * ==============================================
 */

class TestFailure : public std::runtime_error
{
  public:
    explicit TestFailure(const std::string &message)
        : std::runtime_error(message)
    {
    }
};

class TestSkipped : public std::runtime_error
{
  public:
    explicit TestSkipped(const std::string &message)
        : std::runtime_error(message)
    {
    }
};

void expect(bool condition, const std::string &message)
{
    if (!condition)
        throw TestFailure(message);
}

template <typename Exception, typename Callable>
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
    stream << std::hex << std::uppercase << std::setfill('0');

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

void printInfo(const std::string &name, bool value)
{
    printInfo(name, std::string(value ? "true" : "false"));
}

/*
 * ==============================================
 * OpenSSL RAII helpers
 * ==============================================
 */

using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using EVP_PKEY_CTX_ptr = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
using BIGNUM_ptr = std::unique_ptr<BIGNUM, decltype(&BN_free)>;

EVP_PKEY_ptr makeEVPKey(EVP_PKEY *key)
{
    return EVP_PKEY_ptr(key, &EVP_PKEY_free);
}

EVP_PKEY_CTX_ptr makeEVPKeyContext(EVP_PKEY_CTX *context)
{
    return EVP_PKEY_CTX_ptr(context, &EVP_PKEY_CTX_free);
}

BIGNUM_ptr makeBIGNUM(BIGNUM *value)
{
    return BIGNUM_ptr(value, &BN_free);
}

/*
 * ==============================================
 * OpenSSL key helpers
 * ==============================================
 */

EVP_PKEY_ptr generateECKey(int curveNID)
{
    EVP_PKEY_CTX_ptr context = makeEVPKeyContext(EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr));

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
        opensslFailure("EVP_PKEY_keygen returned null");

    return makeEVPKey(generated);
}

bool canGenerateCurve(int curveNID)
{
    try
    {
        return static_cast<bool>(generateECKey(curveNID));
    }
    catch (...)
    {
        ERR_clear_error();
        return false;
    }
}

ByteVector getPrivateScalar(EVP_PKEY *key)
{
    if (key == nullptr)
        throw std::invalid_argument("getPrivateScalar : null key");

    BIGNUM *rawPrivateKey = nullptr;

    if (EVP_PKEY_get_bn_param(key, OSSL_PKEY_PARAM_PRIV_KEY, &rawPrivateKey) != 1)
        opensslFailure("EVP_PKEY_get_bn_param(priv-key)");

    BIGNUM_ptr privateKey = makeBIGNUM(rawPrivateKey);
    ByteVector result(DUOX_COORDINATE_SIZE);

    if (BN_bn2binpad(privateKey.get(), result.data(), static_cast<int>(result.size())) != static_cast<int>(result.size()))
        opensslFailure("BN_bn2binpad(private-key)");

    return result;
}

ByteVector getUncompressedPublicPoint(EVP_PKEY *key)
{
    if (key == nullptr)
        throw std::invalid_argument("getUncompressedPublicPoint : null key");

    std::size_t required = 0U;

    if (EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, nullptr, 0, &required) != 1)
        opensslFailure("EVP_PKEY_get_octet_string_param(size)");

    if (required == 0U)
        throw std::runtime_error("EC public point is empty");

    ByteVector result(required);
    std::size_t written = 0;

    if (EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, result.data(), result.size(), &written) != 1)
        opensslFailure("EVP_PKEY_get_octet_string_param");

    result.resize(written);

    return result;
}

ECPoint getDUOXPoint(EVP_PKEY *key, CurveID curve)
{
    return decodeECPoint(curve, getUncompressedPublicPoint(key));
}

PublicKey makePublicKey(EVP_PKEY *key)
{
    if (key == nullptr)
        throw std::invalid_argument("makePublicKey : null key");

    EVP_PKEY *duplicate = EVP_PKEY_dup(key);

    if (duplicate == nullptr)
        opensslFailure("EVP_PKEY_dup");

    return PublicKey(duplicate);
}

/*
 * ==============================================
 * Curve tests
 * ==============================================
 */

void testCoordinateSize()
{
    printSection("DUOX ECC : coordinate sizes.");

    expect(getCoordinateSize(CurveID::NIST_P256) == DUOX_COORDINATE_SIZE,
           "NIST P-256 coordinate size must be 32.");
    expect(getCoordinateSize(CurveID::BRAINPOOL_P256R1) == DUOX_COORDINATE_SIZE,
           "Brainpool P-256r1 coordinate size must be 32.");

    std::cout << " Both DUOX curves use 32-byte coordinates.\n";
}

void testUnsupportedCurve()
{
    printSection("DUOX ECC : unsupported curve.");

    const auto unsupported = static_cast<CurveID>(0xFF);
    expectThrows<std::invalid_argument>([&] { getCoordinateSize(unsupported); }, "Unsupported curve identifier.");

    std::cout << " Unsupported curve correctly rejected.\n";
}

/*
 * ==============================================
 * EC point encoding / decoding
 * ==============================================
 */

void testP256PointEncodeDecode()
{
    printSection("DUOX ECC : P-256 point encode/decode.");

    const EVP_PKEY_ptr key    = generateECKey(NID_X9_62_prime256v1);

    const ByteVector original = getUncompressedPublicPoint(key.get());
    expect(original.size() == DUOX_POINT_SIZE, "P-256 uncompressed point must contain 65 bytes.");
    expect(original[0] == UNCOMPRESSED_POINT_PREFIX, "P-256 point must use uncompressed encoding.");

    const ECPoint point = decodeECPoint(CurveID::NIST_P256, original);
    expect(point.x.size() == DUOX_COORDINATE_SIZE, "P-256 X coordinate must contain 32 bytes.");
    expect(point.y.size() == DUOX_COORDINATE_SIZE, "P-256 Y coordinate must contain 32 bytes.");

    const ByteVector encoded = encodeECPoint(CurveID::NIST_P256, point);
    expect(encoded == original, "P-256 encode/decode round-trip must preserve bytes.");
    expect(validateECPoint(CurveID::NIST_P256, point), "generated P-256 point must validate.");

    std::cout << " P-256 point round-trip succeeded.\n";
}

void testBrainpoolPointEncodeDecode()
{
    printSection("DUOX ECC : Brainpool point encode/decode.");

    if (!canGenerateCurve(NID_brainpoolP256r1))
        throw TestSkipped("OpenSSL runtime cannot generate Brainpool P-256r1");

    const EVP_PKEY_ptr key = generateECKey(NID_brainpoolP256r1);

    const ByteVector original = getUncompressedPublicPoint(key.get());
    expect(original.size() == DUOX_POINT_SIZE, "Brainpool P-256r1 point must contain 65 bytes.");
    expect(original[0] == UNCOMPRESSED_POINT_PREFIX, "Brainpool point must use uncompressed encoding.");

    const ECPoint point = decodeECPoint(CurveID::BRAINPOOL_P256R1, original);
    expect(point.x.size() == DUOX_COORDINATE_SIZE, "Brainpool X coordinate must contain 32 bytes.");
    expect(point.y.size() == DUOX_COORDINATE_SIZE, "Brainpool Y coordinate must contain 32 bytes.");

    const ByteVector encoded = encodeECPoint(CurveID::BRAINPOOL_P256R1, point);
    expect(encoded == original, "Brainpool encode/decode round-trip must preserve bytes");
    expect(validateECPoint(CurveID::BRAINPOOL_P256R1, point), "generated Brainpool point must validate");

    std::cout << " Brainpool P-256r1 point round-trip succeeded.\n";
}

void testMalformedPointEncoding()
{
    printSection("DUOX ECC : malformed point encoding.");

    const CurveID curve = CurveID::NIST_P256;

    expectThrows<std::invalid_argument>([&] { decodeECPoint(curve, ByteVector{}); }, "empty EC point.");
    expectThrows<std::invalid_argument>([&] { decodeECPoint(curve, ByteVector{0x04}); }, "point containing only prefix.");
    expectThrows<std::invalid_argument>([&] { decodeECPoint(curve, ByteVector(64, 0x00)); }, "64-byte point without prefix.");
    expectThrows<std::invalid_argument>([&] { decodeECPoint(curve, ByteVector(66, 0x00)); }, "incorrect point length.");

    ByteVector wrongPrefix(DUOX_POINT_SIZE, 0x00);
    wrongPrefix[0] = 0x02;
    expectThrows<std::invalid_argument>([&] { decodeECPoint(curve, wrongPrefix); }, "compressed point encoding.");

    wrongPrefix[0] = 0x06;
    expectThrows<std::invalid_argument>([&] { decodeECPoint(curve, wrongPrefix); }, "hybrid/unknown point encoding.");

    std::cout << " Malformed point encodings correctly rejected.\n";
}

void testPointWrongCurve()
{
    printSection("DUOX ECC : point wrong-curve rejection.");

    const EVP_PKEY_ptr key = generateECKey(NID_X9_62_prime256v1);

    const ECPoint point = getDUOXPoint(key.get(), CurveID::NIST_P256);
    expect(validateECPoint(CurveID::NIST_P256, point), "P-256 point must validate on P-256.");
    expect(!validateECPoint(CurveID::BRAINPOOL_P256R1, point), "P-256 point must not validate on Brainpool.");

    std::cout << " Wrong-curve point correctly rejected.\n";
}

void testPointInvalidCoordinates()
{
    printSection("DUOX ECC : invalid point coordinates.");

    const EVP_PKEY_ptr key = generateECKey(NID_X9_62_prime256v1);

    const ECPoint valid = getDUOXPoint(key.get(), CurveID::NIST_P256);
    expect(validateECPoint(CurveID::NIST_P256, valid), "generated point must validate.");

    ECPoint invalid = valid;
    invalid.x[0] ^= 0x01;
    expect(!validateECPoint(CurveID::NIST_P256, invalid), "modified point must fail validation.");

    std::cout << " Invalid point coordinates correctly rejected.\n";
}

void testPointInvalidSizes()
{
    printSection("DUOX ECC : invalid coordinate sizes.");

    ECPoint point;
    point.x = ByteVector(31, 0x00);
    point.y = ByteVector(32, 0x00);
    expectThrows<std::invalid_argument>([&] { encodeECPoint(CurveID::NIST_P256, point); }, "31-byte X coordinate.");

    point.x = ByteVector(32, 0x00);
    point.y = ByteVector(31, 0x00);
    expectThrows<std::invalid_argument>([&] { encodeECPoint(CurveID::NIST_P256, point); }, "31-byte Y coordinate.");

    point.x = ByteVector(33, 0x00);
    point.y = ByteVector(32, 0x00);
    expectThrows<std::invalid_argument>([&] { encodeECPoint(CurveID::NIST_P256, point); }, "33-byte X coordinate.");

    std::cout << " Invalid coordinate sizes correctly rejected.\n";
}

/*
 * ==============================================
 * ECDH
 * ==============================================
 */

void testP256ECDH()
{
    printSection("DUOX ECC : P-256 ECDH.");

    const EVP_PKEY_ptr keyA = generateECKey(NID_X9_62_prime256v1);
    const EVP_PKEY_ptr keyB = generateECKey(NID_X9_62_prime256v1);

    const ByteVector privateKeyA = getPrivateScalar(keyA.get());
    const ByteVector privateKeyB = getPrivateScalar(keyB.get());

    const ECPoint publicKeyA = getDUOXPoint(keyA.get(), CurveID::NIST_P256);
    const ECPoint publicKeyB = getDUOXPoint(keyB.get(), CurveID::NIST_P256);

    const ByteVector sharedSecretA = deriveECDHSharedSecret(CurveID::NIST_P256, privateKeyA, publicKeyB);
    const ByteVector sharedSecretB = deriveECDHSharedSecret(CurveID::NIST_P256, privateKeyB, publicKeyA);

    expect(sharedSecretA.size() == DUOX_COORDINATE_SIZE, "P-256 ECDH secret must be 32 bytes.");
    expect(sharedSecretB.size() == DUOX_COORDINATE_SIZE, "P-256 ECDH secret must be 32 bytes.");
    expect(sharedSecretA  == sharedSecretB, "both sides of P-256 ECDH must derive the same secret.");
    expect(!sharedSecretA.empty(), "ECDH secret must not be empty");

    printInfo("Shared secret size", sharedSecretA.size());
    printInfo("Shared secret", toHex(sharedSecretA));

    std::cout << " P-256 ECDH succeeded.\n";
}

void testBrainpoolECDH()
{
    printSection("DUOX ECC : Brainpool P-256r1 ECDH.");

    if (!canGenerateCurve(NID_brainpoolP256r1))
        throw TestSkipped("OpenSSL runtime cannot generate Brainpool P-256r1");

    const EVP_PKEY_ptr keyA = generateECKey(NID_brainpoolP256r1);
    const EVP_PKEY_ptr keyB = generateECKey(NID_brainpoolP256r1);

    const ByteVector privateKeyA = getPrivateScalar(keyA.get());
    const ByteVector privateKeyB = getPrivateScalar(keyB.get());

    const ECPoint publicKeyA = getDUOXPoint(keyA.get(), CurveID::BRAINPOOL_P256R1);
    const ECPoint publicKeyB = getDUOXPoint(keyB.get(), CurveID::BRAINPOOL_P256R1);

    const ByteVector sharedSecretA = deriveECDHSharedSecret(CurveID::BRAINPOOL_P256R1, privateKeyA, publicKeyB);
    const ByteVector sharedSecretB = deriveECDHSharedSecret(CurveID::BRAINPOOL_P256R1, privateKeyB, publicKeyA);

    expect(sharedSecretA.size() == DUOX_COORDINATE_SIZE, "Brainpool ECDH secret must be 32 bytes.");
    expect(sharedSecretB.size() == DUOX_COORDINATE_SIZE, "Brainpool ECDH secret must be 32 bytes.");
    expect(sharedSecretA == sharedSecretB, "both sides of Brainpool ECDH must derive the same secret.");

    std::cout << " Brainpool P-256r1 ECDH succeeded.\n";
}

void testECDHInvalidPrivateKey()
{
    printSection("DUOX ECC : invalid ECDH private keys.");

    const EVP_PKEY_ptr peer = generateECKey(NID_X9_62_prime256v1);
    const ECPoint peerPublic = getDUOXPoint(peer.get(), CurveID::NIST_P256);

    expectThrows<std::invalid_argument>(
        [&] { deriveECDHSharedSecret(CurveID::NIST_P256, ByteVector{}, peerPublic); }, "empty ECDH private key.");
    expectThrows<std::invalid_argument>(
        [&] { deriveECDHSharedSecret(CurveID::NIST_P256, ByteVector(31, 0x01), peerPublic); }, "31-byte ECDH private key.");
    expectThrows<std::invalid_argument>(
        [&] { deriveECDHSharedSecret(CurveID::NIST_P256, ByteVector(33, 0x01), peerPublic); }, "33-byte ECDH private key.");
    expectThrows<std::invalid_argument>(
        [&] { deriveECDHSharedSecret(CurveID::NIST_P256, ByteVector(32, 0x00), peerPublic); }, "zero ECDH private key.");

    std::cout << " Invalid ECDH private keys correctly rejected.\n";
}

void testECDHInvalidPeerPoint()
{
    printSection("DUOX ECC : invalid ECDH peer point.");

    const EVP_PKEY_ptr keyA = generateECKey(NID_X9_62_prime256v1);
    const ByteVector privateKeyA = getPrivateScalar(keyA.get());

    ECPoint invalidPoint;

    invalidPoint.x = ByteVector(DUOX_COORDINATE_SIZE, 0x00);
    invalidPoint.y = ByteVector(DUOX_COORDINATE_SIZE, 0x00);
    expectThrows<std::invalid_argument>(
        [&] { deriveECDHSharedSecret(CurveID::NIST_P256, privateKeyA, invalidPoint); }, "invalid ECDH peer point.");

    std::cout << " Invalid ECDH peer point correctly rejected.\n";
}

void testECDHWrongCurve()
{
    printSection("DUOX ECC : ECDH wrong-curve rejection.");

    const EVP_PKEY_ptr keyA = generateECKey(NID_X9_62_prime256v1);
    const EVP_PKEY_ptr keyB = generateECKey(NID_X9_62_prime256v1);

    const ByteVector privateKeyA = getPrivateScalar(keyA.get());
    const ECPoint publicKeyB = getDUOXPoint(keyB.get(), CurveID::NIST_P256);

    expectThrows<std::invalid_argument>(
        [&] { deriveECDHSharedSecret(CurveID::BRAINPOOL_P256R1, privateKeyA, publicKeyB); },
        "P-256 key used with Brainpool ECDH");

    std::cout << " Wrong-curve ECDH correctly rejected.\n";
}

/*
 * ==============================================
 * DUOX ECC session key derivation
 * ==============================================
 */

void testDUOXSessionKeyDerivation()
{
    printSection("DUOX ECC : session key derivation.");

    const EVP_PKEY_ptr keyA = generateECKey(NID_X9_62_prime256v1);
    const EVP_PKEY_ptr keyB = generateECKey(NID_X9_62_prime256v1);

    const ECPoint publicA = getDUOXPoint(keyA.get(), CurveID::NIST_P256);
    const ECPoint publicB = getDUOXPoint(keyB.get(), CurveID::NIST_P256);

    const ByteVector privateA = getPrivateScalar(keyA.get());

    const ByteVector sharedSecret = deriveECDHSharedSecret(CurveID::NIST_P256, privateA, publicB);

    const SessionKeys keys = deriveSessionKeys(publicA, publicB, sharedSecret);

    expect(keys.encKey.size() == DUOX_SESSION_KEY_SIZE, "DUOX ENC session key must be 16 bytes");
    expect(keys.macKey.size() == DUOX_SESSION_KEY_SIZE, "DUOX MAC session key must be 16 bytes");
    expect(keys.encKey != keys.macKey, "DUOX ENC and MAC session keys must differ");

    std::cout << " DUOX session-key derivation succeeded.\n";
}

void testDUOXSessionKeyBinding()
{
    printSection("DUOX ECC : session key binding.");

    const EVP_PKEY_ptr keyA = generateECKey(NID_X9_62_prime256v1);
    const EVP_PKEY_ptr keyB = generateECKey(NID_X9_62_prime256v1);

    const ECPoint publicA = getDUOXPoint(keyA.get(), CurveID::NIST_P256);
    const ECPoint publicB = getDUOXPoint(keyB.get(), CurveID::NIST_P256);

    const ByteVector privateA = getPrivateScalar(keyA.get());

    const ByteVector sharedSecret = deriveECDHSharedSecret(CurveID::NIST_P256, privateA, publicB);

    const SessionKeys keys1 = deriveSessionKeys(publicA, publicB, sharedSecret);

    ECPoint modifiedPublicB = publicB;
    modifiedPublicB.x.back() ^= 0x01;

    const SessionKeys keys2 = deriveSessionKeys(publicA, modifiedPublicB, sharedSecret);

    expect(keys1.encKey != keys2.encKey, "Changing ephemeral public key must change ENC session key");
    expect(keys1.macKey != keys2.macKey, "Changing ephemeral public key must change MAC session key");

    std::cout << " DUOX session keys correctly depend on ephemeral public keys.\n";
}

void testDUOXSessionKeyDerivationInvalidInputs()
{
    printSection("DUOX ECC : invalid session key derivation inputs.");

    const ECPoint publicA{ByteVector(32, 0x11), ByteVector(32, 0x22)};
    const ECPoint publicB{ByteVector(32, 0x33), ByteVector(32, 0x44)};

    const ByteVector sharedSecret(DUOX_COORDINATE_SIZE, 0x55);

    expectThrows<std::invalid_argument>(
        [&] { deriveSessionKeys(ECPoint{}, publicB, sharedSecret); }, "invalid ephemeral public key A.");
    expectThrows<std::invalid_argument>(
        [&] { deriveSessionKeys(publicA, ECPoint{}, sharedSecret); }, "invalid ephemeral public key B.");
    expectThrows<std::invalid_argument>(
        [&] { deriveSessionKeys(publicA, publicB, ByteVector(31, 0x55)); }, "31-byte shared secret.");
    expectThrows<std::invalid_argument>(
        [&] { deriveSessionKeys(publicA, publicB, ByteVector(33, 0x55)); }, "33-byte shared secret.");

    std::cout << " Invalid session-key derivation inputs correctly rejected.\n";
}

void testDUOXSessionKeyDerivationDeterministic()
{
    printSection("DUOX ECC : session key derivation determinism.");

    // These are intentionally arbitrary fixed byte strings : this test verifies KDF determinism, not EC point validity
    const ECPoint publicA{ByteVector(32, 0x11), ByteVector(32, 0x22)};
    const ECPoint publicB{ByteVector(32, 0x33), ByteVector(32, 0x44)};

    const ByteVector sharedSecret(DUOX_COORDINATE_SIZE, 0x55);

    const SessionKeys keys1 = deriveSessionKeys(publicA, publicB, sharedSecret);
    const SessionKeys keys2 = deriveSessionKeys(publicA, publicB, sharedSecret);

    expect(keys1.encKey == keys2.encKey, "KDF must be deterministic for identical inputs.");
    expect(keys1.macKey == keys2.macKey, "KDF must be deterministic for identical inputs.");

    std::cout << " DUOX session-key derivation is deterministic.\n";
}

void testBrainpoolSessionKeyDerivation()
{
    printSection("DUOX ECC : Brainpool session key derivation.");

    if (!canGenerateCurve(NID_brainpoolP256r1))
        throw TestSkipped("OpenSSL runtime cannot generate Brainpool P-256r1");

    const EVP_PKEY_ptr keyA = generateECKey(NID_brainpoolP256r1);
    const EVP_PKEY_ptr keyB = generateECKey(NID_brainpoolP256r1);

    const ECPoint publicA = getDUOXPoint(keyA.get(), CurveID::BRAINPOOL_P256R1);
    const ECPoint publicB = getDUOXPoint(keyB.get(), CurveID::BRAINPOOL_P256R1);

    const ByteVector privateA = getPrivateScalar(keyA.get());

    const ByteVector sharedSecret = deriveECDHSharedSecret(CurveID::BRAINPOOL_P256R1, privateA, publicB);

    const SessionKeys keys = deriveSessionKeys(publicA, publicB, sharedSecret);

    expect(keys.encKey.size() == DUOX_SESSION_KEY_SIZE, "Brainpool DUOX ENC session key must be 16 bytes.");
    expect(keys.macKey.size() == DUOX_SESSION_KEY_SIZE, "Brainpool DUOX MAC session key must be 16 bytes.");
    expect(keys.encKey != keys.macKey, "Brainpool DUOX ENC and MAC session keys must differ.");

    std::cout << " Brainpool session-key derivation succeeded.\n";
}

/*
 * ==============================================
 * ECDSA
 * ==============================================
 */

void testP256ECDSASignVerify()
{
    printSection("DUOX ECC : P-256 ECDSA sign/verify.");

    const EVP_PKEY_ptr key = generateECKey(NID_X9_62_prime256v1);

    const ByteVector privateKey = getPrivateScalar(key.get());
    PublicKey publicKey = makePublicKey(key.get());

    const ByteVector message{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    const ECDSASignature signature = signECDSA(CurveID::NIST_P256, privateKey, message);

    expect(signature.r.size() == DUOX_COORDINATE_SIZE, "ECDSA r must be exactly 32 bytes.");
    expect(signature.s.size() == DUOX_COORDINATE_SIZE, "ECDSA s must be exactly 32 bytes.");
    expect(verifyECDSA(CurveID::NIST_P256, publicKey, message, signature), "generated P-256 ECDSA signature must verify.");

    printInfo("r", toHex(signature.r));
    printInfo("s", toHex(signature.s));

    std::cout << " P-256 ECDSA sign/verify succeeded.\n";
}

void testBrainpoolECDSASignVerify()
{
    printSection("DUOX ECC : Brainpool P-256r1 ECDSA sign/verify.");

    if (!canGenerateCurve(NID_brainpoolP256r1))
        throw TestSkipped("OpenSSL runtime cannot generate Brainpool P-256r1");

    const EVP_PKEY_ptr key = generateECKey(NID_brainpoolP256r1);

    const ByteVector privateKey = getPrivateScalar(key.get());
    PublicKey publicKey = makePublicKey(key.get());

    const ByteVector message{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};
    const ECDSASignature signature = signECDSA(CurveID::BRAINPOOL_P256R1, privateKey, message);

    expect(signature.r.size() == DUOX_COORDINATE_SIZE,
           "Brainpool ECDSA r must be exactly 32 bytes");
    expect(signature.s.size() == DUOX_COORDINATE_SIZE,
           "Brainpool ECDSA s must be exactly 32 bytes");
    expect(verifyECDSA(CurveID::BRAINPOOL_P256R1, publicKey, message, signature),
           "generated Brainpool ECDSA signature must verify");

    std::cout << " Brainpool P-256r1 ECDSA sign/verify succeeded.\n";
}

void testECDSAWrongMessage()
{
    printSection("DUOX ECC : ECDSA wrong message rejection.");

    const EVP_PKEY_ptr key = generateECKey(NID_X9_62_prime256v1);

    const ByteVector privateKey = getPrivateScalar(key.get());
    PublicKey publicKey = makePublicKey(key.get());

    const ByteVector message{0x01, 0x02, 0x03};
    const ECDSASignature signature = signECDSA(CurveID::NIST_P256, privateKey, message);

    ByteVector modifiedMessage = message;
    modifiedMessage[0] ^= 0x01;

    expect(!verifyECDSA(CurveID::NIST_P256, publicKey, modifiedMessage, signature),
           "ECDSA signature must fail for modified message.");

    std::cout << " Modified message correctly rejected.\n";
}

void testECDSAWrongSignature()
{
    printSection("DUOX ECC : modified ECDSA signature rejection.");

    const EVP_PKEY_ptr key = generateECKey(NID_X9_62_prime256v1);

    const ByteVector privateKey = getPrivateScalar(key.get());
    PublicKey publicKey = makePublicKey(key.get());

    const ByteVector message{0x01, 0x02, 0x03};
    const ECDSASignature signature = signECDSA(CurveID::NIST_P256, privateKey, message);

    ByteVector modifiedMessage = message;
    modifiedMessage[0] ^= 0x01;

    expect(!verifyECDSA(CurveID::NIST_P256, publicKey, modifiedMessage, signature),
           "ECDSA signature must fail for modified message.");

    std::cout << " Modified ECDSA signatures correctly rejected.\n";
}

void testECDSAWrongPublicKey()
{
    printSection("DUOX ECC : wrong ECDSA public key rejection.");

    const EVP_PKEY_ptr signer = generateECKey(NID_X9_62_prime256v1);
    const EVP_PKEY_ptr wrong = generateECKey(NID_X9_62_prime256v1);

    const ByteVector privateKey = getPrivateScalar(signer.get());
    PublicKey wrongPublicKey = makePublicKey(wrong.get());

    const ByteVector message{0xAA, 0xBB, 0xCC};
    const ECDSASignature signature = signECDSA(CurveID::NIST_P256, privateKey, message);

    expect(!verifyECDSA(CurveID::NIST_P256, wrongPublicKey, message, signature),
           "signature must not verify with unrelated public key.");

    std::cout << " Unrelated ECDSA public key correctly rejected.\n";
}

void testECDSAInvalidInputs()
{
    printSection("DUOX ECC : invalid ECDSA inputs.");

    const EVP_PKEY_ptr key = generateECKey(NID_X9_62_prime256v1);

    PublicKey publicKey = makePublicKey(key.get());

    const ByteVector message{0x01, 0x02};

    expectThrows<std::invalid_argument>(
        [&] { signECDSA(CurveID::NIST_P256, ByteVector{}, message); }, "empty ECDSA private key.");
    expectThrows<std::invalid_argument>(
        [&] { signECDSA(CurveID::NIST_P256, ByteVector(31, 0x01), message); }, "31-byte ECDSA private key.");
    expectThrows<std::invalid_argument>(
        [&] { signECDSA(CurveID::NIST_P256, ByteVector(33, 0x01), message); }, "33-byte ECDSA private key.");
    expectThrows<std::invalid_argument>(
        [&] { signECDSA(CurveID::NIST_P256, ByteVector(32, 0x00), message); }, "zero ECDSA private key.");

    ECDSASignature invalidSignature;
    invalidSignature.r = ByteVector(31, 0x01);
    invalidSignature.s = ByteVector(32, 0x01);

    expectThrows<std::invalid_argument>(
        [&] { verifyECDSA(CurveID::NIST_P256, publicKey, message, invalidSignature); }, "31-byte ECDSA r.");

    invalidSignature.r = ByteVector(32, 0x01);
    invalidSignature.s = ByteVector(31, 0x01);

    expectThrows<std::invalid_argument>(
        [&] { verifyECDSA(CurveID::NIST_P256, publicKey, message, invalidSignature); }, "31-byte ECDSA s.");

    std::cout << " Invalid ECDSA inputs correctly rejected.\n";
}

/*
 * ==============================================
 * DER ECDSA signature encoding
 * ==============================================
 */

void testECDSADERRoundTrip()
{
    printSection("DUOX ECC : ECDSA DER round-trip.");

    const EVP_PKEY_ptr key = generateECKey(NID_X9_62_prime256v1);

    const ByteVector privateKey = getPrivateScalar(key.get());

    const ByteVector message{0xDE, 0xAD, 0xBE, 0xEF};

    const ECDSASignature original = signECDSA(CurveID::NIST_P256, privateKey, message);
    const ByteVector der = encodeECDSASignatureDER(original);

    expect(!der.empty(), "DER signature must not be empty.");

    const ECDSASignature decoded = decodeECDSASignatureDER(der);

    expect(decoded.r.size() == DUOX_COORDINATE_SIZE, "decoded DER r must be 32 bytes.");
    expect(decoded.s.size() == DUOX_COORDINATE_SIZE, "decoded DER s must be 32 bytes.");
    expect(decoded.r == original.r, "DER round-trip must preserve r.");
    expect(decoded.s == original.s, "DER round-trip must preserve s.");

    std::cout << " ECDSA DER round-trip succeeded.\n";
}

void testECDSADERMalformed()
{
    printSection("DUOX ECC : malformed ECDSA DER.");

    expectThrows<std::invalid_argument>(
        [] { decodeECDSASignatureDER(ByteVector{}); }, "empty DER signature.");
    expectThrows<std::invalid_argument>(
        [] { decodeECDSASignatureDER(ByteVector{0x01, 0x02, 0x03}); }, "non-SEQUENCE DER signature.");
    expectThrows<std::invalid_argument>(
        [] { decodeECDSASignatureDER(ByteVector{0x30, 0x00}); }, "empty DER SEQUENCE.");

    // SEQUENCE containing only one INTEGER.
    expectThrows<std::invalid_argument>(
        [] { decodeECDSASignatureDER(ByteVector{0x30, 0x03, 0x02, 0x01, 0x01}); }, "DER signature missing s.");

    //SEQUENCE containing an INTEGER with zero length.
    expectThrows<std::invalid_argument>(
        [] { decodeECDSASignatureDER(ByteVector{0x30, 0x05, 0x02, 0x00, 0x02, 0x01, 0x01}); },
        "DER signature with zero-length r.");

    std::cout << " Malformed ECDSA DER correctly rejected.\n";
}

void testECDSADERSignatureSizes()
{
    printSection("DUOX ECC : ECDSA DER signature size validation.");

    ECDSASignature signature;

    signature.r = ByteVector(31, 0x01);
    signature.s = ByteVector(32, 0x01);
    expectThrows<std::invalid_argument>([&] { encodeECDSASignatureDER(signature); }, "31-byte r.");

    signature.r = ByteVector(32, 0x01);
    signature.s = ByteVector(31, 0x01);
    expectThrows<std::invalid_argument>([&] { encodeECDSASignatureDER(signature); }, "31-byte s.");

    signature.r = ByteVector(33, 0x01);
    signature.s = ByteVector(32, 0x01);
    expectThrows<std::invalid_argument>([&] { encodeECDSASignatureDER(signature); }, "33-byte r.");

    std::cout << " Invalid DER signature component sizes correctly rejected.\n";
}

/*
 * ==============================================
 * PublicKey ownership / move semantics
 * ==============================================
 */

void testPublicKeyDefault()
{
    printSection("PublicKey : default construction.");

    PublicKey key;
    expect(key.getPublicKey() == nullptr, "default PublicKey must not contain an EVP_PKEY.");

    std::cout << " Default PublicKey is empty.\n";
}

void testPublicKeyConstruction()
{
    printSection("PublicKey : ownership construction.");

    EVP_PKEY_ptr generated = generateECKey(NID_X9_62_prime256v1);
    EVP_PKEY *raw = generated.release();

    PublicKey key(raw);

    expect(key.getPublicKey() == raw, "PublicKey must take ownership of supplied EVP_PKEY.");
    expect(key.getPublicKey() != nullptr, "PublicKey must contain supplied EVP_PKEY.");

    std::cout << " PublicKey ownership construction succeeded.\n";
}

void testPublicKeySet()
{
    printSection("PublicKey : setPublicKey.");

    EVP_PKEY_ptr first = generateECKey(NID_X9_62_prime256v1);
    EVP_PKEY_ptr second = generateECKey(NID_X9_62_prime256v1);

    PublicKey key(first.release());
    EVP_PKEY *secondRaw = second.release();
    key.setPublicKey(secondRaw);

    expect(key.getPublicKey() == secondRaw, "setPublicKey must replace the owned EVP_PKEY");

    std::cout << " PublicKey replacement succeeded.\n";
}

void testPublicKeyMoveConstruction()
{
    printSection("PublicKey : move construction.");

    EVP_PKEY_ptr generated = generateECKey(NID_X9_62_prime256v1);

    PublicKey original(generated.release());

    EVP_PKEY *originalKey = original.getPublicKey();
    expect(originalKey != nullptr, "original PublicKey must contain key.");

    PublicKey moved(std::move(original));
    expect(moved.getPublicKey() == originalKey, "move construction must transfer EVP_PKEY.");
    expect(original.getPublicKey() == nullptr, "moved-from PublicKey must be empty.");

    std::cout << " PublicKey move construction succeeded.\n";
}

void testPublicKeyMoveAssignment()
{
    printSection("PublicKey : move assignment.");

    EVP_PKEY_ptr first = generateECKey(NID_X9_62_prime256v1);
    EVP_PKEY_ptr second = generateECKey(NID_X9_62_prime256v1);

    PublicKey source(first.release());
    PublicKey destination(second.release());

    EVP_PKEY *sourceKey = source.getPublicKey();
    expect(sourceKey != nullptr, "source must contain key before move");

    destination = std::move(source);
    expect(destination.getPublicKey() == sourceKey, "move assignment must transfer source key.");
    expect(source.getPublicKey() == nullptr, "moved-from PublicKey must be empty.");

    std::cout << " PublicKey move assignment succeeded.\n";
}

/*
 * ==============================================
 * Test runner
 * ==============================================
 */

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
        std::cout << "[ SKIPPED ] " << name << '\n' << exception.what() << '\n';

        return {name, TestStatus::Skipped, exception.what()};
    }
    catch (const TestFailure &exception)
    {
        std::cout << "[ FAILED ] " << name << '\n' << exception.what() << '\n';

        return {name, TestStatus::Failed, exception.what()};
    }
    catch (const std::exception &exception)
    {
        std::cout << "[ FAILED ] " << name << '\n' << "Unexpected exception : " << exception.what() << '\n';

        return {name, TestStatus::Failed, exception.what()};
    }
    catch (...)
    {
        std::cout << "[ FAILED ] " << name << '\n' << "Unknown exception\n";

        return {name, TestStatus::Failed, "unknown exception"};
    }
}

} // namespace

int main()
{
    std::cout << "==============================================\n"
              << "LLA DUOX ECC TEST SUITE\n"
              << "==============================================\n";

    std::cout << "OpenSSL : " << OpenSSL_version(OPENSSL_VERSION) << '\n';
    std::vector<TestResult> results;

#define RUN_TEST(function) results.push_back(runTest(#function, function))

    // Curve abstraction
    RUN_TEST(testCoordinateSize);
    RUN_TEST(testUnsupportedCurve);

    // EC point representation
    RUN_TEST(testP256PointEncodeDecode);
    RUN_TEST(testBrainpoolPointEncodeDecode);
    RUN_TEST(testMalformedPointEncoding);
    RUN_TEST(testPointWrongCurve);
    RUN_TEST(testPointInvalidCoordinates);
    RUN_TEST(testPointInvalidSizes);

    // ECDH
    RUN_TEST(testP256ECDH);
    RUN_TEST(testBrainpoolECDH);
    RUN_TEST(testECDHInvalidPrivateKey);
    RUN_TEST(testECDHInvalidPeerPoint);
    RUN_TEST(testECDHWrongCurve);

    // DUOX ECC session key derivation
    RUN_TEST(testDUOXSessionKeyDerivation);
    RUN_TEST(testBrainpoolSessionKeyDerivation);
    RUN_TEST(testDUOXSessionKeyBinding);
    RUN_TEST(testDUOXSessionKeyDerivationInvalidInputs);
    RUN_TEST(testDUOXSessionKeyDerivationDeterministic);

    // ECDSA
    RUN_TEST(testP256ECDSASignVerify);
    RUN_TEST(testBrainpoolECDSASignVerify);
    RUN_TEST(testECDSAWrongMessage);
    RUN_TEST(testECDSAWrongSignature);
    RUN_TEST(testECDSAWrongPublicKey);
    RUN_TEST(testECDSAInvalidInputs);

    // ECDSA DER representation
    RUN_TEST(testECDSADERRoundTrip);
    RUN_TEST(testECDSADERMalformed);
    RUN_TEST(testECDSADERSignatureSizes);

    // PublicKey ownership
    RUN_TEST(testPublicKeyDefault);
    RUN_TEST(testPublicKeyConstruction);
    RUN_TEST(testPublicKeySet);
    RUN_TEST(testPublicKeyMoveConstruction);
    RUN_TEST(testPublicKeyMoveAssignment);

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

    if (failed != 0U)
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
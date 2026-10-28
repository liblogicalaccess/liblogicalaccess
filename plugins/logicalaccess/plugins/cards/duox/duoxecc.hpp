#pragma once

#include <cstddef>
#include <cstdint>

#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/plugins/cards/desfire/lla_cards_desfire_api.hpp>
#include <logicalaccess/plugins/crypto/public_key.hpp>

namespace logicalaccess
{

// TODO later for a final version : add namespace duox inside namespace logicalaccess

// \brief Identifier of the ECC curve used by DUOX authentication
enum class CurveID : std::uint8_t
{
    NIST_P256        = 0x0C,
    BRAINPOOL_P256R1 = 0x0D
};


// \brief Uncompressed elliptic curve public point without 0x04
struct ECPoint
{
    ByteVector x;
    ByteVector y;
};

// \brief ECDSA signature represented as two fixed width big-endian integers
struct ECDSASignature
{
    ByteVector r;
    ByteVector s;
};

/**
 * \brief Derived DUOX ECC authentication session keys
 *
 * Both keys are always exactly 16 bytes
 */
struct SessionKeys
{
    ByteVector encKey;
    ByteVector macKey;
};

// \brief DUOX elliptic curve key pair
struct ECKeyPair
{
    ByteVector privateKey;
    ECPoint publicKey;
};

/**
 * \brief Return the encoded coordinate size for a DUOX curve.
 *
 * \param curve DUOX elliptic curve
 *
 * \return Number of bytes used by each affine coordinate.
 */
LLA_CARDS_DESFIRE_API std::size_t getCoordinateSize(CurveID curve);

/**
 * \brief Decode a DUOX uncompressed EC point
 *
 * The input must use the exact DUOX representation : 0x04 || X || Y
 *
 * The point is structurally decoded only. No cryptographic point validation is performed
 *
 * \param curve DUOX elliptic curve used to determine the coordinate size
 * \param encoded DUOX encoded EC point
 *
 * \return Decoded EC point
 */
LLA_CARDS_DESFIRE_API ECPoint decodeECPoint(CurveID curve, const ByteVector &encoded);

/**
 * \brief Encode an EC point using the DUOX uncompressed representation
 *
 * The returned representation is : 0x04 || X || Y
 *
 * Coordinates must already have the exact size required by the curve
 *
 * \param curve DUOX elliptic curve
 * \param point EC point to encode
 *
 * \return DUOX encoded EC point
 */
LLA_CARDS_DESFIRE_API ByteVector encodeECPoint(CurveID curve, const ECPoint &point);

/**
 * \brief Validate an EC point against the selected DUOX curve
 *
 * Validation checks that :
 *   - the point is not at infinity
 *   - the coordinates are valid for the selected curve
 *   - the point belongs to the selected curve
 *
 * This performs actual elliptic curve public point validation rather than merely checking
 * the size or encoding
 *
 * \param curve DUOX elliptic curve.
 * \param point EC point to validate.
 *
 * \return true if the point is valid for the selected curve, false otherwise
 */
LLA_CARDS_DESFIRE_API bool validateECPoint(CurveID curve, const ECPoint &point);

/**
 * \brief Perform ECDH using a DUOX EC private key and peer public point
 *
 * The private key is a fixed width, big-endian scalar whose size must match the
 * coordinate size of the selected DUOX curve. The peer public point is validated before
 * the ECDH operation is performed
 *
 * The returned shared secret is the fixed width affine X coordinate of
 * the resulting shared point, as required by standard ECDH
 *
 * \param curve DUOX elliptic curve used by the key pair
 * \param privateKey Fixed width big-endian private scalar
 * \param peerPublicPoint Validated public point belonging to the curve
 *
 * \return The ECDH shared secret as a fixed width big-endian byte vector
 */
LLA_CARDS_DESFIRE_API ByteVector deriveECDHSharedSecret(CurveID curve, const ByteVector &privateKey,
    const ECPoint &peerPublicPoint);

/**
 * \brief Generate an ECDSA signature using ECDSA with SHA-256
 *
 * The supplied message is hashed internally using SHA-256 before the ECDSA operation
 *
 * The private key must be provided as a fixed width, big-endian scalar whose size matches
 * the selected DUOX curve
 *
 * \param curve DUOX elliptic curve used by the private key
 * \param privateKey Big-endian private scalar
 * \param message Message to authenticate
 *
 * \return ECDSA signature containing fixed-width r and s values
 */
LLA_CARDS_DESFIRE_API ECDSASignature signECDSA(CurveID curve, const ByteVector &privateKey, const ByteVector &message);

/**
 * \brief Verify an ECDSA signature using ECDSA with SHA-256
 *
 * \param curve DUOX elliptic curve used by the public key
 * \param publicKey Public EC point
 * \param message Message whose signature is being verified
 * \param signature Signature to verify
 *
 * \return true if the signature is valid, false if it is cryptographically invalid
 */
LLA_CARDS_DESFIRE_API bool verifyECDSA(CurveID curve, const PublicKey &publicKey, const ByteVector &message,
    const ECDSASignature &signature);

/**
 * \brief Encode a DUOX ECDSA signature using DER encoding
 *
 * \param signature ECDSA signature containing r and s values
 *
 * \return DER-encoded ECDSA signature
 */
LLA_CARDS_DESFIRE_API ByteVector encodeECDSASignatureDER(const ECDSASignature &signature);

/**
 * \brief Decode a DER ECDSA signature
 *
 * The resulting r and s values are always exactly 32 bytes
 *
 * \param encoded DER-encoded ECDSA signature
 *
 * \return Decoded ECDSA signature
 */
LLA_CARDS_DESFIRE_API ECDSASignature decodeECDSASignatureDER(const ByteVector &encoded);

/**
 * \brief Derive the DUOX ECC authentication session keys
 *
 * Implements the two steps DUOX key derivation
 *
 * \return Derived DUOX session encryption and MAC keys
 */
LLA_CARDS_DESFIRE_API SessionKeys deriveSessionKeys(const ECPoint &ephemeralPublicA, const ECPoint &ephemeralPublicB,
    const ByteVector &sharedSecret);

/**
 * \brief Generate a DUOX elliptic curve key pair
 *
 * \param curve DUOX elliptic curve
 *
 * \return Generated EC key pair
 */
LLA_CARDS_DESFIRE_API ECKeyPair generateECKeyPair(CurveID curve);


} // namespace logicalaccess
#pragma once

#include <cstdint>
#include <memory>
#include <utility>

#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/plugins/cards/duox/duoxevcharging.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2crypto.hpp>
#include <logicalaccess/plugins/cards/duox/duoxCertificateValidator.hpp>

namespace logicalaccess
{

/**
 * \brief Cryptographic implementation for DUOX cards.
 *
 * Provides the cryptographic operations required by DUOX authentication
 * and key management while reusing the common DESFire EV2 secure-messaging implementation from DESFireEV2Crypto.
 */
class LLA_CARDS_DESFIRE_API DUOXCrypto : public DESFireEV2Crypto
{
  public:
    DUOXCrypto()           = default;
    ~DUOXCrypto() override = default;
    
    /**
     * \brief Rotate an EV2 authentication challenge left by one byte.
     *
     * \param rnd Authentication challenge. Must contain exactly 16 bytes.
     *
     * \return The rotated authentication challenge.
     *
     * \throws LibLogicalAccessException if rnd does not contain exactly 16 bytes.
     */
    static ByteVector rotateAuthenticationChallenge(const ByteVector &rnd);
    
    /**
     * \brief Derive the DUOX EV2 session encryption and MAC keys
     *
     * DUOX supports both AES-128 and AES-256 authentication keys.
     * AES-128 derives each session key from one CMAC block, while AES-256 derives each session key from two CMAC blocks.
     *
     * \param key Diversified authentication key.
     * \param rndA PCD-generated authentication challenge.
     * \param rndB PICC-generated authentication challenge.
     * 
     * \return A pair containing the session encryption key followed by the session MAC key.
     *
     * \throws LibLogicalAccessException if the authentication key or random challenges have an invalid size.
     */
    std::pair<ByteVector, ByteVector>
    deriveSessionKeys(const ByteVector &key, const ByteVector &rndA, const ByteVector &rndB);

    /**
     * \brief Build the plaintext key change payload for DUOX.
     *
     * DUOX supports AES-128 and AES-256 keys.
     * When changing a key other than the currently authenticated key, the payload contains the XOR
     * between the existing key and the new key, plus the new key version and CRC32.
     *
     * When changing the currently authenticated key, the new key is stored directly followed by its key version.
     *
     * \param keySetNo Key set containing the target key.
     * \param keyNo Target key number.
     * \param oldKeyDiversify Diversification data for the existing key.
     * \param newKey New AES key.
     * \param newKeyDiversify Diversification data for the new key.
     *
     * \return The DUOX KeyData payload.
     *
     * \throws LibLogicalAccessException if the new key is invalid or the generated payload has an invalid length.
     */
    ByteVector buildChangeKeyData(std::uint8_t keySetNo, std::uint8_t keyNo,
                                  const ByteVector &oldKeyDiversify,
                                  const std::shared_ptr<DESFireKey> newKey,
                                  const ByteVector &newKeyDiversify);

    // ========================================
    // EV CHARGING - CRYPTOGRAPHIC API
    // ========================================

    /**
     * \brief Validate an EV Charging challenge.
     *
     * \throws LibLogicalAccessException if the challenge length is not exactly 32 bytes
     */
    static void VDE_ValidateChallenge(const ByteVector &challenge);

    /**
     * \brief Validate a raw ECDSA signature.
     *
     * \throws LibLogicalAccessException if the signature length is not exactly 64 bytes
     */
    static void VDE_ValidateSignature(const ByteVector &signature);

    /**
     * \brief Validate an uncompressed BrainpoolP256r1 public key.
     *
     * Checks :
     * - Exact 65 byte representation
     * - Uncompressed point prefix
     * - Valid coordinates
     * - Point membership on BrainpoolP256r1
     * - Point is not the point at infinity
     *
     * \throws LibLogicalAccessException if invalid
     */
    static void VDE_ValidatePublicKey(const ByteVector &publicKey);

    /**
     * \brief Verify an ECDSA signature over a challenge.
     *
     * Uses ECDSA with SHA-256 and BrainpoolP256r1.
     *
     * \param publicKey Uncompressed public key.
     * \param challenge Original 32 byte challenge.
     * \param signature Raw r || s signature.
     *
     * \return true if verification succeeds.
     */
    static bool VDE_ECDSAVerify(const ByteVector &publicKey, const ByteVector &challenge,
                                const ByteVector &signature);

    /**
     * \brief Parse an EV Charging certificate.
     *
     * Does not establish certificate authenticity
     */
    static EVChargingCertificate VDE_ParseCertificate(const ByteVector &certificate);

    /**
     * \brief Verify a parsed EV Charging certificate.
     *
     * \param certificate Parsed certificate
     * \param caPublicKey Trusted CA public key.
     */
    static bool VDE_VerifyCertificate(const EVChargingCertificate &certificate, const ByteVector &caPublicKey);

    /**
     * \brief Parse and verify an EV Charging certificate
     */
    static bool VDE_VerifyCertificate(const ByteVector &certificate, const ByteVector &caPublicKey);

    /**
     * \brief Verify the certificate UID against the physical card UID
     */
    static bool VDE_VerifyCertificateUID(const EVChargingCertificate &certificate, const ByteVector &cardUID);

    /**
     * \brief Verify certificate validity against a supplied date.
     */
    static bool VDE_IsCertificateValidAt(const EVChargingCertificate &certificate, const EVChargingDate &currentDate);

    /**
     * \brief Validate the structural and semantic consistency of a parsed EV Charging certificate.
     *
     * This operation does not establish certificate authenticity.
     *
     * Checks :
     * - Mandatory certificate fields are present
     * - UID has a supported length
     * - Public key has a valid BrainpoolP256r1 representation
     * - Signature has the expected raw ECDSA representation
     * - Certificate dates have the expected encoding length
     * - Signed data is present
     *
     * \param certificate Parsed EV Charging certificate.
     *
     * \throws LibLogicalAccessException if the certificate is invalid.
     */
    static void VDE_ValidateCertificate(const EVChargingCertificate &certificate);

  private:
    /**
     * \brief Build the DUOX EV2 KDF context from RndA and RndB.
     * 
     * The resulting context is the 26 byte value used by the DUOX EV2 session-key derivation.
     */
    static ByteVector buildKDFContext(const ByteVector &rndA, const ByteVector &rndB);


    /**
     * \brief Build a 32 byte DUOX EV2 KDF input vector.
     *
     * \param label0 First byte of the KDF label.
     * \param label1 Second byte of the KDF label.
     * \param counter KDF block counter.
     * \param length Requested KDF output length in bits.
     * \param context 26 byte KDF context.
     * 
     * \return The 32 byte KDF input vector.
     */
    static ByteVector buildKDFVector(std::uint8_t label0, std::uint8_t label1,
                                         std::uint16_t counter, std::uint16_t length,
                                         const ByteVector &context);

    /**
     * \brief Calculates SHA-256 over a byte sequence.
     *
     * \param data Input data.
     *
     * \return 32 byte SHA-256 digest.
     */
    static ByteVector calculateSHA256(const ByteVector &data);
};

} // namespace logicalaccess
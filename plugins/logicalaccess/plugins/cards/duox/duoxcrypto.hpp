#pragma once

#include <cstdint>
#include <memory>
#include <utility>

#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2crypto.hpp>

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
     * \brief Build the plaintext key-change payload for DUOX.
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

  private:
    /**
     * \brief Build the DUOX EV2 KDF context from RndA and RndB.
     * 
     * The resulting context is the 26-byte value used by the DUOX EV2 session-key derivation.
     */
    static ByteVector buildKDFContext(const ByteVector &rndA, const ByteVector &rndB);


    /**
     * \brief Build a 32-byte DUOX EV2 KDF input vector.
     *
     * \param label0 First byte of the KDF label.
     * \param label1 Second byte of the KDF label.
     * \param counter KDF block counter.
     * \param length Requested KDF output length in bits.
     * \param context 26-byte KDF context.
     * 
     * \return The 32-byte KDF input vector.
     */
    static ByteVector buildKDFVector(std::uint8_t label0, std::uint8_t label1,
                                         std::uint16_t counter, std::uint16_t length,
                                         const ByteVector &context);
};

} // namespace logicalaccess
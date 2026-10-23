#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <logicalaccess/plugins/cards/desfire/desfireaccessinfo.hpp>
#include <logicalaccess/plugins/crypto/openssl.hpp>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{

/**
 * \brief Transaction MAC session keys.
 *
 * Both keys have the same size as the application TMAC key : 16 bytes for AES-128 or 32 bytes for AES-256.
 */
struct TransactionMACSessionKeys
{
    ByteVector macKey;
    ByteVector encryptionKey;
};

/**
 * \brief DESFire Transaction MAC cryptographic operations.
 * 
 * Provides functions for deriving session keys, verifying transaction MAC values and decrypting reader IDs.
 */
class LLA_CARDS_DESFIRE_API DESFireTransactionMACCrypto
{
  public:
    static constexpr std::size_t AES_128_KEY_SIZE = 16;
    static constexpr std::size_t AES_256_KEY_SIZE = 32;
    static constexpr std::size_t AES_BLOCK_SIZE   = 16;

    // TODO : centralize code with namespace TransactionManagement from desfirrev2commands.hpp
    static constexpr std::size_t TMC_SIZE         = 4;
    static constexpr std::size_t TMV_SIZE         = 8;
    static constexpr std::size_t READER_ID_SIZE   = 16;

    /**
     * \brief Derive AES transaction session keys from TMAC key and counter.
     *
     * Supports AES-128 and AES-256 application TMAC keys.
     * AES-128 produces 16 byte session keys.
     * AES-256 produces 32 byte session keys by concatenating two CMAC outputs per key.
     * 
     * \param applicationTMACKey The application Transaction MAC key (16 or 32 bytes).
     * \param tmcForTransaction Counter value used directly for session-key derivation, already incremented
     * \param uid Card UID
     *
     * \return Derived session keys for MAC and encryption
     */
    static TransactionMACSessionKeys deriveAESSessionKeys(const ByteVector &applicationTMACKey,
                                                          const std::array<std::uint8_t, TMC_SIZE> &tmcForTransaction,
                                                          const ByteVector &uid);

    /**
     * Verify a Transaction MAC Value against protocol-defined TMI.
     *
     * \param applicationTMACKey The application Transaction MAC key
     * \param tmcForTransaction Counter value used directly for session-key derivation, already incremented
     * \param uid Card UID
     * \param tmi Transaction MAC Input with protocol-defined padding
     * \param expectedTMV Expected 8 byte TMV value
     * 
     * \return true if TMV is valid, false otherwise
     */
    static bool verifyTMV(const ByteVector &applicationTMACKey,
                          const std::array<std::uint8_t, TMC_SIZE> &tmcForTransaction,
                          const ByteVector &uid, const ByteVector &tmi,
                          const std::array<std::uint8_t, TMV_SIZE> &expectedTMV);

    /**
     * \brief Decrypt previous Reader ID from CommitReaderID response.
     * 
     * Uses AES-CBC with all-zero IV and no padding.
     *
     * \param applicationTMACKey The application Transaction MAC key
     * \param tmcForTransaction Counter value used directly for session-key derivation, already incremented
     * \param uid Card UID
     * \param encryptedReaderID Encrypted Reader ID (exactly one AES block, 16 bytes)
     * 
     * \return Decrypted 16 byte Reader ID
     */
    static std::array<std::uint8_t, READER_ID_SIZE>
    decryptPreviousReaderID(const ByteVector &applicationTMACKey,
                            const std::array<std::uint8_t, TMC_SIZE> &tmcForTransaction,
                            const ByteVector &uid, const ByteVector &encryptedReaderID);
};

} // namespace logicalaccess
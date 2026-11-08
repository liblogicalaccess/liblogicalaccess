#pragma once

#include <array>
#include <cstdint>

#include <logicalaccess/plugins/cards/desfire/transactioncommon.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireaccessinfo.hpp>
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
 * Provides functions for deriving session keys, verifying transaction MAC values (TMV) and decrypting reader IDs.
 */
class LLA_CARDS_DESFIRE_API DESFireTransactionMACCrypto
{
  public:

    /**
     * \brief Derive AES transaction session keys from TMAC key and counter.
     *
     * Supports AES-128 and AES-256 application TMAC keys.
     * AES-128 produces 16 byte session keys.
     * AES-256 produces 32 byte session keys by concatenating two CMAC outputs per key.
     * 
     * Callers must secure the returned keys using SecureZeroGuard or equivalent RAII wrapper
     * to prevent sensitive key material from persisting in memory.
     * 
     * \param applicationTMACKey The application Transaction MAC key (16 or 32 bytes).
     * \param tmcForTransaction Counter value used directly for session-key derivation, already incremented
     * \param uid Card UID
     *
     * \return Derived session keys for MAC and encryption
     */
    static TransactionMACSessionKeys deriveAESSessionKeys(const ByteVector &applicationTMACKey,
                                                          const TransactionManagement::TransactionCounter &tmcForTransaction,
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
                          const TransactionManagement::TransactionCounter &tmcForTransaction,
                          const ByteVector &uid, const ByteVector &tmi,
                          const TransactionManagement::TransactionMACValue &expectedTMV);

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
    static TransactionManagement::ReaderID decryptPreviousReaderID(
        const ByteVector &applicationTMACKey,
        const TransactionManagement::TransactionCounter &tmcForTransaction,
        const ByteVector &uid, const ByteVector &encryptedReaderID);
};

} // namespace logicalaccess
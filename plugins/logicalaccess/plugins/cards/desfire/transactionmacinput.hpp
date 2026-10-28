#pragma once

#include <cstddef>
#include <cstdint>

#include <logicalaccess/plugins/cards/desfire/transactioncommon.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireaccessinfo.hpp>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{

/**
 * \brief Builds and maintains the Transaction MAC Input (TMI) for DESFire transactions.
 *
 * Encodes supported DESFire commands and their associated data into the protocol-defined TMI representation,
 * including zero-padding and block alignment.
 *
 * Each successful append operation adds a complete TMI update without modifying previously accumulated data.
 * Accumulated input can be cleared after a successful transaction commit or abort.
 */
class LLA_CARDS_DESFIRE_API DESFireTransactionMACInput
{
  public:
    static constexpr std::size_t BLOCK_SIZE     = TransactionManagement::AES_BLOCK_SIZE;
    static constexpr std::size_t READER_ID_SIZE = TransactionManagement::READER_ID_SIZE;
    static constexpr std::uint32_t U24_MAX      = TransactionManagement::U24_MAX;

    // DESFire command identifiers
    static constexpr std::uint8_t INS_READ_DATA         = 0xBD;
    static constexpr std::uint8_t INS_WRITE_DATA        = 0x3D;
    static constexpr std::uint8_t INS_GET_VALUE         = 0x6C;
    static constexpr std::uint8_t INS_CREDIT            = 0x0C;
    static constexpr std::uint8_t INS_DEBIT             = 0xDC;
    static constexpr std::uint8_t INS_LIMITED_CREDIT    = 0x1C;
    static constexpr std::uint8_t INS_WRITE_RECORD      = 0x3B;
    static constexpr std::uint8_t INS_READ_RECORDS      = 0xBB;
    static constexpr std::uint8_t INS_UPDATE_RECORD     = 0xBA;
    static constexpr std::uint8_t INS_CLEAR_RECORD_FILE = 0xEB;
    static constexpr std::uint8_t INS_COMMIT_READER_ID  = 0xC8;

    /**
     * \brief Removes all accumulated TMI data.
     */
    void clear() noexcept;
    /**
     * \brief Checks whether the accumulated TMI is empty.
     *
     * \return true if no TMI data has been accumulated, false otherwise.
     */
    bool empty() const noexcept;
    /**
     * \brief Returns the size of the accumulated TMI in bytes.
     *
     * \return Number of accumulated bytes.
     */
    std::size_t size() const noexcept;

    /**
     * \brief Replaces the accumulated TMI data.
     *
     * \param data New TMI data.
     */
    void setData(ByteVector data);
    /**
     * \brief Returns the accumulated TMI data.
     *
     * \return Constant reference to the encoded TMI bytes.
     */
    const ByteVector &data() const noexcept;

    /**
     * \brief Encodes an unsigned 24-bit integer in little-endian byte order.
     *
     * \param value Value to encode, in the range 0 to 0xFFFFFF.
     * \return Three encoded bytes.
     * 
     * \throws LibLogicalAccessException If the value exceeds the 24-bit range.
     */
    static ByteVector encodeU24(std::uint32_t value);
    
    /**
     * \brief Encodes a signed 32-bit integer in little-endian byte order.
     *
     * \param value Value to encode.
     * \return Four encoded bytes.
     */
    static ByteVector encodeI32(std::int32_t value);

    /**
     * \brief Appends a logical TMI update with zero-padding to a block boundary.
     *
     * \param update Logical TMI update to append.
     * 
     * \throws LibLogicalAccessException If the update is empty or the append fails validation.
     */
    void appendUpdate(const ByteVector &update);

    /**
     * \brief Appends a padded command header followed by a separately padded payload.
     *
     * \param header Command header to encode.
     * \param payload Logical payload associated with the header.
     * 
     * \throws LibLogicalAccessException If the header is empty or the resulting update fails validation.
     */
    void appendHeaderAndData(const ByteVector &header, const ByteVector &payload);

    // ReadData TMI update
    void appendReadData(std::uint8_t fileNo, std::uint32_t offset, std::uint32_t length, const ByteVector &returnedData);

    // WriteData TMI update
    void appendWriteData(std::uint8_t fileNo, std::uint32_t offset, const ByteVector &writtenData);

    // CommitReaderID TMI update
    // Includes the encrypted previous Reader ID when authenticated
    void appendCommitReaderID(const ByteVector &readerID, bool authenticated,
                              const ByteVector &encryptedPreviousReaderID = {});

    // GetValue / Credit / Debit / LimitedCredit TMI update
    void appendValueOperation(std::uint8_t command, std::uint8_t fileNo, std::int32_t value);

    // ReadRecords TMI update
    void appendReadRecords(std::uint8_t fileNo, std::uint32_t recordNo, std::uint32_t requestedRecordCount,
                           std::uint32_t actualRecordCount, const ByteVector &returnedRecords);

    // WriteRecord TMI update
    void appendWriteRecord(std::uint8_t fileNo, std::uint32_t offset, const ByteVector &recordData);

    // UpdateRecord TMI update
    void appendUpdateRecord(std::uint8_t fileNo, std::uint32_t recordNo,
                            std::uint32_t offset, const ByteVector &recordData);

    // ClearRecordFile TMI update
    void appendClearRecordFile(std::uint8_t fileNo);

  private:
    /**
     * \brief Appends an unsigned 24-bit integer in little-endian order.
     *
     * \param out Destination byte vector.
     * \param value Value to encode.
     */
    static void appendU24(ByteVector &out, std::uint32_t value);
    /**
     * \brief Pads data with zero bytes to the next block boundary.
     *
     * Leaves data unchanged if it is already block-aligned.
     *
     * \param data Data to pad.
     */
    static void padToBlock(ByteVector &data);
    /**
     * \brief Appends a non-empty, block-aligned TMI update.
     *
     * \param update Encoded update to append.
     * 
     * \throws LibLogicalAccessException If the update is invalid or the accumulated size would overflow.
     */
    void appendAligned(const ByteVector &update);

    ByteVector data_;
};

} // namespace logicalaccess
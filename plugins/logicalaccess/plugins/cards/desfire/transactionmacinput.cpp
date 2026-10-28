#include <logicalaccess/plugins/cards/desfire/transactionmacinput.hpp>

#include <limits>
#include <utility>

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{

void DESFireTransactionMACInput::clear() noexcept
{
    data_.clear();
}

bool DESFireTransactionMACInput::empty() const noexcept
{
    return data_.empty();
}

std::size_t DESFireTransactionMACInput::size() const noexcept
{
    return data_.size();
}

void DESFireTransactionMACInput::setData(ByteVector data)
{
    data_ = std::move(data);
}

const ByteVector &DESFireTransactionMACInput::data() const noexcept
{
    return data_;
}

ByteVector DESFireTransactionMACInput::encodeU24(std::uint32_t value)
{
    EXCEPTION_ASSERT_WITH_LOG(value <= U24_MAX,
        LibLogicalAccessException, "DESFire TMI : Value exceeds the unsigned 24-bit range.");

    return {static_cast<std::uint8_t>(value),
            static_cast<std::uint8_t>(value >> 8U),
            static_cast<std::uint8_t>(value >> 16U)};
}

ByteVector DESFireTransactionMACInput::encodeI32(std::int32_t value)
{
    const auto encoded = static_cast<std::uint32_t>(value);

    return {static_cast<std::uint8_t>(encoded),
            static_cast<std::uint8_t>(encoded >> 8U),
            static_cast<std::uint8_t>(encoded >> 16U),
            static_cast<std::uint8_t>(encoded >> 24U)};
}

void DESFireTransactionMACInput::appendUpdate(const ByteVector &update)
{
    EXCEPTION_ASSERT_WITH_LOG(!update.empty(),
        LibLogicalAccessException, "DESFire TMI : Update cannot be empty.");

    ByteVector padded = update;
    padToBlock(padded);
    appendAligned(padded);
}

void DESFireTransactionMACInput::appendHeaderAndData(const ByteVector &header, const ByteVector &payload)
{
    EXCEPTION_ASSERT_WITH_LOG(!header.empty(),
        LibLogicalAccessException, "DESFire TMI : Header cannot be empty.");

    ByteVector update = header;
    padToBlock(update);

    update.insert(update.end(), payload.begin(), payload.end());
    padToBlock(update);

    appendAligned(update);
}

void DESFireTransactionMACInput::appendReadData(std::uint8_t fileNo, std::uint32_t offset,
                                                std::uint32_t length,
                                                const ByteVector &returnedData)
{
    EXCEPTION_ASSERT_WITH_LOG(offset <= U24_MAX,
        LibLogicalAccessException, "DESFire TMI : ReadData offset exceeds the 24-bit range.");

    EXCEPTION_ASSERT_WITH_LOG(returnedData.size() <= U24_MAX,
        LibLogicalAccessException, "DESFire TMI : ReadData response exceeds the 24-bit length range.");

    const auto effectiveLength = length == 0 ? static_cast<std::uint32_t>(returnedData.size()) : length;

    EXCEPTION_ASSERT_WITH_LOG(effectiveLength == returnedData.size(),
        LibLogicalAccessException, "DESFire TMI : ReadData response length does not match the effective length.");

    ByteVector header{INS_READ_DATA, fileNo};
    appendU24(header, offset);
    appendU24(header, effectiveLength);

    appendHeaderAndData(header, returnedData);
}

void DESFireTransactionMACInput::appendWriteData(std::uint8_t fileNo, std::uint32_t offset, const ByteVector &writtenData)
{
    EXCEPTION_ASSERT_WITH_LOG(offset <= U24_MAX,
        LibLogicalAccessException, "DESFire TMI : WriteData offset exceeds the 24-bit range.");

    EXCEPTION_ASSERT_WITH_LOG(writtenData.size() <= U24_MAX,
        LibLogicalAccessException, "DESFire TMI : WriteData payload exceeds the 24-bit length range.");

    ByteVector header{INS_WRITE_DATA, fileNo};
    appendU24(header, offset);
    appendU24(header, static_cast<std::uint32_t>(writtenData.size()));

    appendHeaderAndData(header, writtenData);
}

void DESFireTransactionMACInput::appendCommitReaderID(const ByteVector &readerID, bool authenticated,
                                                      const ByteVector &encryptedPreviousReaderID)
{
    EXCEPTION_ASSERT_WITH_LOG(readerID.size() == READER_ID_SIZE, LibLogicalAccessException,
        "DESFire TMI : Reader ID must contain exactly one AES block.");

    EXCEPTION_ASSERT_WITH_LOG(authenticated || encryptedPreviousReaderID.empty(), LibLogicalAccessException,
        "DESFire TMI : Unauthenticated CommitReaderID mustn't include previous Reader ID data.");

    EXCEPTION_ASSERT_WITH_LOG(!authenticated || encryptedPreviousReaderID.size() == READER_ID_SIZE,
        LibLogicalAccessException,
        "DESFire TMI : Authenticated CommitReaderID requires a block-sized encrypted previous Reader ID.");

    ByteVector update{INS_COMMIT_READER_ID};
    update.insert(update.end(), readerID.begin(), readerID.end());

    if (authenticated)
        update.insert(update.end(), encryptedPreviousReaderID.begin(), encryptedPreviousReaderID.end());

    appendUpdate(update);
}

void DESFireTransactionMACInput::appendValueOperation(std::uint8_t command, std::uint8_t fileNo, std::int32_t value)
{
    EXCEPTION_ASSERT_WITH_LOG(command == INS_GET_VALUE ||
                              command == INS_CREDIT ||
                              command == INS_DEBIT ||
                              command == INS_LIMITED_CREDIT,
                              LibLogicalAccessException, "DESFire TMI : Unsupported value operation command.");

    ByteVector update{command, fileNo};
    const auto encoded = encodeI32(value);

    update.insert(update.end(), encoded.begin(), encoded.end());

    appendUpdate(update);
}

void DESFireTransactionMACInput::appendReadRecords(std::uint8_t fileNo, std::uint32_t recordNo,
                                                   std::uint32_t requestedRecordCount,
                                                   std::uint32_t actualRecordCount,
                                                   const ByteVector &returnedRecords)
{
    EXCEPTION_ASSERT_WITH_LOG(recordNo <= U24_MAX && requestedRecordCount <= U24_MAX && actualRecordCount <= U24_MAX,
        LibLogicalAccessException, "DESFire TMI : ReadRecords parameter exceeds the 24-bit range.");

    EXCEPTION_ASSERT_WITH_LOG(requestedRecordCount == 0 || actualRecordCount <= requestedRecordCount,
        LibLogicalAccessException, "DESFire TMI : ReadRecords returned more records than requested.");

    EXCEPTION_ASSERT_WITH_LOG(requestedRecordCount == 0 || actualRecordCount == requestedRecordCount,
        LibLogicalAccessException, "DESFire TMI : ReadRecords returned count differs from requested count.");

    EXCEPTION_ASSERT_WITH_LOG(returnedRecords.size() % BLOCK_SIZE == 0,
        LibLogicalAccessException, "DESFire TMI : ReadRecords data must contain complete records.");

    ByteVector update{INS_READ_RECORDS, fileNo};

    appendU24(update, recordNo);
    appendU24(update, requestedRecordCount == 0 ? actualRecordCount : requestedRecordCount);
    padToBlock(update); // Pad header (8 bytes) to next 16 byte block boundary with zeros

    // Record data is already composed of complete 16 byte records
    update.insert(update.end(), returnedRecords.begin(), returnedRecords.end());

    appendAligned(update);
}

void DESFireTransactionMACInput::appendWriteRecord(std::uint8_t fileNo, std::uint32_t offset, const ByteVector &recordData)
{
    EXCEPTION_ASSERT_WITH_LOG(!recordData.empty() && recordData.size() % BLOCK_SIZE == 0,
        LibLogicalAccessException, "DESFire TMI : WriteRecord data must contain complete records.");

    EXCEPTION_ASSERT_WITH_LOG(offset <= U24_MAX && recordData.size() <= U24_MAX,
        LibLogicalAccessException, "DESFire TMI : WriteRecord parameter exceeds the 24-bit range.");

    ByteVector update{INS_WRITE_RECORD, fileNo};

    appendU24(update, offset);
    appendU24(update, static_cast<std::uint32_t>(recordData.size()));

    padToBlock(update); // Pad header (8 bytes) to next 16 byte block boundary with zeros

    update.insert(update.end(), recordData.begin(), recordData.end());

    appendAligned(update);
}

void DESFireTransactionMACInput::appendUpdateRecord(std::uint8_t fileNo, std::uint32_t recordNo,
                                                    std::uint32_t offset, const ByteVector &recordData)
{
    EXCEPTION_ASSERT_WITH_LOG(!recordData.empty(),
        LibLogicalAccessException, "DESFire TMI : UpdateRecord data cannot be empty.");

    EXCEPTION_ASSERT_WITH_LOG(recordNo <= U24_MAX && offset <= U24_MAX && recordData.size() <= U24_MAX,
        LibLogicalAccessException, "DESFire TMI : UpdateRecord parameter exceeds the 24-bit range.");

    ByteVector update{INS_UPDATE_RECORD, fileNo};

    appendU24(update, recordNo);
    appendU24(update, offset);
    appendU24(update, static_cast<std::uint32_t>(recordData.size()));

    padToBlock(update); // Pad header (11 bytes) to next 16 byte block boundary with zeros

    update.insert(update.end(), recordData.begin(), recordData.end());

    // UpdateRecord data is not unconditionally padded by this encoder
    // Reject data not block-aligned with TMI invariant.
    EXCEPTION_ASSERT_WITH_LOG(update.size() % BLOCK_SIZE == 0,
        LibLogicalAccessException, "DESFire TMI : UpdateRecord data must end on a block boundary.");

    appendAligned(update);
}

void DESFireTransactionMACInput::appendClearRecordFile(std::uint8_t fileNo)
{
    appendUpdate(ByteVector{INS_CLEAR_RECORD_FILE, fileNo});
}

void DESFireTransactionMACInput::appendU24(ByteVector &out, std::uint32_t value)
{
    const auto encoded = encodeU24(value);
    out.insert(out.end(), encoded.begin(), encoded.end());
}

void DESFireTransactionMACInput::padToBlock(ByteVector &data)
{
    const auto remainder = data.size() % BLOCK_SIZE;
    if (remainder != 0)
        data.insert(data.end(), BLOCK_SIZE - remainder, 0x00);
}

void DESFireTransactionMACInput::appendAligned(const ByteVector &update)
{
    EXCEPTION_ASSERT_WITH_LOG(!update.empty() && update.size() % BLOCK_SIZE == 0,
        LibLogicalAccessException, "DESFire TMI : Update must be non-empty and block-aligned.");

    EXCEPTION_ASSERT_WITH_LOG(update.size() <= (std::numeric_limits<std::size_t>::max)() - data_.size(),
        LibLogicalAccessException, "DESFire TMI : Accumulated input size overflow.");

    data_.insert(data_.end(), update.begin(), update.end());
}

} // namespace logicalaccess
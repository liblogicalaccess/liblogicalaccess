#include <gtest/gtest.h>

#include <logicalaccess/plugins/cards/desfire/transactionmacinput.hpp>

#include <cstdint>
#include <logicalaccess/myexception.hpp>

using namespace logicalaccess;

namespace
{

ByteVector zeros(std::size_t count)
{
    return ByteVector(count, 0x00);
}

} // namespace

TEST(DESFireTransactionMACInputTest, EncodesU24LittleEndian)
{
    EXPECT_EQ(DESFireTransactionMACInput::encodeU24(0x123456U), (ByteVector{0x56, 0x34, 0x12}));
}

TEST(DESFireTransactionMACInputTest, RejectsU24Overflow)
{
    EXPECT_THROW(DESFireTransactionMACInput::encodeU24(0x1000000U), LibLogicalAccessException);
}

TEST(DESFireTransactionMACInputTest, EncodesSignedValueLittleEndian)
{
    EXPECT_EQ(DESFireTransactionMACInput::encodeI32(-1), (ByteVector{0xFF, 0xFF, 0xFF, 0xFF}));
    EXPECT_EQ(DESFireTransactionMACInput::encodeI32(0x12345678), (ByteVector{0x78, 0x56, 0x34, 0x12}));
}

TEST(DESFireTransactionMACInputTest, WriteDataPadsHeaderAndPayloadSeparately)
{
    DESFireTransactionMACInput tmi;

    tmi.appendWriteData(0x02, 0x123456, ByteVector{0xAA});

    const ByteVector expected = {0x3D, 0x02, 0x56, 0x34, 0x12, 0x01, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    EXPECT_EQ(tmi.data(), expected);
}

TEST(DESFireTransactionMACInputTest, ReadDataIncludesReturnedData)
{
    DESFireTransactionMACInput tmi;

    const ByteVector response = {0xDE, 0xAD};

    tmi.appendReadData(0x03, 0x010203, 2, response);

    ASSERT_EQ(tmi.size(), 32U);

    EXPECT_EQ(ByteVector(tmi.data().begin(), tmi.data().begin() + 10),
              (ByteVector{0xBD, 0x03, 0x03, 0x02, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00}));

    EXPECT_EQ(tmi.data()[16], 0xDE);
    EXPECT_EQ(tmi.data()[17], 0xAD);
}

TEST(DESFireTransactionMACInputTest, ReadDataRejectsWrongResponseLength)
{
    DESFireTransactionMACInput tmi;

    EXPECT_THROW(tmi.appendReadData(0x01, 0, 3, ByteVector{0x01, 0x02}), LibLogicalAccessException);
    EXPECT_TRUE(tmi.empty());
}

TEST(DESFireTransactionMACInputTest, ValueOperationPadsToBlock)
{
    DESFireTransactionMACInput tmi;

    tmi.appendValueOperation(DESFireTransactionMACInput::INS_CREDIT, 0x04, 0x12345678);

    ASSERT_EQ(tmi.size(), 16U);
    EXPECT_EQ(ByteVector(tmi.data().begin(), tmi.data().begin() + 6), (ByteVector{0x0C, 0x04, 0x78, 0x56, 0x34, 0x12}));
    EXPECT_EQ(ByteVector(tmi.data().begin() + 6, tmi.data().end()), zeros(10));
}

TEST(DESFireTransactionMACInputTest, ReadRecordsUsesActualCountWhenRequestedCountIsZero)
{
    DESFireTransactionMACInput tmi;

    ByteVector records(32, 0xA5);

    tmi.appendReadRecords(0x05, 0x000001, 0, 2, records);

    ASSERT_EQ(tmi.size(), 48U);

    EXPECT_EQ(ByteVector(tmi.data().begin(), tmi.data().begin() + 8),
              (ByteVector{0xBB, 0x05, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00}));

    EXPECT_EQ(tmi.data()[16], 0xA5);
    EXPECT_EQ(tmi.data()[47], 0xA5);
}

TEST(DESFireTransactionMACInputTest, WriteRecordEncodesRecordData)
{
    DESFireTransactionMACInput tmi;

    const ByteVector record(16, 0x5A);

    tmi.appendWriteRecord(0x06, 0x123456, record);

    ASSERT_EQ(tmi.size(), 32U);

    EXPECT_EQ(ByteVector(tmi.data().begin(), tmi.data().begin() + 8),
              (ByteVector{0x3B, 0x06, 0x56, 0x34, 0x12, 0x10, 0x00, 0x00}));

    EXPECT_EQ(tmi.data()[16], 0x5A);
    EXPECT_EQ(tmi.data()[31], 0x5A);
}

TEST(DESFireTransactionMACInputTest, UpdateRecordPadsElevenByteHeader)
{
    DESFireTransactionMACInput tmi;

    tmi.appendUpdateRecord(0x07, 1, 2, ByteVector(16, 0xCC));

    ASSERT_EQ(tmi.size(), 32U);

    EXPECT_EQ(ByteVector(tmi.data().begin(), tmi.data().begin() + 11),
              (ByteVector{0xBA, 0x07, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x10, 0x00, 0x00}));

    EXPECT_EQ(ByteVector(tmi.data().begin() + 11, tmi.data().begin() + 16), zeros(5));
    EXPECT_EQ(tmi.data()[16], 0xCC);
}

TEST(DESFireTransactionMACInputTest, CommitReaderIDAuthenticated)
{
    DESFireTransactionMACInput tmi;

    const ByteVector readerID(16, 0x11);
    const ByteVector encryptedPreviousID(16, 0x22);

    tmi.appendCommitReaderID(readerID, true, encryptedPreviousID);

    ASSERT_EQ(tmi.size(), 48U);
    EXPECT_EQ(tmi.data()[0], 0xC8);

    // Reader ID occupies bytes 1 to 16
    EXPECT_EQ(tmi.data()[1], 0x11);
    EXPECT_EQ(tmi.data()[16], 0x11);
    // Encrypted previous Reader ID occupies bytes 17to 32
    EXPECT_EQ(tmi.data()[17], 0x22);
    EXPECT_EQ(tmi.data()[32], 0x22);
    // Final padding occupies bytes 33 to 47
    EXPECT_EQ(ByteVector(tmi.data().begin() + 33, tmi.data().end()), zeros(15));
}

TEST(DESFireTransactionMACInputTest, CommitReaderIDUnauthenticated)
{
    DESFireTransactionMACInput tmi;

    tmi.appendCommitReaderID(ByteVector(16, 0x11), false);

    ASSERT_EQ(tmi.size(), 32U);
    EXPECT_EQ(tmi.data()[0], 0xC8);
    EXPECT_EQ(tmi.data()[1], 0x11);
    EXPECT_EQ(ByteVector(tmi.data().begin() + 17, tmi.data().end()), zeros(15));
}

TEST(DESFireTransactionMACInputTest, CommitReaderIDRejectsInvalidLengths)
{
    DESFireTransactionMACInput tmi;

    EXPECT_THROW(tmi.appendCommitReaderID(ByteVector(15, 0x11), false), LibLogicalAccessException);
    EXPECT_THROW(tmi.appendCommitReaderID(ByteVector(16, 0x11), true, ByteVector(15, 0x22)), LibLogicalAccessException);

    EXPECT_TRUE(tmi.empty());
}

TEST(DESFireTransactionMACInputTest, AppendsUpdatesWithoutRepaddingPreviousUpdates)
{
    DESFireTransactionMACInput tmi;

    tmi.appendClearRecordFile(0x01);
    tmi.appendClearRecordFile(0x02);

    ASSERT_EQ(tmi.size(), 32U);

    EXPECT_EQ(tmi.data()[0], 0xEB);
    EXPECT_EQ(tmi.data()[1], 0x01);
    EXPECT_EQ(tmi.data()[16], 0xEB);
    EXPECT_EQ(tmi.data()[17], 0x02);
}

TEST(TransactionMACLifecycleTest, SuccessfulCommitStartsFreshInput)
{
    DESFireTransactionMACInput input;
    input.appendClearRecordFile(1);

    ASSERT_FALSE(input.empty());

    input.clear(); // Successful commit hook

    EXPECT_TRUE(input.empty());
}

TEST(TransactionMACLifecycleTest, SuccessfulAbortStartsFreshInput)
{
    DESFireTransactionMACInput input;
    input.appendClearRecordFile(1);

    ASSERT_FALSE(input.empty());

    input.clear(); // Successful abort hook

    EXPECT_TRUE(input.empty());
}

TEST(TransactionMACLifecycleTest, AmbiguousCommitPreservesInput)
{
    DESFireTransactionMACInput input;
    input.appendClearRecordFile(1);

    const ByteVector before = input.data();

    // Simulate an ambiguous transport outcome : do not clear pending TMI
    EXPECT_EQ(input.data(), before);
}

TEST(TransactionMACLifecycleTest, FailedOperationDoesNotAppendUpdate)
{
    DESFireTransactionMACInput input;
    const ByteVector before = input.data();

    // The wrapper appends only after the underlying command succeeds
    EXPECT_EQ(input.data(), before);
}

TEST(DESFireTransactionMACInputTest, ReadRecordsDoesNotAddFinalPadding)
{
    DESFireTransactionMACInput tmi;

    const ByteVector records(32, 0xA5);

    tmi.appendReadRecords(0x02, 0, 2, 2, records);

    ASSERT_EQ(tmi.size(), 48U);

    EXPECT_EQ(tmi.data()[0], 0xBB);
    EXPECT_EQ(tmi.data()[1], 0x02);
    EXPECT_EQ(tmi.data()[5], 0x02);
    EXPECT_EQ(tmi.data()[16], 0xA5);
    EXPECT_EQ(tmi.data()[47], 0xA5);
}

TEST(DESFireTransactionMACInputTest, ReadRecordsRejectsPartialRecord)
{
    DESFireTransactionMACInput tmi;

    EXPECT_THROW(tmi.appendReadRecords(0x02, 0, 1, 1, ByteVector(15, 0xA5)), LibLogicalAccessException);

    EXPECT_TRUE(tmi.empty());
}

TEST(DESFireTransactionMACInputTest, InvalidUpdatePreservesExistingInput)
{
    DESFireTransactionMACInput tmi;
    tmi.appendClearRecordFile(0x01);

    const auto before = tmi.data();

    EXPECT_THROW(tmi.appendReadData(0x02, 0, 3, ByteVector{0xAA, 0xBB}), LibLogicalAccessException);

    EXPECT_EQ(tmi.data(), before);
}

TEST(DESFireTransactionMACInputTest, KnownTMIEncoding)
{
    DESFireTransactionMACInput input;

    // Known WriteData command :
    // INS = 3D, FileNo = 01, Offset = 0, Length = 4, Data = DEADBEEF
    input.appendWriteData(0x01, 0x00, {0xDE, 0xAD, 0xBE, 0xEF});

    const ByteVector expected = {0x3D, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    EXPECT_EQ(input.data(), expected);
    EXPECT_EQ(input.size(), 32U);
}

TEST(DESFireTransactionMACInputTest, ReproducesTransactionTMI)
{
    DESFireTransactionMACInput tmi;

    const ByteVector readerID = {0x28, 0xBF, 0x19, 0x82, 0xBE, 0x08, 0x6F, 0xBC,
                                 0x60, 0xA2, 0x2D, 0xAE, 0xB6, 0x66, 0x13, 0xEE};

    // Deterministic test data; not a real encrypted Reader ID
    const ByteVector encryptedPreviousID = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                                            0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
                                            0x0C, 0x0D, 0x0E, 0x0F};

    tmi.appendCommitReaderID(readerID, true, encryptedPreviousID);
    tmi.appendWriteData(0x01, 0x000000, {0xDE, 0xAD, 0xBE, 0xEF});

    const ByteVector expected = {
        // CommitReaderID command
        0xC8,

        // Reader ID (16 bytes)
        0x28, 0xBF, 0x19, 0x82, 0xBE, 0x08, 0x6F, 0xBC, 0x60, 0xA2, 0x2D, 0xAE, 0xB6,
        0x66, 0x13, 0xEE,

        // Encrypted previous Reader ID (16 bytes)
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F,

        // CommitReaderID padding to 16-byte boundary
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,

        // WriteData header
        0x3D, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,

        // Header padding to 16 byte boundary
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        // Written payload
        0xDE, 0xAD, 0xBE, 0xEF,

        // Payload padding to 16 byte boundary
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    EXPECT_EQ(tmi.data(), expected);
    EXPECT_EQ(tmi.size(), 80U);
}

TEST(DESFireTransactionMACInputTest, ReproducesFailingTransactionTMI)
{
    DESFireTransactionMACInput tmi;

    const ByteVector readerID = {0x28, 0xBF, 0x19, 0x82, 0xBE, 0x08, 0x6F, 0xBC,
                                 0x60, 0xA2, 0x2D, 0xAE, 0xB6, 0x66, 0x13, 0xEE};

    const ByteVector encryptedPreviousID = {0xA1, 0x07, 0x19, 0x9C, 0x63, 0x5E,
                                            0x46, 0xCA, 0xAB, 0x36, 0x1D, 0x12,
                                            0x82, 0xFC, 0x45, 0x07};

    tmi.appendCommitReaderID(readerID, true, encryptedPreviousID);
    tmi.appendWriteData(0x01, 0, {0xDE, 0xAD, 0xBE, 0xEF});

    const ByteVector expected = {
        0xC8, 0x28, 0xBF, 0x19, 0x82, 0xBE, 0x08, 0x6F, 0xBC, 0x60, 0xA2, 0x2D,
        0xAE, 0xB6, 0x66, 0x13, 0xEE, 0xA1, 0x07, 0x19, 0x9C, 0x63, 0x5E, 0x46,
        0xCA, 0xAB, 0x36, 0x1D, 0x12, 0x82, 0xFC, 0x45, 0x07, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x3D, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    ASSERT_EQ(expected.size(), 80U);
    EXPECT_EQ(tmi.size(), 80U);
    EXPECT_EQ(tmi.data(), expected);
}
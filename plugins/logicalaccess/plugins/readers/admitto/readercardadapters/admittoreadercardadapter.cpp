/**
 * \file admittoreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Admitto reader/card adapter.
 */

#include <logicalaccess/plugins/readers/admitto/readercardadapters/admittoreadercardadapter.hpp>

namespace logicalaccess
{
const unsigned char AdmittoReaderCardAdapter::CR = 0x0d;
const unsigned char AdmittoReaderCardAdapter::LF = 0x0a;

AdmittoReaderCardAdapter::AdmittoReaderCardAdapter()
    : ReaderCardAdapter()
{
}

AdmittoReaderCardAdapter::~AdmittoReaderCardAdapter()
{
}

ByteVector AdmittoReaderCardAdapter::adaptCommand(const ByteVector &command)
{
    return command;
}

ByteVector AdmittoReaderCardAdapter::adaptAnswer(const ByteVector &answer)
{
    EXCEPTION_ASSERT_WITH_LOG(
        answer.size() >= 2, std::invalid_argument,
        "A valid command buffer size must be at least 2 bytes long");
    EXCEPTION_ASSERT_WITH_LOG(
        answer[answer.size() - 2] == CR, std::invalid_argument,
        "The supplied command buffer is not valid (bad first stop byte)");
    EXCEPTION_ASSERT_WITH_LOG(
        answer[answer.size() - 1] == LF, std::invalid_argument,
        "The supplied command buffer is not valid (bad second stop byte)");

    return ByteVector(answer.begin() + 2, answer.end() - 2);
}
}
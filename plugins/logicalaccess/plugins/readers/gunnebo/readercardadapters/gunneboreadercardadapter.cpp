/**
 * \file gunneboreadercardadapter.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Gunnebo reader/card adapter.
 */

#include <logicalaccess/plugins/readers/gunnebo/readercardadapters/gunneboreadercardadapter.hpp>

namespace logicalaccess
{
const unsigned char GunneboReaderCardAdapter::STX = 0x02;
const unsigned char GunneboReaderCardAdapter::ETX = 0x03;

GunneboReaderCardAdapter::GunneboReaderCardAdapter()
    : ReaderCardAdapter()
{
}

GunneboReaderCardAdapter::~GunneboReaderCardAdapter()
{
}

ByteVector GunneboReaderCardAdapter::adaptCommand(const ByteVector &command)
{
    return command;
}

ByteVector GunneboReaderCardAdapter::adaptAnswer(const ByteVector &answer)
{
    LOG(LogLevel::COMS) << "Processing the received command buffer "
                        << BufferHelper::getHex(answer) << " command size {"
                        << answer.size() << "}...";

    std::shared_ptr<GunneboReaderUnit> ru =
        std::dynamic_pointer_cast<GunneboReaderUnit>(getDataTransport()->getReaderUnit());

    EXCEPTION_ASSERT_WITH_LOG(
        answer.size() >= 3, std::invalid_argument,
        "A valid command buffer size must be at least 3 bytes long");
    EXCEPTION_ASSERT_WITH_LOG(answer[0] == STX, std::invalid_argument,
                              "The supplied command buffer is not valid (bad STX)");
    size_t foolen = 2;
    if (answer[1] == 0x31 && answer[2] == 0x46)
    {
        foolen = 1;
    }
    EXCEPTION_ASSERT_WITH_LOG(answer[answer.size() - foolen] == ETX,
                              std::invalid_argument,
                              "The supplied command buffer is not valid (bad ETX)");

    // Gunnebo ID is like that: [STX - 1 byte][id - x bytes][ETX - 1 byte][checksum - 1
    // byte]
    // We don't calculate the checksum ! Who cares ?!? :)

    return ByteVector(answer.begin() + 1, answer.end() - 2);
}
}
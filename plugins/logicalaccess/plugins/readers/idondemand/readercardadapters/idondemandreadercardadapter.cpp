/**
 * \file idondemandreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief IdOnDemand reader/card adapter.
 */

#include <logicalaccess/plugins/readers/idondemand/readercardadapters/idondemandreadercardadapter.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
IdOnDemandReaderCardAdapter::IdOnDemandReaderCardAdapter()
    : ReaderCardAdapter()
{
}

IdOnDemandReaderCardAdapter::~IdOnDemandReaderCardAdapter()
{
}

ByteVector IdOnDemandReaderCardAdapter::adaptCommand(const ByteVector &command)
{
    LOG(LogLevel::COMS) << "Sending command with command "
                        << BufferHelper::getHex(command) << " command size {"
                        << command.size() << "} {" << BufferHelper::getStdString(command)
                        << "}...";
    ByteVector cmd = command;
    cmd.push_back(0x0d);

    return cmd;
}

ByteVector IdOnDemandReaderCardAdapter::adaptAnswer(const ByteVector &answer)
{
    LOG(LogLevel::COMS) << "Processing the received command buffer "
                        << BufferHelper::getHex(answer) << " command size {"
                        << answer.size() << "} {" << BufferHelper::getStdString(answer)
                        << "}...";

    ByteVector ret;
    EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 2, std::invalid_argument,
                              "A valid buffer size must be at least 2 bytes long");

    std::string strcmdbuf = BufferHelper::getStdString(answer);

    EXCEPTION_ASSERT_WITH_LOG(strcmdbuf.find("ERROR ") == std::string::npos,
                              CardException, "ERROR code returned.");
    EXCEPTION_ASSERT_WITH_LOG(strcmdbuf.find("OK ") != std::string::npos,
                              std::invalid_argument,
                              "The command doesn't return OK code.");

    return ret;
}
}
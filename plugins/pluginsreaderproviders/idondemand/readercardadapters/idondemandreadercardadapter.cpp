/**
 * \file idondemandreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief IdOnDemand reader/card adapter.
 */

#include "idondemandreadercardadapter.hpp"

namespace logicalaccess
{
    IdOnDemandReaderCardAdapter::IdOnDemandReaderCardAdapter()
        : ReaderCardAdapter()
    {
    }

    IdOnDemandReaderCardAdapter::~IdOnDemandReaderCardAdapter()
    {
    }

    std::vector<unsigned char> IdOnDemandReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
    {
        LOG(LogLevel::COMS) << "Sending command with command " << BufferHelper::getHex(command) << " command size {" << command.size() << "} {" << BufferHelper::getStdString(command) << "}...";
        std::vector<unsigned char> cmd = command;
        cmd.push_back(0x0d);

        return cmd;
    }

    std::vector<unsigned char> IdOnDemandReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
    {
        LOG(LogLevel::COMS) << "Processing the received command buffer " << BufferHelper::getHex(answer) << " command size {" << answer.size() << "} {" << BufferHelper::getStdString(answer) << "}...";

        std::vector<unsigned char> ret;
        EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 2, std::invalid_argument, "A valid buffer size must be at least 2 bytes long");

        std::string strcmdbuf = BufferHelper::getStdString(answer);

        EXCEPTION_ASSERT_WITH_LOG(strcmdbuf.find("ERROR ") == std::string::npos, CardException, "ERROR code returned.");
        EXCEPTION_ASSERT_WITH_LOG(strcmdbuf.find("OK ") != std::string::npos, std::invalid_argument, "The command doesn't return OK code.");

        return ret;
    }
}
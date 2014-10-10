/**
 * \file ok5553readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief OK5553 reader/card adapter.
 */

#include "ok5553readercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{
    OK5553ReaderCardAdapter::OK5553ReaderCardAdapter()
        : ReaderCardAdapter()
    {
    }

    OK5553ReaderCardAdapter::~OK5553ReaderCardAdapter()
    {
    }

    std::vector<unsigned char> OK5553ReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
    {
        return command;
    }

    std::vector<unsigned char> OK5553ReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
    {
        LOG(LogLevel::COMS) << "Processing response : " << BufferHelper::getHex(answer);
        if (answer.size() == 1)
        {
            EXCEPTION_ASSERT_WITH_LOG(answer[0] != 'N', LibLogicalAccessException, "No tag present in rfid field.");
            EXCEPTION_ASSERT_WITH_LOG(answer[0] != 'F', LibLogicalAccessException, "Fail to execute the command.");
        }
        else if (answer.size() >= 5)
        {
            EXCEPTION_ASSERT_WITH_LOG(answer[0] != 'M' || answer[1] != 'i' || answer[2] != 'f' || answer[3] != 'a' || answer[4] != 'r', LibLogicalAccessException, "The reader as reboot.");
        }

        return answer;
    }

    std::vector<unsigned char> OK5553ReaderCardAdapter::sendAsciiCommand(const std::string& command, long int timeout)
    {
        std::vector<unsigned char> cmd;
        cmd.insert(cmd.end(), command.begin(), command.end());
        return sendCommand(cmd, timeout);
    }
}
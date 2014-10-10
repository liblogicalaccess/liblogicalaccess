/**
 * \file readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader/Card adapter base class.
 */

#include "logicalaccess/cards/readercardadapter.hpp"

namespace logicalaccess
{
    std::vector<unsigned char> ReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
    {
        return command;
    }

    std::vector<unsigned char> ReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
    {
        return answer;
    }

    std::vector<unsigned char> ReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long timeout)
    {
        std::vector<unsigned char> res;

        if (d_dataTransport)
        {
            res = adaptAnswer(d_dataTransport->sendCommand(adaptCommand(command), timeout));
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Cannot transmit the command, data transport is not set!";
        }

        return res;
    }
}
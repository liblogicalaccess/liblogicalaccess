/**
 * \file datatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Data transport base class.
 */

#include "logicalaccess/readerproviders/datatransport.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/logs.hpp"
#include "logicalaccess/settings.hpp"

namespace logicalaccess
{
    std::vector<unsigned char> DataTransport::sendCommand(const std::vector<unsigned char>& command, long int timeout)
    {
        if (timeout == -1)
            timeout = Settings::getInstance()->DataTransportTimeout;

        LOG(LogLevel::COMS) << "Sending command " << BufferHelper::getHex(command) << " command size {" << command.size() << "} timeout {" << timeout << "}...";

        std::vector<unsigned char> res;
        d_lastCommand = command;
        d_lastResult.clear();

        if (command.size() > 0)
        {
            connect();

            send(command);
        }

        res = receive(timeout);
		d_lastResult = res;

        LOG(LogLevel::COMS) << "Response received successfully ! Response: " << BufferHelper::getHex(res) << " size {" << res.size() << "}";
        return res;
    }
}
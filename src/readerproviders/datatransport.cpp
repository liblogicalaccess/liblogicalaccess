/**
 * \file datatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Data transport base class.
 */

#include <logicalaccess/readerproviders/datatransport.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>

namespace logicalaccess
{
ByteVector DataTransport::sendCommand(const ByteVector &command, long int timeout)
{
    if (timeout == -1)
        timeout = Settings::getInstance()->DataTransportTimeout;

    LOG(LogLevel::COMS) << "Sending command " << BufferHelper::getHex(command)
                        << " command size {" << command.size() << "} timeout {" << timeout
                        << "}...";

    d_lastCommand = command;
    d_lastResult.clear();

    if (command.size() > 0)
    {
        connect();

        send(command);
    }

    ByteVector res = receive(timeout);
    d_lastResult   = res;

    LOG(LogLevel::COMS) << "Response received successfully ! Response: "
                        << BufferHelper::getHex(res) << " size {" << res.size() << "}";
    return res;
}
}
/**
 * \file libusbreadercardadapter.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief LibUSB reader/card adapter.
 */

#include <logicalaccess/plugins/readers/libusb/readercardadapters/libusbreadercardadapter.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderunit.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbdatatransport.hpp>

namespace logicalaccess
{
LibUSBReaderCardAdapter::LibUSBReaderCardAdapter()
{
    d_dataTransport.reset(new LibUSBDataTransport());
}

LibUSBReaderCardAdapter::~LibUSBReaderCardAdapter()
{
}

std::vector<unsigned char> LibUSBReaderCardAdapter::adaptCommand(const std::vector<unsigned char> &command)
{
    return command;
}

std::vector<unsigned char> LibUSBReaderCardAdapter::adaptAnswer(const std::vector<unsigned char> &answer)
{
    return answer;
}

std::vector<unsigned char> LibUSBReaderCardAdapter::sendCommand(const std::vector<unsigned char> &command, long timeout)
{
    std::vector<unsigned char> res;

    if (d_dataTransport)
    {
        res = adaptAnswer(d_dataTransport->sendCommand(adaptCommand(command), timeout));

        if (res.size() > 0 && getResultChecker())
        {
            LOG(LogLevel::DEBUGS) << "Call ResultChecker..." << BufferHelper::getHex(res);
            getResultChecker()->CheckResult(&res[0], res.size());
        }
    }
    else
    {
        LOG(LogLevel::ERRORS)
            << "Cannot transmit the command, data transport is not set!";
    }
    return res;
}
}
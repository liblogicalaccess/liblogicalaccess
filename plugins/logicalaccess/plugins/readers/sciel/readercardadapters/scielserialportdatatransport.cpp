/**
 * \file scieldatatransport.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Sciel buffer parser.
 */

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/readers/sciel/readercardadapters/scielserialportdatatransport.hpp>
#include <logicalaccess/bufferhelper.hpp>

namespace logicalaccess
{
ByteVector ScielSerialPortDataTransport::checkValideBufferAvailable()
{
    ByteVector ret;

    d_port->getSerialPort()->lockedExecute([&]() {
        if (d_port->getSerialPort()->getCircularBufferParser())
            ret = d_port->getSerialPort()->getCircularBufferParser()->getValidBuffer(
                d_port->getSerialPort()->getCircularReadBuffer());
    });
    LOG(LogLevel::COMS) << "checkValideBufferAvailable: " << BufferHelper::getHex(ret);
    return ret;
}
}

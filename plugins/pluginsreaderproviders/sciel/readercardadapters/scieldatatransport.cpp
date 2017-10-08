/**
 * \file scieldatatransport.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Sciel buffer parser.
 */

#include <logicalaccess/logs.hpp>
#include "scieldatatransport.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{
    ByteVector ScielDataTransport::checkValideBufferAvailable()
    {
        ByteVector ret;

        d_port->getSerialPort()->lockedExecute([&](){
            if (d_port->getSerialPort()->getCircularBufferParser())
                ret = d_port->getSerialPort()->getCircularBufferParser()->getValidBuffer(d_port->getSerialPort()->getCircularReadBuffer());
        });
        LOG(LogLevel::COMS) << "checkValideBufferAvailable: " << BufferHelper::getHex(ret);
        return ret;
    }
}

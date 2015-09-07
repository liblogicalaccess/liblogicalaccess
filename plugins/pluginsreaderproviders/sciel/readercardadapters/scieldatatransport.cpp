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
    std::vector<unsigned char> ScielDataTransport::checkValideBufferAvailable()
    {
        std::vector<unsigned char> ret;

        d_port->getSerialPort()->lockedExecute([&](){
            if (d_port->getSerialPort()->getCircularBufferParser())
                ret = d_port->getSerialPort()->getCircularBufferParser()->getValidBuffer(d_port->getSerialPort()->getCircularReadBuffer());
        });
        LOG(LogLevel::COMS) << "checkValideBufferAvailable: " << BufferHelper::getHex(ret);
        return ret;
    }
}

/**
 * \file scieldatatransport.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Sciel buffer parser.
 */

#include "scieldatatransport.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{		
	std::vector<unsigned char> ScielDataTransport::checkValideBufferAvailable()
    {
        std::vector<unsigned char> ret;

        d_port->getSerialPort()->getReadMutex().lock();
        if (d_port->getSerialPort()->getCircularBufferParser())
            ret = d_port->getSerialPort()->getCircularBufferParser()->getValidBuffer(d_port->getSerialPort()->getCircularReadBuffer());
        d_port->getSerialPort()->getReadMutex().unlock();
        LOG(LogLevel::COMS) << "checkValideBufferAvailable: " << BufferHelper::getHex(ret);
        return ret;
    }
}

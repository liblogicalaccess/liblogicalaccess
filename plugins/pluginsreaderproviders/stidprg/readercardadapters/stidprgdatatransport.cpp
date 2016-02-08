#include "stidprgdatatransport.hpp"
#include "stidprgbufferparser.hpp"

using namespace logicalaccess;

void STidPRGDataTransport::setSerialPort(std::shared_ptr<SerialPortXml> port)
{
    d_port = port;
    port->getSerialPort()->setCircularBufferParser(new STidPRGBufferParser());
}

std::vector<unsigned char> STidPRGDataTransport::receive(long int timeout)
{
    return SerialPortDataTransport::receive(receiveTimeout_);
}

STidPRGDataTransport::STidPRGDataTransport()
    : receiveTimeout_(3000)
{
}

void STidPRGDataTransport::setReceiveTimeout(long int t)
{
    receiveTimeout_ = t;
}

long int STidPRGDataTransport::getReceiveTimeout() const
{
    return receiveTimeout_;
}

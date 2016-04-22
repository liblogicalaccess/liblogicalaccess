#include "stidprgdatatransport.hpp"
#include "stidprgbufferparser.hpp"
#include "logicalaccess/logs.hpp"
#include <boost/property_tree/ptree.hpp>

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

void STidPRGDataTransport::unSerialize(boost::property_tree::ptree& node)
{
    LOG(INFOS) << "Will unserialize STIDPRGDataTransport.";
    SerialPortDataTransport::unSerialize(node.get_child(SerialPortDataTransport::getDefaultXmlNodeName()));
    d_port->getSerialPort()->setCircularBufferParser(new STidPRGBufferParser());
}

void STidPRGDataTransport::serialize(boost::property_tree::ptree& parentNode)
{
    boost::property_tree::ptree node;	
    SerialPortDataTransport::serialize(node); 
    parentNode.add_child(getDefaultXmlNodeName(), node);
}
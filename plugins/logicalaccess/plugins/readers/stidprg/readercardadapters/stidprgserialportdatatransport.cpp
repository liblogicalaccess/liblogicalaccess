#include <logicalaccess/plugins/readers/stidprg/readercardadapters/stidprgserialportdatatransport.hpp>
#include <logicalaccess/plugins/readers/stidprg/readercardadapters/stidprgbufferparser.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace logicalaccess;

void STidPRGSerialPortDataTransport::setSerialPort(std::shared_ptr<SerialPortXml> port)
{
    d_port = port;
    port->getSerialPort()->setCircularBufferParser(new STidPRGBufferParser());
}

ByteVector STidPRGSerialPortDataTransport::receive(long int /*timeout*/)
{
    return SerialPortDataTransport::receive(receiveTimeout_);
}

STidPRGSerialPortDataTransport::STidPRGSerialPortDataTransport()
    : receiveTimeout_(3000)
{
}

void STidPRGSerialPortDataTransport::setReceiveTimeout(long int t)
{
    receiveTimeout_ = t;
}

long int STidPRGSerialPortDataTransport::getReceiveTimeout() const
{
    return receiveTimeout_;
}

void STidPRGSerialPortDataTransport::unSerialize(boost::property_tree::ptree &node)
{
    LOG(INFOS) << "Will unserialize STIDPRGDataTransport.";
    SerialPortDataTransport::unSerialize(
        node.get_child(SerialPortDataTransport::getDefaultXmlNodeName()));
    d_port->getSerialPort()->setCircularBufferParser(new STidPRGBufferParser());
}

void STidPRGSerialPortDataTransport::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    SerialPortDataTransport::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}
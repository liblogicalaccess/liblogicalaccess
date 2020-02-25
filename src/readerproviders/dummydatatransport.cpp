#include <logicalaccess/readerproviders/dummydatatransport.hpp>

namespace logicalaccess {

std::string DummyDataTransport::getTransportType() const
{
    throw std::runtime_error("Not implemented");
}
ByteVector DummyDataTransport::receive(long int timeout)
{
    throw std::runtime_error("Not implemented");
}
std::string DummyDataTransport::getDefaultXmlNodeName() const
{
    throw std::runtime_error("Not implemented");
}
bool DummyDataTransport::connect()
{
    throw std::runtime_error("Not implemented");
}
void DummyDataTransport::disconnect()
{
    throw std::runtime_error("Not implemented");
}
bool DummyDataTransport::isConnected()
{
    throw std::runtime_error("Not implemented");
}
std::string DummyDataTransport::getName() const
{
    throw std::runtime_error("Not implemented");
}
void DummyDataTransport::serialize(boost::property_tree::ptree &parentNode)
{
    throw std::runtime_error("Not implemented");
}
void DummyDataTransport::unSerialize(boost::property_tree::ptree &node)
{
    throw std::runtime_error("Not implemented");
}
void DummyDataTransport::send(const ByteVector &data)
{
    throw std::runtime_error("Not implemented");
}
}

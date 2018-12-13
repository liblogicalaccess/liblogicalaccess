/**
 * \file udpdatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief UDP data transport.
 */

#include <logicalaccess/readerproviders/udpdatatransport.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/boost_version_types.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>
#include <boost/property_tree/ptree.hpp>
#include <thread>

namespace logicalaccess
{
UDPDataTransport::UDPDataTransport()
    : d_ipAddress("127.0.0.1")
    , d_port(9559)
{
}

UDPDataTransport::~UDPDataTransport()
{
}

std::string UDPDataTransport::getIpAddress() const
{
    return d_ipAddress;
}

void UDPDataTransport::setIpAddress(std::string ipAddress)
{
    d_ipAddress = ipAddress;
}

int UDPDataTransport::getPort() const
{
    return d_port;
}

void UDPDataTransport::setPort(int port)
{
    d_port = port;
}

std::shared_ptr<boost::asio::ip::udp::socket> UDPDataTransport::getSocket() const
{
    return d_socket;
}

bool UDPDataTransport::connect()
{
    if (!d_socket)
    {
        boost::asio::ip::udp::endpoint endpoint(BOOST_ASIO_MAKE_ADDRESS(getIpAddress()),
                                                getPort());
        d_socket.reset(new boost::asio::ip::udp::socket(ios));

        try
        {
            d_socket->connect(endpoint);
        }
        catch (boost::system::system_error &)
        {
            d_socket.reset();
        }
    }

    return bool(d_socket);
}

void UDPDataTransport::disconnect()
{
    if (d_socket)
    {
        d_socket->close();
        d_socket.reset();
    }
}

bool UDPDataTransport::isConnected()
{
    return bool(d_socket);
}

std::string UDPDataTransport::getName() const
{
    return d_ipAddress;
}

void UDPDataTransport::send(const ByteVector &data)
{
    if (data.size() > 0)
    {
        std::shared_ptr<boost::asio::ip::udp::socket> socket = getSocket();
        socket->send(boost::asio::buffer(data));
    }
}

ByteVector UDPDataTransport::receive(long int timeout)
{
    ByteVector res;
    std::shared_ptr<boost::asio::ip::udp::socket> socket = getSocket();

    long int currentWait = 0;
    size_t lenav         = socket->available();
    while (lenav == 0 && (timeout == 0 || currentWait < timeout))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        currentWait += 250;

        lenav = socket->available();
    }

    if (lenav > 0)
    {
        boost::array<char, 128> recv_buf;
        boost::asio::ip::udp::endpoint sender_endpoint;
        // TODO: Need to set up a timeout here !
        size_t len = socket->receive_from(boost::asio::buffer(recv_buf), sender_endpoint);
        res        = ByteVector(recv_buf.begin(), recv_buf.begin() + len);
    }

    return res;
}

void UDPDataTransport::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getTransportType());
    node.put("IpAddress", d_ipAddress);
    node.put("Port", d_port);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void UDPDataTransport::unSerialize(boost::property_tree::ptree &node)
{
    d_ipAddress = node.get_child("IpAddress").get_value<std::string>();
    d_port      = node.get_child("Port").get_value<int>();
}

std::string UDPDataTransport::getDefaultXmlNodeName() const
{
    return "UdpDataTransport";
}
}
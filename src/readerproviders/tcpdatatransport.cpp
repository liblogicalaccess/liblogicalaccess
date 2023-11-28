/**
 * \file tcpdatatransport.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief TCP data transport.
 */

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/readerproviders/tcpdatatransport.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/boost_version_types.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>

namespace logicalaccess
{
TCPDataTransport::TCPDataTransport()
    : d_socket(d_ios)
    , d_timer(d_ios)
    , d_read_error(true)
    , d_bytes_transferred(0)
    , d_ipAddress("127.0.0.1")
    , d_port(9559)
{
}

TCPDataTransport::~TCPDataTransport()
{
}

std::string TCPDataTransport::getIpAddress() const
{
    return d_ipAddress;
}

void TCPDataTransport::setIpAddress(std::string ipAddress)
{
    d_ipAddress = ipAddress;
}

int TCPDataTransport::getPort() const
{
    return d_port;
}

void TCPDataTransport::setPort(int port)
{
    d_port = port;
}

bool TCPDataTransport::connect()
{
    return connect(Settings::getInstance()->DataTransportTimeout);
}

bool TCPDataTransport::connect(long int timeout)
{
    if (d_socket.is_open())
        d_socket.close();

    try
    {
        d_ios.reset();
        d_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
        d_timer.async_wait(boost::bind(&TCPDataTransport::time_out, this,
                                       boost::asio::placeholders::error));

        boost::asio::ip::tcp::endpoint endpoint(BOOST_ASIO_MAKE_ADDRESS(getIpAddress()),
                                                getPort());
        d_socket.async_connect(endpoint,
                               boost::bind(&TCPDataTransport::connect_complete, this,
                                           boost::asio::placeholders::error));

        d_ios.run();

        if (d_read_error)
            d_socket.close();

        LOG(LogLevel::INFOS) << "Connected to " << getIpAddress() << " on port "
                             << getPort() << ".";
    }
    catch (boost::system::system_error &ex)
    {
        LOG(LogLevel::ERRORS) << "Cannot establish connection on " << getIpAddress()
                              << ":" << getPort() << " : " << ex.what();
        disconnect();
    }

    return bool(d_socket.is_open());
}

void TCPDataTransport::disconnect()
{
    LOG(LogLevel::INFOS) << getIpAddress() << ":" << getPort() << "Disconnected.";
    d_socket.close();
}

bool TCPDataTransport::isConnected()
{
    return bool(d_socket.is_open());
}

std::string TCPDataTransport::getName() const
{
    return d_ipAddress;
}

void TCPDataTransport::send(const ByteVector &data)
{
    if (data.size() > 0)
    {
        try
        {
            LOG(LogLevel::COMS) << "TCP Send Data: " << BufferHelper::getHex(data);
            d_socket.send(boost::asio::buffer(data));
        }
        catch (boost::system::system_error &ex)
        {
            std::exception_ptr eptr = std::current_exception();
            LOG(LogLevel::ERRORS) << "Cannot send on " << getIpAddress() << ":"
                                  << getPort() << " : " << ex.what();
            disconnect();
            std::rethrow_exception(eptr);
        }
    }
}

void TCPDataTransport::connect_complete(const boost::system::error_code &error)
{
    // 0 is success.
    d_read_error = static_cast<bool>(error.value());
    d_timer.cancel();
}

void TCPDataTransport::read_complete(const boost::system::error_code &error,
                                     size_t bytes_transferred)
{
    d_read_error        = (error || bytes_transferred == 0);
    d_bytes_transferred = bytes_transferred;
    d_timer.cancel();
}

void TCPDataTransport::time_out(const boost::system::error_code &error)
{
    if (error)
        return;
    d_socket.cancel();
}

ByteVector TCPDataTransport::receive(long int timeout)
{
    ByteVector recv(256);
    d_ios.reset();
    d_bytes_transferred = 0;

    d_ios.reset();
    d_socket.async_receive(boost::asio::buffer(recv),
                           boost::bind(&TCPDataTransport::read_complete, this,
                                       boost::asio::placeholders::error,
                                       boost::asio::placeholders::bytes_transferred));

    d_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
    d_timer.async_wait(
        boost::bind(&TCPDataTransport::time_out, this, boost::asio::placeholders::error));

    d_ios.run();

    recv.resize(d_bytes_transferred);
    if (d_read_error || recv.size() == 0)
    {
        char buf[64];
        sprintf(buf, "Socket receive timeout (> %ld milliseconds).", timeout);
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, buf);
    }

    LOG(LogLevel::COMS) << "TCP Data read: " << BufferHelper::getHex(recv);
    return recv;
}

void TCPDataTransport::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getTransportType());
    node.put("IpAddress", d_ipAddress);
    node.put("Port", d_port);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void TCPDataTransport::unSerialize(boost::property_tree::ptree &node)
{
    d_ipAddress = node.get_child("IpAddress").get_value<std::string>();
    d_port      = node.get_child("Port").get_value<int>();
}

std::string TCPDataTransport::getDefaultXmlNodeName() const
{
    return "TcpDataTransport";
}
}

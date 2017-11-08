/**
 * \file tcpdatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief TCP data transport.
 */

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/readerproviders/tcpdatatransport.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/bufferhelper.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>
#include <logicalaccess/logs.hpp>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/settings.hpp>

namespace logicalaccess
{
TcpDataTransport::TcpDataTransport()
    : d_socket(d_ios)
    , d_timer(d_ios)
    , d_read_error(true)
    , d_bytes_transferred(0)
    , d_ipAddress("127.0.0.1")
    , d_port(9559)
{
}

TcpDataTransport::~TcpDataTransport()
{
}

std::string TcpDataTransport::getIpAddress() const
{
    return d_ipAddress;
}

void TcpDataTransport::setIpAddress(std::string ipAddress)
{
    d_ipAddress = ipAddress;
}

int TcpDataTransport::getPort() const
{
    return d_port;
}

void TcpDataTransport::setPort(int port)
{
    d_port = port;
}

bool TcpDataTransport::connect()
{
    return connect(Settings::getInstance()->DataTransportTimeout);
}

bool TcpDataTransport::connect(long int timeout)
{
    if (d_socket.is_open())
        d_socket.close();

    try
    {
        d_ios.reset();
        d_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
        d_timer.async_wait(boost::bind(&TcpDataTransport::time_out, this,
                                       boost::asio::placeholders::error));

        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string(getIpAddress()), getPort());
        d_socket.async_connect(endpoint,
                               boost::bind(&TcpDataTransport::connect_complete, this,
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

void TcpDataTransport::disconnect()
{
    LOG(LogLevel::INFOS) << getIpAddress() << ":" << getPort() << "Disconnected.";
    d_socket.close();
}

bool TcpDataTransport::isConnected()
{
    return bool(d_socket.is_open());
}

std::string TcpDataTransport::getName() const
{
    return d_ipAddress;
}

void TcpDataTransport::send(const ByteVector &data)
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

void TcpDataTransport::connect_complete(const boost::system::error_code &error)
{
    d_read_error = (error != nullptr);
    d_timer.cancel();
}

void TcpDataTransport::read_complete(const boost::system::error_code &error,
                                     size_t bytes_transferred)
{
    d_read_error        = (error || bytes_transferred == 0);
    d_bytes_transferred = bytes_transferred;
    d_timer.cancel();
}

void TcpDataTransport::time_out(const boost::system::error_code &error)
{
    if (error)
        return;
    d_socket.cancel();
}

ByteVector TcpDataTransport::receive(long int timeout)
{
    ByteVector recv(256);
    d_ios.reset();
    d_bytes_transferred = 0;

    d_ios.reset();
    d_socket.async_receive(boost::asio::buffer(recv),
                           boost::bind(&TcpDataTransport::read_complete, this,
                                       boost::asio::placeholders::error,
                                       boost::asio::placeholders::bytes_transferred));

    d_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
    d_timer.async_wait(
        boost::bind(&TcpDataTransport::time_out, this, boost::asio::placeholders::error));

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

void TcpDataTransport::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getTransportType());
    node.put("IpAddress", d_ipAddress);
    node.put("Port", d_port);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void TcpDataTransport::unSerialize(boost::property_tree::ptree &node)
{
    d_ipAddress = node.get_child("IpAddress").get_value<std::string>();
    d_port      = node.get_child("Port").get_value<int>();
}

std::string TcpDataTransport::getDefaultXmlNodeName() const
{
    return "TcpDataTransport";
}
}

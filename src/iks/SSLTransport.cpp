#include "logicalaccess/iks/SSLTransport.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/myexception.hpp"

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/logs.hpp>

namespace logicalaccess
{
SSLTransport::SSLTransport(
#ifdef ENABLE_SSLTRANSPORT
    boost::asio::ssl::context &ctx
#endif /* ENABLE_SSLTRANSPORT */
    )
    :
#ifdef ENABLE_SSLTRANSPORT
    ssl_ctx_(ctx)
    , d_socket(d_ios, ssl_ctx_)
    , d_timer(d_ios)
    ,
#endif /* ENABLE_SSLTRANSPORT */
    d_read_error(true)
    , d_bytes_transferred(0)
    , d_port(0)
{
}

SSLTransport::~SSLTransport()
{
}

std::string SSLTransport::getIpAddress() const
{
    return d_ipAddress;
}

void SSLTransport::setIpAddress(std::string ipAddress)
{
    d_ipAddress = ipAddress;
}

int SSLTransport::getPort() const
{
    return d_port;
}

void SSLTransport::setPort(int port)
{
    d_port = port;
}

bool SSLTransport::connect(long int timeout)
{
#ifndef ENABLE_SSLTRANSPORT
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "SSLTransport feature is disabled.");
#else
    if (d_socket.lowest_layer().is_open())
        d_socket.lowest_layer().close();

    try
    {
        d_ios.reset();
        d_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
        d_timer.async_wait(
            boost::bind(&SSLTransport::time_out, this, boost::asio::placeholders::error));

        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string(getIpAddress()), getPort());
        d_socket.lowest_layer().async_connect(
            endpoint, boost::bind(&SSLTransport::connect_complete, this,
                                  boost::asio::placeholders::error));

        d_ios.run();

        if (d_read_error)
            d_socket.lowest_layer().close();
        else
            LOG(LogLevel::INFOS) << "Connected to " << getIpAddress() << " on port "
                                 << getPort() << ".";
    }
    catch (boost::system::system_error &ex)
    {
        LOG(LogLevel::ERRORS) << "Cannot establish connection on " << getIpAddress()
                              << ":" << getPort() << " : " << ex.what();
        disconnect();
    }

    return handshake(timeout);
#endif /* ENABLE_SSLTRANSPORT */
}

void SSLTransport::disconnect()
{
    LOG(LogLevel::INFOS) << getIpAddress() << ":" << getPort() << "Disconnected.";
#ifdef ENABLE_SSLTRANSPORT
    d_socket.lowest_layer().close();
#endif /* ENABLE_SSLTRANSPORT */
}

bool SSLTransport::isConnected()
{
#ifndef ENABLE_SSLTRANSPORT
    return false;
#else
    return bool(d_socket.lowest_layer().is_open());
#endif /* ENABLE_SSLTRANSPORT */
}

std::string SSLTransport::getName() const
{
    return d_ipAddress;
}

void SSLTransport::send(const ByteVector &data)
{
#ifndef ENABLE_SSLTRANSPORT
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "SSLTransport feature is disabled.");
#else
    if (data.size() > 0)
    {
        try
        {
            LOG(LogLevel::COMS) << "TCP Send Data: " << BufferHelper::getHex(data);
            boost::asio::write(d_socket, boost::asio::buffer(data));
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
#endif /* ENABLE_SSLTRANSPORT */
}

void SSLTransport::connect_complete(const boost::system::error_code &error) const
{
#ifdef ENABLE_SSLTRANSPORT
    d_read_error = (error != 0);
    d_timer.cancel();
#endif /* ENABLE_SSLTRANSPORT */
}

void SSLTransport::read_complete(const boost::system::error_code &error,
                                 size_t bytes_transferred) const
{
#ifdef ENABLE_SSLTRANSPORT
    d_read_error        = (error || bytes_transferred == 0);
    d_bytes_transferred = bytes_transferred;
    d_timer.cancel();
#endif /* ENABLE_SSLTRANSPORT */
}

void SSLTransport::time_out(const boost::system::error_code &error) const
{
#ifdef ENABLE_SSLTRANSPORT
    if (error)
        return;
    d_socket.lowest_layer().cancel();
#endif /* ENABLE_SSLTRANSPORT */
}

ByteVector SSLTransport::receive(long int timeout)
{
    ByteVector recv(256);
#ifdef ENABLE_SSLTRANSPORT
    d_ios.reset();
    d_bytes_transferred = 0;

    d_ios.reset();
    d_socket.async_read_some(boost::asio::buffer(recv),
                             boost::bind(&SSLTransport::read_complete, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));

    d_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
    d_timer.async_wait(
        boost::bind(&SSLTransport::time_out, this, boost::asio::placeholders::error));

    d_ios.run();

    recv.resize(d_bytes_transferred);
    if (d_read_error || recv.size() == 0)
    {
        char buf[64];
        sprintf(buf, "Socket receive timeout (> %ld milliseconds).", timeout);
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, buf);
    }

    LOG(LogLevel::COMS) << "TCP Data read: " << BufferHelper::getHex(recv);
#endif /* ENABLE_SSLTRANSPORT */
    return recv;
}

void SSLTransport::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getTransportType());
    node.put("IpAddress", d_ipAddress);
    node.put("Port", d_port);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void SSLTransport::unSerialize(boost::property_tree::ptree &node)
{
    d_ipAddress = node.get_child("IpAddress").get_value<std::string>();
    d_port      = node.get_child("Port").get_value<int>();
}

std::string SSLTransport::getDefaultXmlNodeName() const
{
    return "SSLTransport";
}

bool SSLTransport::handshake(long timeout) const
{
#ifndef ENABLE_SSLTRANSPORT
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "SSLTransport feature is disabled.");
#else
    try
    {
        timeout *= 3;
        LOG(INFOS) << "Timeout value: " << timeout;
        d_ios.reset();
        d_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
        d_timer.async_wait(
            boost::bind(&SSLTransport::time_out, this, boost::asio::placeholders::error));

        d_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                 [&](const boost::system::error_code &error) {
                                     LOG(LogLevel::INFOS)
                                         << "Handshake return code: " << error
                                         << " ::: " << error.message();
                                     d_read_error = (error != 0);

                                     if (!d_read_error)
                                         d_timer.cancel();
                                 });
        d_ios.run();
        if (d_read_error)
            d_socket.lowest_layer().close();
        else
            LOG(LogLevel::INFOS) << "SSL Handshake to " << getIpAddress() << " on port "
                                 << getPort() << ""
                                                 "completed.";
    }
    catch (boost::system::system_error &ex)
    {
        LOG(LogLevel::ERRORS) << "Cannot finish handshak against " << getIpAddress()
                              << ":" << getPort() << " : " << ex.what();
        disconnect();
    }
    return d_socket.lowest_layer().is_open();
#endif /* ENABLE_SSLTRANSPORT */
}
}

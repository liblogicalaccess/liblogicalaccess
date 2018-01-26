/**
 * \file udpdatatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief UDP transport for reader/card commands.
 */

#ifndef LOGICALACCESS_UDPDATATRANSPORT_HPP
#define LOGICALACCESS_UDPDATATRANSPORT_HPP

#include <logicalaccess/readerproviders/datatransport.hpp>
#include <boost/asio.hpp>

namespace logicalaccess
{
#define TRANSPORT_UDP "UDP"

/**
 * \brief An UDP data transport class.
 */
class LIBLOGICALACCESS_API UdpDataTransport : public DataTransport
{
  public:
    /**
     * \brief Constructor.
     */
    UdpDataTransport();

    /**
     * \brief Destructor.
     */
    virtual ~UdpDataTransport();

    /**
     * \brief Get the transport type of this instance.
     * \return The transport type.
     */
    std::string getTransportType() const override
    {
        return TRANSPORT_UDP;
    }

    /**
     * \param Connect to the transport layer.
     * \return True on success, false otherwise.
     */
    bool connect() override;

    /**
     * \param Disconnect from the transport layer.
     */
    void disconnect() override;

    /**
     * \briaf Get if connected to the transport layer.
     * \return True if connected, false otherwise.
     */
    bool isConnected() override;

    /**
     * \brief Get the data transport endpoint name.
     * \return The data transport endpoint name.
     */
    std::string getName() const override;

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override;

    /**
     * \brief Get the client socket.
     */
    std::shared_ptr<boost::asio::ip::udp::socket> getSocket() const;

    /**
     * \brief Get the ip address.
     * \return The ip address.
     */
    std::string getIpAddress() const;

    /**
     * \brief Set the ip address.
     * \param ipAddress The ip address.
     */
    void setIpAddress(std::string ipAddress);

    /**
     * \brief Get the port.
     * \return The port.
     */
    int getPort() const;

    /**
     * \brief Set the port.
     * \param port The port.
     */
    void setPort(int port);

    void send(const ByteVector &data) override;

    ByteVector receive(long int timeout) override;

  protected:
    /**
     * \brief Client socket use to communicate with the reader.
     */
    std::shared_ptr<boost::asio::ip::udp::socket> d_socket;

    /**
     * \brief Provides core I/O functionality
     */
    boost::asio::io_context ios;

    /**
     * \brief The ip address
     */
    std::string d_ipAddress;

    /**
     * \brief The listening port.
     */
    int d_port;
};
}

#endif /* LOGICALACCESS_UDPDATATRANSPORT_HPP */
/**
 * \file udpdatatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief UDP transport for reader/card commands. 
 */

#ifndef LOGICALACCESS_UDPDATATRANSPORT_HPP
#define LOGICALACCESS_UDPDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/datatransport.hpp"
#include <boost/asio.hpp>

namespace logicalaccess
{
	#define	TRANSPORT_UDP			"UDP"

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
			virtual std::string getTransportType() const { return TRANSPORT_UDP; };

			/**
			 * \param Connect to the transport layer.
			 * \return True on success, false otherwise.
			 */
			virtual bool connect();

			/**
			 * \param Disconnect from the transport layer.
			 */
			virtual void disconnect();

			/**
			 * \briaf Get if connected to the transport layer.
			 * \return True if connected, false otherwise.
			 */
			virtual bool isConnected();

			/**
			 * \brief Get the data transport endpoint name.
			 * \return The data transport endpoint name.
			 */
			virtual std::string getName() const;

			/**
			 * \brief Serialize the current object to XML.
			 * \param parentNode The parent node.
			 */
			virtual void serialize(boost::property_tree::ptree& parentNode);

			/**
			 * \brief UnSerialize a XML node to the current object.
			 * \param node The XML node.
			 */
			virtual void unSerialize(boost::property_tree::ptree& node);

			/**
			 * \brief Get the default Xml Node name for this object.
			 * \return The Xml node name.
			 */
			virtual std::string getDefaultXmlNodeName() const;

			/**
			 * \brief Get the client socket.
			 */
			boost::shared_ptr<boost::asio::ip::udp::socket> getSocket();

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

			virtual void send(const std::vector<unsigned char>& data);

			virtual std::vector<unsigned char> receive(long int timeout);

		protected:

			/**
			 * \brief Client socket use to communicate with the reader.
			 */
			boost::shared_ptr<boost::asio::ip::udp::socket> d_socket;
			
			/**
			 * \brief Provides core I/O functionality
			 */
			boost::asio::io_service ios;

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


/**
 * \file rplethdatatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth transport for reader/card commands. 
 */

#ifndef LOGICALACCESS_RPLETHDATATRANSPORT_HPP
#define LOGICALACCESS_RPLETHDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/tcpdatatransport.hpp"
#include <boost/asio.hpp>
#include <list>

namespace logicalaccess
{
	#define	TRANSPORT_RPLETH			"Rpleth"

	/**
	 * \brief An rpleth data transport class.
	 */
	class LIBLOGICALACCESS_API RplethDataTransport : public TcpDataTransport
	{
		public:

			/**
			 * \brief Constructor.
			 */
			RplethDataTransport();

			/**
			 * \brief Destructor.
			 */
			virtual ~RplethDataTransport();

			/**
			 * \brief Get the transport type of this instance.
			 * \return The transport type.
			 */
			virtual std::string getTransportType() const { return TRANSPORT_RPLETH; };

			/**
			 * \brief Get the default Xml Node name for this object.
			 * \return The Xml node name.
			 */
			virtual std::string getDefaultXmlNodeName() const;

			/**
			 * \brief Send the data without computation.
			 * \param data The data to send.
			 */
			void sendll(const std::vector<unsigned char>& data);

			/**
			 * \brief Send the data using rpleth protocol computation.
			 * \param data The data to send.
			 */
			virtual void send(const std::vector<unsigned char>& data);

			/**
			 * \brief Receive data from reader.
			 * \param timeout The time to wait data.
			 * \return The data from reader.
			 */
			virtual std::vector<unsigned char> receive(long int timeout = 5000);

			/**
			 * \brief Send the Ping packet.
			 */
			void sendPing();

			/**
			 * \brief Get the buffer.
			 * \return The buffer.
			 */
			const std::vector<unsigned char>& getBuffer() { return d_buffer; };


			/**
			 * \brief Get the badges list.
			 * \return The list of badge.
			 */
			std::list<std::vector<unsigned char> > &getBadges() { return d_badges; };

			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return the result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout = 2000);

		protected:

			/**
			 * \brief Calculate command checksum.
			 * \param data The data to calculate checksum
			 * \return The checksum.
			 */
			unsigned char calcChecksum(const std::vector<unsigned char>& data);	

			/**
			 * \brief d_buffer from last commands response.
			 */
			std::vector<unsigned char> d_buffer;

			/**
			 * \brief Badges received from the latest commands response.
			 */
			std::list<std::vector<unsigned char> > d_badges;
	};

}

#endif /* LOGICALACCESS_RPLETHDATATRANSPORT_HPP */


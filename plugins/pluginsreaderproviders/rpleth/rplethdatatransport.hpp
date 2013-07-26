/**
 * \file rplethdatatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth transport for reader/card commands. 
 */

#ifndef LOGICALACCESS_RPLETHDATATRANSPORT_HPP
#define LOGICALACCESS_RPLETHDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/tcpdatatransport.hpp"
#include <boost/asio.hpp>

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
			virtual std::vector<unsigned char> receive(long int timeout = 2000);

		protected:

			/**
			 * \brief Calculate command checksum.
			 * \param data The data to calculate checksum
			 * \return The checksum.
			 */
			unsigned char calcChecksum(const std::vector<unsigned char>& data);	

			/**
			 * \brief Trashed data from last command response.
			 */
			std::vector<unsigned char> d_trashedData;
	};

}

#endif /* LOGICALACCESS_RPLETHDATATRANSPORT_HPP */


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
			 * \brief Get the data transport endpoint name.
			 * \return The data transport endpoint name.
			 */
			virtual std::string getName() const;

			/**
			 * \brief Get the default Xml Node name for this object.
			 * \return The Xml node name.
			 */
			virtual std::string getDefaultXmlNodeName() const;

		protected:

			virtual void send(const std::vector<unsigned char>& data);

			virtual std::vector<unsigned char> receive(long int timeout);
	};

}

#endif /* LOGICALACCESS_RPLETHDATATRANSPORT_HPP */


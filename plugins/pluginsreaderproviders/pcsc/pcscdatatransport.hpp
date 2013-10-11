/**
 * \file pcscdatatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC transport for reader/card commands. 
 */

#ifndef LOGICALACCESS_UDPDATATRANSPORT_HPP
#define LOGICALACCESS_UDPDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/datatransport.hpp"
#include "pcscreaderunit.hpp"
#include <boost/asio.hpp>

namespace logicalaccess
{
	#define	TRANSPORT_PCSC			"PCSC"

	/**
	 * \brief An PCSC data transport class.
	 */
	class LIBLOGICALACCESS_API PCSCDataTransport : public DataTransport
	{
		public:

			/**
			 * \brief Constructor.
			 */
			PCSCDataTransport();

			/**
			 * \brief Destructor.
			 */
			virtual ~PCSCDataTransport();

			/**
			 * \brief Get the transport type of this instance.
			 * \return The transport type.
			 */
			virtual std::string getTransportType() const { return TRANSPORT_PCSC; };

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
			 * \brief Get the PC/SC reader unit.
			 * \return The PC/SC reader unit.
			 */
			boost::shared_ptr<PCSCReaderUnit> getPCSCReaderUnit() const { return boost::dynamic_pointer_cast<PCSCReaderUnit>(getReaderUnit()); };

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
			 * \brief Check the card error and throw exception if needed.
			 * \param errorFlag The error flag.
			 */
			static void CheckCardError(unsigned int errorFlag);

			/**
			 * \brief Check the command result and throw exception if needed.
			 * \param data The result buffer.
			 * \param datalen The result buffer length.
			 */
			virtual void CheckResult(const void* data, size_t datalen) const;

			virtual void send(const std::vector<unsigned char>& data);

			virtual std::vector<unsigned char> receive(long int timeout);

		protected:

			bool d_isConnected;

			std::vector<unsigned char> d_response;
	};

}

#endif /* LOGICALACCESS_PCSCDATATRANSPORT_HPP */


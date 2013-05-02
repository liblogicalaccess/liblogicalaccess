/**
 * \file A3MLGM5600ReaderUnitConfiguration.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief A3MLGM5600 Reader unit configuration.
 */

#ifndef LOGICALACCESS_A3MLGM5600READERUNITCONFIGURATION_H
#define LOGICALACCESS_A3MLGM5600READERUNITCONFIGURATION_H

#include "logicalaccess/ReaderProviders/ReaderUnitConfiguration.h"


namespace LOGICALACCESS
{	
	/**
	 * \brief The A3MLGM5600 reader unit configuration base class.
	 */
	class LIBLOGICALACCESS_API A3MLGM5600ReaderUnitConfiguration : public ReaderUnitConfiguration
	{
		public:

			/**
			 * \brief Constructor.
			 */
			A3MLGM5600ReaderUnitConfiguration();

			/**
			 * \brief Destructor.
			 */
			virtual ~A3MLGM5600ReaderUnitConfiguration();

			/**
			 * \brief Reset the configuration to default values
			 */
			virtual void resetConfiguration();

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
			 * \brief Get the reader ip address.
			 * \return The reader ip address.
			 */
			std::string getReaderIpAddress() const;

			/**
			 * \brief Set the reader ip address.
			 * \param ipAddress The reader ip address.
			 */
			void setReaderIpAddress(std::string ipAddress);

			/**
			 * \brief Get the reader port.
			 * \return The reader port.
			 */
			int getReaderPort() const;

			/**
			 * \brief Set the reader port.
			 * \param port The reader port.
			 */
			void setReaderPort(int port);

			/**
			 * \brief Get the local port.
			 * \return The local port.
			 */
			int getLocalPort() const;

			/**
			 * \brief Set the local port.
			 * \param port The local port.
			 */
			void setLocalPort(int port);

		protected:

			/**
			 * \brief The reader ip address
			 */
			std::string d_readerIpAddress;

			/**
			 * \brief The reader listening port.
			 */
			int d_readerPort;

			/**
			 * \brief The location listening port.
			 */
			int d_localPort;
	};
}

#endif

/**
 * \file rwk400readerunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rwk400 Reader unit configuration.
 */

#ifndef LOGICALACCESS_RWK400READERUNITCONFIGURATION_HPP
#define LOGICALACCESS_RWK400READERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"

namespace logicalaccess
{
	/**
	 * \brief The baudrate of Serial output.
	 */
	typedef enum
	{
		BRO_4800BAUD = 0x02,
		BRO_9600BAUD = 0x03,
		BRO_19200BAUD = 0x04,
		BRO_38400BAUD = 0x05,
		BRO_57600BAUD = 0x06,
		BRO_115200BAUD = 0x07,
		BRO_230400BAUD = 0x08,
		BRO_460800BAUD = 0x09,
		BRO_921600BAUD = 0x0A
	}BaudRateOutput;

	/**
	 * \brief The Rwk400 reader unit configuration base class.
	 */
	class LIBLOGICALACCESS_API Rwk400ReaderUnitConfiguration : public ReaderUnitConfiguration
	{
		public:

			/**
			 * \brief Constructor.
			 */
			Rwk400ReaderUnitConfiguration();

			/**
			 * \brief Destructor.
			 */
			virtual ~Rwk400ReaderUnitConfiguration();

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
			 * \brief Get the baud rate.
			 * \return The baud rate.
			 */
			BaudRateOutput getBaudrate();

			/**
			 * \brief Set the baud rate.
			 * \param baudrate The baud rate.
			 */
			void setBaudrate(BaudRateOutput baudrate);

			/**
			 * \brief Get the system baud rate.
			 * \return The system baud rate.
			 */
			int getSystemBaudrate();

		protected:

			/**
			 * \brief Get the system baudrate from the baudrate output type.
			 * \param baudrate The baudrate output value.
			 * \return The system baudrate value.
			 */
			int getSystemBaudrateFromBaudrateOutput(BaudRateOutput baudrate);

			/**
			 * \brief The baudrate output.
			 */
			BaudRateOutput d_baudrate;
	};
}

#endif /* LOGICALACCESS_RWK400READERUNITCONFIGURATION_HPP */

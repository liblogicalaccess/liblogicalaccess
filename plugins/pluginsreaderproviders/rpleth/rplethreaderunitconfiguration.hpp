/**
 * \file rplethreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth Reader unit configuration.
 */

#ifndef LOGICALACCESS_RPLETHREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_RPLETHREADERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"


namespace logicalaccess
{	
	typedef enum
	{
		WIEGAND = 0x01,
		RS = 0x02
	} RplethMode;

	/**
	 * \brief The Rpleth reader unit configuration base class.
	 */
	class LIBLOGICALACCESS_API RplethReaderUnitConfiguration : public ReaderUnitConfiguration
	{
		public:

			/**
			 * \brief Constructor.
			 */
			RplethReaderUnitConfiguration();

			/**
			 * \brief Destructor.
			 */
			virtual ~RplethReaderUnitConfiguration();

			/**
			 * \brief Reset the configuration to default values
			 */
			virtual void resetConfiguration();

			/**
			 * \brief Set the configuration to param values
			 * \param readerAddress The reader address
			 * \param port The reader port
			 */
			virtual void setConfiguration(const std::string& readerAddress, int port);

			/**
			 * \brief Set the wiegand configuration to param values
			 * \param offset The offset in wiegand communication
			 * \param lenght The lenght of csn in wiegand communication
			 */
			void setWiegandConfiguration (unsigned char offset, unsigned char lenght);

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
			 * \brief Get the reader address.
			 * \return The reader address.
			 */
			std::string getReaderAddress() const;

			
			/**
			 * \brief Set the reader address.
			 * \param readerAddress The new reader address.
			 */
			void setReaderAddress(const std::string& readerAddress);
			
			/**
			 * \brief Get the reader port.
			 * \return The reader port.
			 */
			int getPort() const;

			/**
			 * \brief Set the reader port.
			 * \param port The new reader port.
			 */
			void setPort(int port);
			
			/**
			 * \brief Get the offset in wiegand communication.
			 * \return The offset in wiegand communication.
			 */
			unsigned char getOffset() const;

			/**
			 * \brief Set the offset in wiegand communication.
			 * \param offset The new offset in wiegand communication.
			 */
			void setOffset(unsigned char offset);
			
			/**
			 * \brief Get the length of csn in wiegand communication.
			 * \return The length of csn in wiegand communication.
			 */
			unsigned char getLength() const;

			/**
			 * \brief Set the lenght of csn in wiegand communication.
			 * \param length The new length of csn in wiegand communication.
			 */
			void setLength(unsigned char length);

			/**
			 * \brief Get the mode of the reader.
			 * \return The mode of the reader.
			 */
			RplethMode getMode() const;

			/**
			 * \brief Set the type of the reader.
			 * \param mode The new type of the reader.
			 */
			void setMode(RplethMode mode);

		protected:

			std::string d_readerAddress;

			int d_port;

			unsigned char d_offset;

			unsigned char d_length;

			RplethMode d_mode;
	};
}

#endif

/**
 * \file iso7816location.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 location.
 */

#ifndef LOGICALACCESS_ISO7816LOCATION_HPP
#define LOGICALACCESS_ISO7816LOCATION_HPP

#include "logicalaccess/cards/location.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	/**
	 * \brief ISO7816 file type.
	 */
	typedef enum
	{
		IFT_NONE = 0x00,
		IFT_MASTER = 0x01,
		IFT_DIRECTORY = 0x02,
		IFT_TRANSPARENT = 0x04,
		IFT_LINEAR_FIXED = 0x08,
		IFT_LINEAR_VARIABLE = 0x10,
		IFT_CYCLIC = 0x20
	}ISO7816FileType;

	/**
	 * \brief A ISO7816 location informations.
	 */
	class LIBLOGICALACCESS_API ISO7816Location : public Location
	{
		public:
			/**
			 * \brief Constructor.
			 */
			ISO7816Location();

			/**
			 * \brief Destructor.
			 */
			virtual ~ISO7816Location();

			/**
			 * \brief Serialize the current object to XML.
			 * \param xmlwriter The XML writer.
			 * \return The XML stream.
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
			 * \brief Get the card type for this location.
			 * \return The card type.
			 */
			virtual std::string getCardType() { return "ISO7816"; }

			/**
			 * \brief Equality operator
			 * \param location Location to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const Location& location) const;

		public:
			
			/**
			 * \brief The DF name.
			 */
			unsigned char dfname[16];

			/**
			 * \brief The DF name length.
			 */
			int dfnamelen;

			/**
			 * \brief The file identifier.
			 */
			unsigned short fileid;

			/**
			 * \brief The file type.
			 */
			ISO7816FileType fileType;

			/**
			 * \brief The data object.
			 */
			unsigned short dataObject;
	};	
}

#endif /* LOGICALACCESS_ISO7816LOCATION_H */

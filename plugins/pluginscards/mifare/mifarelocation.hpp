/**
 * \file mifarelocation.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief MifareLocation.
 */

#ifndef LOGICALACCESS_MIFARELOCATION_HPP
#define LOGICALACCESS_MIFARELOCATION_HPP

#include "logicalaccess/cards/location.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	/**
	 * \brief A  Mifare location informations.
	 */
	class LIBLOGICALACCESS_API MifareLocation : public Location
	{
		public:
			/**
			 * \brief Constructor.
			 */
			MifareLocation();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareLocation();

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
			 * \brief Get the card type for this location.
			 * \return The card type.
			 */
			virtual std::string getCardType() { return "Mifare"; }

			/**
			 * \brief Equality operator
			 * \param location Location to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const Location& location) const;

		public:
			/**
			 * \brief The sector.
			 */
			int sector;

			/**
			 * \brief The block.
			 */
			int block;

			/**
			 * \brief The byte.
			 */
			int byte;

			/**
			 * \brief MAD is used ?
			 */
			bool useMAD;

			/**
			 * \brief The MAD Application ID.
			 */
			unsigned short aid;
	};	
}

#endif /* LOGICALACCESS_MIFARELOCATION_HPP */

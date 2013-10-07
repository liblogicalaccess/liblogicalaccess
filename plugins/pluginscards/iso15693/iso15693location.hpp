/**
 * \file iso15693location.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO15693 location.
 */

#ifndef LOGICALACCESS_ISO15693LOCATION_HPP
#define LOGICALACCESS_ISO15693LOCATION_HPP

#include "logicalaccess/cards/location.hpp"

namespace logicalaccess
{
	/**
	 * \brief A ISO15693 location informations.
	 */
	class LIBLOGICALACCESS_API ISO15693Location : public Location
	{
		public:
			/**
			 * \brief Constructor.
			 */
			ISO15693Location();

			/**
			 * \brief Destructor.
			 */
			virtual ~ISO15693Location();
			
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
			virtual std::string getCardType() { return "ISO15693"; }

			/**
			 * \brief Equality operator
			 * \param location Location to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const Location& location) const;

		public:
			/**
			 * \brief The block.
			 */
			int block;
	};	
}

#endif /* LOGICALACCESS_ISO15693LOCATION_H */

/**
 * \file ISO15693Location.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO15693 location.
 */

#ifndef LOGICALACCESS_ISO15693LOCATION_H
#define LOGICALACCESS_ISO15693LOCATION_H

#include "logicalaccess/Cards/Location.h"

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
			 * \brief Export DESFire location informations to a buffer.
			 * \return The data.
			 */
			virtual std::vector<unsigned char> getLinearData() const;

			/**
			 * \brief Import DESFire location informations from a buffer.
			 * \param data The buffer.
			 * \param offset An offset.
			 */
			virtual void setLinearData(const std::vector<unsigned char>&, size_t offset = 0);

			/**
			 * \brief Prox location informations data size.
			 */
			virtual size_t getDataSize();

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

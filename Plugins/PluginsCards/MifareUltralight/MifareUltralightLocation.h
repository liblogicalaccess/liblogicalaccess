/**
 * \file MifareUltralightLocation.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Mifare Ultralight Location.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTLOCATION_H
#define LOGICALACCESS_MIFAREULTRALIGHTLOCATION_H

#include "logicalaccess/Cards/Location.h"

namespace LOGICALACCESS
{
	/**
	 * \brief A  Mifare Ultralight location informations.
	 */
	class LIBLOGICALACCESS_API MifareUltralightLocation : public Location
	{
		public:
			/**
			 * \brief Constructor.
			 */
			MifareUltralightLocation();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareUltralightLocation();
			
			/**
			 * \brief Export DESFire location informations to a buffer.
			 * \return The data.
			 */
			virtual std::vector<unsigned char> getLinearData() const;

			/**
			 * \brief Import location informations from a buffer.
			 * \param data The buffer.
			 * \param offset An offset.
			 */
			virtual void setLinearData(const std::vector<unsigned char>&, size_t offset = 0);

			/**
			 * \brief Location informations data size.
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
			virtual std::string getCardType() { return "MifareUltralight"; }

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
			int page;

			/**
			 * \brief The byte.
			 */
			int byte;
	};	
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTLOCATION_H */

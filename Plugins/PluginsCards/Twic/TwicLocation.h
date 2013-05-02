/**
 * \file TwicLocation.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Twic location.
 */

#ifndef LOGICALACCESS_TWICLOCATION_H
#define LOGICALACCESS_TWICLOCATION_H

#include "logicalaccess/Cards/Location.h"

namespace LOGICALACCESS
{	
	/**
	 * \brief A Twic location informations.
	 */
	class LIBLOGICALACCESS_API TwicLocation : public Location
	{
		public:
			/**
			 * \brief Constructor.
			 */
			TwicLocation();

			/**
			 * \brief Destructor.
			 */
			virtual ~TwicLocation();
			
			/**
			 * \brief Export location informations to a buffer.
			 * \param data The buffer.
			 */
			virtual std::vector<unsigned char> getLinearData() const;

			/**
			 * \brief Import location informations from a buffer.
			 * \param data The buffer.
			 * \param offset The offset.
			 */
			virtual void setLinearData(const std::vector<unsigned char>& data, size_t offset);

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
			virtual std::string getCardType() { return "Twic"; }

			/**
			 * \brief Equality operator
			 * \param location Location to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const Location& location) const;

		public:
			
			/**
			 * \brief The data object.
			 */
			uint64_t dataObject;

			/**
			 * \brief The tag. 0x00 means the full data object.
			 */
			unsigned char tag;
	};	
}

#endif /* LOGICALACCESS_TWICLOCATION_H */

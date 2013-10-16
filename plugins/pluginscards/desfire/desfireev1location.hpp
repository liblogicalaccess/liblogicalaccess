/**
 * \file desfirelocation.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireLocation.
 */

#ifndef LOGICALACCESS_DESFIREEV1LOCATION_HPP
#define LOGICALACCESS_DESFIREEV1LOCATION_HPP

#include "desfirelocation.hpp"

namespace logicalaccess
{
	/**
	 * \brief A DESFire EV1 location informations.
	 */
	class LIBLOGICALACCESS_API DESFireEV1Location : public DESFireLocation
	{
		public:
			/**
			 * \brief Constructor.
			 */
			DESFireEV1Location();

			/**
			 * \brief Destructor.
			 */
			virtual ~DESFireEV1Location();

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
			virtual std::string getCardType() { return "DESFireEV1"; }

			/**
			 * \brief Equality operator
			 * \param location Location to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const Location& location) const;

		public:
			
			/**
			 * \brief Use EV1 functionalities.
			 */
			bool useEV1;

			/**
			 * \brief The application crypto method.
			 */
			DESFireKeyType cryptoMethod;

			/**
			 * \brief Use ISO7816 naming.
			 */
			bool useISO7816;

			/**
			 * \brief The ISO7816 application FID.
			 */
			unsigned short applicationFID;

			/**
			 * \brief The ISO7816 file FID.
			 */
			unsigned short fileFID;
	};	
}

#endif /* LOGICALACCESS_DESFIREEV1LOCATION_HPP */

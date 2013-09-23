/**
 * \file desfireaccessinfo.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireAccessInfo.
 */

#ifndef LOGICALACCESS_DESFIREACCESSINFO_HPP
#define LOGICALACCESS_DESFIREACCESSINFO_HPP

#include "logicalaccess/cards/accessinfo.hpp"
#include "desfirekey.hpp"

namespace logicalaccess
{
	/**
	 * \brief A DESFire access informations.
	 */
	class LIBLOGICALACCESS_API DESFireAccessInfo : public AccessInfo
	{
		public:
			/**
			 * \brief Constructor.
			 */
			DESFireAccessInfo();

			/**
			 * \brief Destructor.
			 */
			virtual ~DESFireAccessInfo();

			/**
			 * \brief Generate pseudo-random DESFire access informations.
			 */
			virtual void generateInfos();

			/**
			 * \brief Generate pseudo-random DESFire access informations for a given card serial number.
			 * \param csn The card serial number.
			 */
			virtual void generateInfos(const std::string& csn);

			/**
			 * \brief DESFire access informations data size.
			 */
			virtual size_t getDataSize();

			/**
			 * \brief Get the card type for this access infos.
			 * \return The card type.
			 */
			virtual std::string getCardType() const;

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
			 * \brief Equality operator
			 * \param ai Access infos to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const AccessInfo& ai) const;

			/**
			 * \brief Get the default Xml Node name for this object.
			 * \return The Xml node name.
			 */
			virtual std::string getDefaultXmlNodeName() const;

		public:
			/**
			 * \brief The master card key.
			 */
			boost::shared_ptr<DESFireKey> masterCardKey;

			/**
			 * \brief The master application key.
			 */
			boost::shared_ptr<DESFireKey> masterApplicationKey;

			/**
			 * \brief The read key.
			 */
			boost::shared_ptr<DESFireKey> readKey;

			/**
			 * \brief The write key.
			 */
			boost::shared_ptr<DESFireKey> writeKey;

			/**
			 * \brief The read key number.
			 */
			unsigned char readKeyno;

			/**
			 * \brief The write key number.
			 */
			unsigned char writeKeyno;			
	};	
}

#endif /* LOGICALACCESS_DESFIREACCESSINFO_HPP */

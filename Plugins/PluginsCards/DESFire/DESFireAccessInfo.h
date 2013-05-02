/**
 * \file DESFireAccessInfo.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief DESFireAccessInfo.
 */

#ifndef LOGICALACCESS_DESFIREACCESSINFO_H
#define LOGICALACCESS_DESFIREACCESSINFO_H

#include "logicalaccess/Cards/AccessInfo.h"
#include "DESFireKey.h"

namespace LOGICALACCESS
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
			virtual void generateInfos(const string& csn);

			/**
			 * \brief Export access informations to a buffer.
			 * \param data The buffer.
			 */
			virtual std::vector<unsigned char> getLinearData() const;

			/**
			 * \brief Import access informations from a buffer.
			 * \param data The buffer.
			 * \param offset The offset. Default is 0.
			 */
			virtual void setLinearData(const std::vector<unsigned char>& data, size_t offset = 0);

			/**
			 * \brief DESFire access informations data size.
			 */
			virtual size_t getDataSize();

			/**
			 * \brief Get the card type for this access infos.
			 * \return The card type.
			 */
			virtual std::string getCardType() { return "DESFire"; }

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

#endif /* LOGICALACCESS_DESFIREACCESSINFO_H */

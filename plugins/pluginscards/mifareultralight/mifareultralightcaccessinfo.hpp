/**
 * \file mifareultralightcaccessinfo.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C AccessInfo.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCACCESSINFO_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCACCESSINFO_HPP

#include "mifareultralightaccessinfo.hpp"
#include "logicalaccess/cards/tripledeskey.hpp"


namespace logicalaccess
{
	/**
	 * \brief A Mifare Ultralight C access informations.
	 */
	class LIBLOGICALACCESS_API MifareUltralightCAccessInfo : public MifareUltralightAccessInfo
	{
		public:
			/**
			 * \brief Constructor.
			 */
			MifareUltralightCAccessInfo();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareUltralightCAccessInfo();

			/**
			 * \brief Generate pseudo-random Mifare Ultralight C access informations.
			 */
			virtual void generateInfos();

			/**
			 * \brief Generate pseudo-random Mifare Ultralight C access informations for a given card serial number.
			 * \param csn The card serial number.
			 */
			virtual void generateInfos(const std::string& csn);

			/**
			 * \brief Export location informations to a buffer.
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
			 * \brief Mifare access informations data size.
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
			 * \brief Get the default Xml Node name for this object.
			 * \return The Xml node name.
			 */
			virtual std::string getDefaultXmlNodeName() const;

			/**
			 * \brief Equality operator
			 * \param ai Access infos to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const AccessInfo& ai) const;

		public:

			/**
			 * \brief The authentication key.
			 */
			boost::shared_ptr<TripleDESKey> key;
	};	
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCACCESSINFO_H */

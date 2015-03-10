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
        using XmlSerializable::serialize;
        using XmlSerializable::unSerialize;

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
         * \brief Get the card type for this access infos.
         * \return The card type.
         */
        virtual std::string getCardType() const;

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
        virtual void serialize(boost::property_tree::ptree& parentNode) override;

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
        virtual void unSerialize(boost::property_tree::ptree& node) override;

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
        virtual std::string getDefaultXmlNodeName() const override;

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
        std::shared_ptr<TripleDESKey> key;
    };
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCACCESSINFO_H */
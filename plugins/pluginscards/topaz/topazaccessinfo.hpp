/**
 * \file topazaccessinfo.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz AccessInfo.
 */

#ifndef LOGICALACCESS_TOPAZACCESSINFO_HPP
#define LOGICALACCESS_TOPAZACCESSINFO_HPP

#include "logicalaccess/cards/accessinfo.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief A Topaz access informations.
     */
    class LIBLOGICALACCESS_API TopazAccessInfo : public AccessInfo
    {
    public:
        /**
         * \brief Constructor.
         */
        TopazAccessInfo();

        /**
         * \brief Destructor.
         */
        virtual ~TopazAccessInfo();

        /**
         * \brief Generate pseudo-random Topaz access informations.
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
         * \brief Page is locked ?
         */
        bool lockPage;
    };
}

#endif /* LOGICALACCESS_TOPAZACCESSINFO_HPP */
/**
 * \file mifareultralightlocation.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight Location.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTLOCATION_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTLOCATION_HPP

#include "logicalaccess/cards/location.hpp"

namespace logicalaccess
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
        int byte_;
    };
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTLOCATION_HPP */
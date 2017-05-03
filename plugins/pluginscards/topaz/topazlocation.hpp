/**
 * \file topazlocation.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz Location.
 */

#ifndef LOGICALACCESS_TOPAZLOCATION_HPP
#define LOGICALACCESS_TOPAZLOCATION_HPP

#include "logicalaccess/cards/location.hpp"

namespace logicalaccess
{
    /**
     * \brief A  Topaz location informations.
     */
    class LIBLOGICALACCESS_API TopazLocation : public Location
    {
    public:
        /**
         * \brief Constructor.
         */
        TopazLocation();

        /**
         * \brief Destructor.
         */
        virtual ~TopazLocation();

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
        virtual std::string getCardType() { return "Topaz"; }

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

#endif /* LOGICALACCESS_TOPAZLOCATION_HPP */
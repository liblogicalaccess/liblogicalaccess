/**
 * \file proxlocation.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ProxLocation.
 */

#ifndef LOGICALACCESS_PROXLOCATION_HPP
#define LOGICALACCESS_PROXLOCATION_HPP

#include "logicalaccess/cards/location.hpp"

namespace logicalaccess
{
    /**
     * \brief A  Prox location informations.
     */
    class LIBLOGICALACCESS_API ProxLocation : public Location
    {
    public:
        /**
         * \brief Constructor.
         */
        ProxLocation();

        /**
         * \brief Destructor.
         */
        virtual ~ProxLocation();

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
        virtual std::string getCardType() { return "Prox"; }

        /**
         * \brief Equality operator
         * \param location Location to compare.
         * \return True if equals, false otherwise.
         */
        virtual bool operator==(const Location& location) const;

    public:
        /**
         * \brief The bit.
         */
        int bit;
    };
}

#endif /* LOGICALACCESS_PROXLOCATION_HPP */
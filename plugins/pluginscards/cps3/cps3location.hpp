/**
 * \file cps3location.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief CPS3 location.
 */

#ifndef LOGICALACCESS_CPS3LOCATION_HPP
#define LOGICALACCESS_CPS3LOCATION_HPP

#include "logicalaccess/cards/location.hpp"

namespace logicalaccess
{
    /**
     * \brief A CPS3 location informations.
     */
	class LIBLOGICALACCESS_API CPS3Location : public Location
    {
    public:
        /**
         * \brief Constructor.
         */
		CPS3Location();

        /**
         * \brief Destructor.
         */
		virtual ~CPS3Location();

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
        virtual std::string getCardType() { return "FeliCa"; }

        /**
         * \brief Equality operator
         * \param location Location to compare.
         * \return True if equals, false otherwise.
         */
        virtual bool operator==(const Location& location) const;

    public:

		
    };
}

#endif /* LOGICALACCESS_CPS3LOCATION_H */
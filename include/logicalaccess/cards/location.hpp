/**
 * \file location.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location.
 */

#ifndef LOGICALACCESS_LOCATION_HPP
#define LOGICALACCESS_LOCATION_HPP

#include <string>
#include "logicalaccess/liblogicalaccess_export.hpp"
#include "logicalaccess/xmlserializable.hpp"

namespace logicalaccess
{
    /**
     * \brief A location informations.
     */
    class LIBLOGICALACCESS_API Location : public XmlSerializable
    {
    public:

        /**
         * \brief Get the card type for this location.
         * \return The card type.
         */
        virtual std::string getCardType() = 0;

        /**
         * \brief Equality operator
         * \param location Location to compare.
         * \return True if equals, false otherwise.
         */
        virtual bool operator==(const Location& location) const;

        /**
         * \brief Inequality operator
         * \param location Location to compare.
         * \return True if inequals, false otherwise.
         */
        bool operator!=(const Location& location) const { return !operator==(location); }
    };
}

#endif /* LOGICALACCESS_LOCATION_HPP */
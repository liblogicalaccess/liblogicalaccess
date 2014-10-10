/**
 * \file proxprofile.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox card profiles.
 */

#ifndef LOGICALACCESS_PROX_HPP
#define LOGICALACCESS_PROX_HPP

#include "logicalaccess/cards/accessinfo.hpp"
#include "proxlocation.hpp"
#include "logicalaccess/cards/profile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The HID Prox base profile class.
     */
    class LIBLOGICALACCESS_API ProxProfile : public Profile
    {
    public:

        /**
         * \brief Constructor.
         */
        ProxProfile();

        /**
         * \brief Destructor.
         */
        virtual ~ProxProfile();

        /**
         * \brief Set default keys for the mifare card in memory.
         */
        virtual void setDefaultKeys();

        /**
         * \brief Set default keys for the type card in memory at a specific location.
         */
        virtual void setDefaultKeysAt(boost::shared_ptr<Location> location);

        /**
         * \brief Clear all keys in memory.
         */
        virtual void clearKeys();

        /**
         * \brief Set Mifare key at a specific location.
         * \param location The key's location.
         * \param AccessInfo The key's informations.
         */
        virtual void setKeyAt(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> AccessInfo);

        /**
         * \brief Create default PROX access informations.
         * \return Default PROX access informations. Always null.
         */
        virtual boost::shared_ptr<AccessInfo> createAccessInfo() const;

        /**
         * \brief Create default PROX location.
         * \return Default PROX location.
         */
        virtual boost::shared_ptr<Location> createLocation() const;

        /**
         * \brief Get the supported format list.
         * \return The format list.
         */
        virtual FormatList getSupportedFormatList();

    protected:
    };
}

#endif
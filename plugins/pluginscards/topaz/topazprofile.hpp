/**
 * \file topazprofile.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz card profiles.
 */

#ifndef LOGICALACCESS_TOPAZ_HPP
#define LOGICALACCESS_TOPAZ_HPP

#include "logicalaccess/cards/accessinfo.hpp"
#include "topazlocation.hpp"
#include "logicalaccess/cards/profile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The Topaz base profile class.
     */
    class LIBLOGICALACCESS_API TopazProfile : public Profile
    {
    public:

        /**
         * \brief Constructor.
         */
        TopazProfile();

        /**
         * \brief Destructor.
         */
        virtual ~TopazProfile();

        /**
         * \brief Set default keys for the card in memory.
         */
        virtual void setDefaultKeys();

        /**
         * \brief Set default keys for the type card in memory at a specific location.
         */
        virtual void setDefaultKeysAt(std::shared_ptr<Location> location);

        /**
         * \brief Clear all keys in memory.
         */
        virtual void clearKeys();

        /**
         * \brief Set key at a specific location.
         * \param location The key's location.
         * \param AccessInfo The key's informations.
         */
        virtual void setKeyAt(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> AccessInfo);

        /**
         * \brief Set default keys for a sector.
         * \param index The sector index.
         */
        void setDefaultKeysAt(size_t index);

        /**
         * \brief Create default access informations.
         * \return Default access informations. Always null.
         */
        virtual std::shared_ptr<AccessInfo> createAccessInfo() const;

        /**
         * \brief Create default location.
         * \return Default location.
         */
        virtual std::shared_ptr<Location> createLocation() const;

        /**
         * \brief Get the supported format list.
         * \return The format list.
         */
        virtual FormatList getSupportedFormatList();

    protected:
    };
}

#endif
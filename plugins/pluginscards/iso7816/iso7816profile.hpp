/**
 * \file iso7816profile.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 card profiles.
 */

#ifndef LOGICALACCESS_ISO7816PROFILE_HPP
#define LOGICALACCESS_ISO7816PROFILE_HPP

#include "logicalaccess/cards/accessinfo.hpp"
#include "iso7816location.hpp"
#include "logicalaccess/cards/profile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The ISO7816 base profile class.
     */
    class LIBLOGICALACCESS_API ISO7816Profile : public Profile
    {
    public:

        /**
         * \brief Constructor.
         */
        ISO7816Profile();

        /**
         * \brief Destructor.
         */
        virtual ~ISO7816Profile();

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
         * \brief Set ISO7816 key at a specific location.
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
         * \brief Create default ISO7816 access informations.
         * \return Default ISO7816 access informations. Always null.
         */
        virtual std::shared_ptr<AccessInfo> createAccessInfo() const;

        /**
         * \brief Create default ISO7816 location.
         * \return Default ISO7816 location.
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
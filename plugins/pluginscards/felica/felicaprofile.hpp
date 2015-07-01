/**
 * \file felicaprofile.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa card profiles.
 */

#ifndef LOGICALACCESS_FELICAPROFILE_HPP
#define LOGICALACCESS_FELICAPROFILE_HPP

#include "logicalaccess/cards/accessinfo.hpp"
#include "felicalocation.hpp"
#include "logicalaccess/cards/profile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The FeliCa base profile class.
     */
	class LIBLOGICALACCESS_API FeliCaProfile : public Profile
    {
    public:

        /**
         * \brief Constructor.
         */
		FeliCaProfile();

        /**
         * \brief Destructor.
         */
		virtual ~FeliCaProfile();

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
         * \brief Set ISO15693 key at a specific location.
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
         * \brief Create default ISO15693 access informations.
         * \return Default ISO15693 access informations. Always null.
         */
        virtual std::shared_ptr<AccessInfo> createAccessInfo() const;

        /**
         * \brief Create default ISO15693 location.
         * \return Default ISO15693 location.
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
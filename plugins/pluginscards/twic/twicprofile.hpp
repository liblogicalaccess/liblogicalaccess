/**
 * \file twicprofile.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic card profiles.
 */

#ifndef LOGICALACCESS_TWIC_HPP
#define LOGICALACCESS_TWIC_HPP

#include "logicalaccess/cards/accessinfo.hpp"
#include "../iso7816/iso7816location.hpp"
#include "../iso7816/iso7816profile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The Twic base profile class.
     */
    class LIBLOGICALACCESS_API TwicProfile : public ISO7816Profile
    {
    public:

        /**
         * \brief Constructor.
         */
        TwicProfile();

        /**
         * \brief Destructor.
         */
        virtual ~TwicProfile();

        /**
         * \brief Create default Twic location.
         * \return Default Twic location.
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
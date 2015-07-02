/**
 * \file cps3profile.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief CPS3 card profiles.
 */

#ifndef LOGICALACCESS_CPS3PROFILE_HPP
#define LOGICALACCESS_CPS3PROFILE_HPP

#include "logicalaccess/cards/accessinfo.hpp"
#include "cps3location.hpp"
#include "../iso7816/iso7816location.hpp"
#include "../iso7816/iso7816profile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The CPS3 base profile class.
     */
    class LIBLOGICALACCESS_API CPS3Profile : public ISO7816Profile
    {
    public:

        /**
         * \brief Constructor.
         */
		CPS3Profile();

        /**
         * \brief Destructor.
         */
		virtual ~CPS3Profile();

        /**
         * \brief Create default CPS3 location.
         * \return Default CPS3 location.
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
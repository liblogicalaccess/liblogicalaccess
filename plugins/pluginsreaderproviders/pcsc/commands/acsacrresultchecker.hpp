/**
 * \file acsacrresultchecker.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ACS ACR Result checker.
 */

#ifndef LOGICALACCESS_ACSACRRESULTCHECKER_HPP
#define LOGICALACCESS_ACSACRRESULTCHECKER_HPP

#include "iso7816resultchecker.hpp"

namespace logicalaccess
{
    /**
     * \brief The ACS ACR implementation result checker.
     */
    class LIBLOGICALACCESS_API ACSACRResultChecker : public ISO7816ResultChecker
    {
    public:
        ACSACRResultChecker();

        virtual ~ACSACRResultChecker();
    };
}

#endif
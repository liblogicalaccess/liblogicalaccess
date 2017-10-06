/**
 * \file springcardresultchecker.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SpringCard Result checker.
 */

#ifndef LOGICALACCESS_SPRINGCARDRESULTCHECKER_HPP
#define LOGICALACCESS_SPRINGCARDRESULTCHECKER_HPP

#include "iso7816/iso7816resultchecker.hpp"

namespace logicalaccess
{
    /**
     * \brief The SpringCard implementation result checker.
     */
    class LIBLOGICALACCESS_API SpringCardResultChecker : public ISO7816ResultChecker
    {
    public:
        SpringCardResultChecker();

        virtual ~SpringCardResultChecker();
    };
}

#endif
/**
 * \file springcardresultchecker.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SpringCard Result checker.
 */

#ifndef LOGICALACCESS_SPRINGCARDRESULTCHECKER_HPP
#define LOGICALACCESS_SPRINGCARDRESULTCHECKER_HPP

#include <logicalaccess/plugins/readers/iso7816/iso7816resultchecker.hpp>
#include <logicalaccess/plugins/readers/pcsc/lla_readers_pcsc_api.hpp>

namespace logicalaccess
{
/**
 * \brief The SpringCard implementation result checker.
 */
class LLA_READERS_PCSC_API SpringCardResultChecker : public ISO7816ResultChecker
{
  public:
    SpringCardResultChecker();

    virtual ~SpringCardResultChecker();
};
}

#endif
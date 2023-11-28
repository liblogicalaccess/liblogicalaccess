/**
 * \file acsacrresultchecker.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ACS ACR Result checker.
 */

#ifndef LOGICALACCESS_ACSACRRESULTCHECKER_HPP
#define LOGICALACCESS_ACSACRRESULTCHECKER_HPP

#include <logicalaccess/plugins/readers/iso7816/iso7816resultchecker.hpp>
#include <logicalaccess/plugins/readers/pcsc/lla_readers_pcsc_api.hpp>

namespace logicalaccess
{
/**
 * \brief The ACS ACR implementation result checker.
 */
class LLA_READERS_PCSC_API ACSACRResultChecker : public ISO7816ResultChecker
{
  public:
    ACSACRResultChecker();

    virtual ~ACSACRResultChecker();
};
}

#endif
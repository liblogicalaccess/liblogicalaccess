/**
 * \file desfireiso7816resultchecker.hpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief DESFire ISO 7816 Result checker.
 */

#ifndef LOGICALACCESS_DESFIREISO7816RESULTCHECKER_HPP
#define LOGICALACCESS_DESFIREISO7816RESULTCHECKER_HPP

#include <logicalaccess/plugins/readers/iso7816/iso7816resultchecker.hpp>

namespace logicalaccess
{
/**
 * \brief The DESFire ISO 7816 result checker.
 */
class LLA_READERS_ISO7816_API DESFireISO7816ResultChecker : public ISO7816ResultChecker
{
  public:
    DESFireISO7816ResultChecker();

    virtual ~DESFireISO7816ResultChecker();
};

class LLA_READERS_ISO7816_API SetConfiguration0ResultChecker : public DESFireISO7816ResultChecker
{
  public:
    SetConfiguration0ResultChecker()
    {
        AddCheck(0x91, 0x9D,
                 "Trying to enable AuthVCMandatory while VCSelect(Enc/Mac) keys are still disabled. "
                 "Or trying to enable Mandatory Proximity Check while VCProximityKey is still disabled");
        // todo : what is "Issued at app level while targeting a convential app ?"
    }
};
}

#endif
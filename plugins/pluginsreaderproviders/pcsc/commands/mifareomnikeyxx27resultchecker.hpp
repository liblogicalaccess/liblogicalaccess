/**
 * \file mifareomnikeyxx27resultchecker.hpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief DESFire ISO 7816 Result checker.
 */

#ifndef LOGICALACCESS_MIFAREOMNIKEYXX27RESULTCHECKER_HPP
#define LOGICALACCESS_MIFAREOMNIKEYXX27RESULTCHECKER_HPP

#include "iso7816/iso7816resultchecker.hpp"

namespace logicalaccess
{
/**
 * \brief The Omnikey xx27 mifare implementation result checker.
 */
class LIBLOGICALACCESS_API MifareOmnikeyXX27ResultChecker : public ISO7816ResultChecker
{
  public:
    MifareOmnikeyXX27ResultChecker();

    virtual ~MifareOmnikeyXX27ResultChecker();
};
}

#endif
/**
 * \file iso7816resultchecker.hpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief ISO 7816 Result checker.
 */

#ifndef LOGICALACCESS_ISO7816RESULTCHECKER_HPP
#define LOGICALACCESS_ISO7816RESULTCHECKER_HPP

#include <logicalaccess/resultchecker.hpp>
#include <logicalaccess/plugins/readers/iso7816/lla_readers_iso7816_api.hpp>

namespace logicalaccess
{
/**
 * \brief The ISO 7816 result checker.
 */
class LLA_READERS_ISO7816_API ISO7816ResultChecker : public ResultChecker
{
  public:
    ISO7816ResultChecker();

    virtual ~ISO7816ResultChecker();
};
}

#endif
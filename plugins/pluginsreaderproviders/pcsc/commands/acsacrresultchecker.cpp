/**
 * \file acsacrresultchecker.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ACS ACR result checker.
 */

#include "acsacrresultchecker.hpp"

namespace logicalaccess
{
    ACSACRResultChecker::ACSACRResultChecker()
    {
        AddCheck(0x63, 0x0, "The operation failed.");
    }

    ACSACRResultChecker::~ACSACRResultChecker() {}
}
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
        AddCheck(0x63, 0x00, "The operation failed.", CardException::UNKOWN_ERROR);
    }

    ACSACRResultChecker::~ACSACRResultChecker() {}
}
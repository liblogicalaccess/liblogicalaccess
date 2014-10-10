/**
 * \file cherryreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Cherry reader unit.
 */

#include "../readers/cherryreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
    CherryReaderUnit::CherryReaderUnit(const std::string& name)
        : PCSCReaderUnit(name)
    {
    }

    CherryReaderUnit::~CherryReaderUnit()
    {
    }

    PCSCReaderUnitType CherryReaderUnit::getPCSCType() const
    {
        return PCSC_RUT_CHERRY;
    }
}
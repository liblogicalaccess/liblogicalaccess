/**
 * \file scmreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SCM reader unit.
 */

#include <logicalaccess/plugins/readers/pcsc/readers/scmreaderunit.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
SCMReaderUnit::SCMReaderUnit(const std::string &name)
    : PCSCReaderUnit(name)
{
}

SCMReaderUnit::~SCMReaderUnit()
{
}

PCSCReaderUnitType SCMReaderUnit::getPCSCType() const
{
    return PCSC_RUT_SCM;
}
}
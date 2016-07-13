/**
* \file omnikeyxx22readerunit.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Omnikey XX22 reader unit.
*/

#include "../readers/omnikeyxx22readerunit.hpp"

namespace logicalaccess
{
    OmnikeyXX22ReaderUnit::OmnikeyXX22ReaderUnit(const std::string &name)
        : OmnikeyXX21ReaderUnit(name)
    {
        
    }

    OmnikeyXX22ReaderUnit::~OmnikeyXX22ReaderUnit()
    {
    }

    PCSCReaderUnitType OmnikeyXX22ReaderUnit::getPCSCType() const
    {
        return PCSC_RUT_OMNIKEY_XX22;
    }
}

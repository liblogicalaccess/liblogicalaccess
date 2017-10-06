/**
* \file omnikeyxx23readerunit.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Omnikey XX23 reader unit.
*/

#include "../readers/omnikeyxx23readerunit.hpp"

namespace logicalaccess
{
    OmnikeyXX23ReaderUnit::OmnikeyXX23ReaderUnit(const std::string &name)
        : OmnikeyXX21ReaderUnit(name)
    {

    }

    OmnikeyXX23ReaderUnit::~OmnikeyXX23ReaderUnit()
    {
    }

    PCSCReaderUnitType OmnikeyXX23ReaderUnit::getPCSCType() const
    {
        return PCSC_RUT_OMNIKEY_XX23;
    }
}

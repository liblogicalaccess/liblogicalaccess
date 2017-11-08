/**
 * \file omnikeyxx25readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX25 reader unit.
 */

#include <logicalaccess/plugins/readers/pcsc/readers/omnikeyxx25readerunit.hpp>
#include <logicalaccess/plugins/cards/prox/proxchip.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
OmnikeyXX25ReaderUnit::OmnikeyXX25ReaderUnit(const std::string &name)
    : OmnikeyReaderUnit(name)
{
}

OmnikeyXX25ReaderUnit::~OmnikeyXX25ReaderUnit()
{
}

PCSCReaderUnitType OmnikeyXX25ReaderUnit::getPCSCType() const
{
    return PCSC_RUT_OMNIKEY_XX25;
}

ByteVector OmnikeyXX25ReaderUnit::getCardSerialNumber()
{
    return atr_;
}

std::shared_ptr<Chip> OmnikeyXX25ReaderUnit::createChip(std::string /*type*/)
{
    return std::static_pointer_cast<Chip>(std::make_shared<ProxChip>());
}
}
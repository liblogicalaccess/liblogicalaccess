/**
 * \file springcardreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SpringCard reader unit.
 */

#include "../readers/springcardreaderunit.hpp"
#include "../commands/springcardresultchecker.hpp"
#include "cardprobes/springcardprobe.hpp"

#include <iomanip>

namespace logicalaccess
{
    SpringCardReaderUnit::SpringCardReaderUnit(const std::string& name)
        : PCSCReaderUnit(name)
    {
    }

    SpringCardReaderUnit::~SpringCardReaderUnit()
    {
    }

    PCSCReaderUnitType SpringCardReaderUnit::getPCSCType() const
    {
        return PCSC_RUT_SPRINGCARD;
    }

    std::shared_ptr<ResultChecker> SpringCardReaderUnit::createDefaultResultChecker() const
    {
        return std::make_shared<SpringCardResultChecker>();
    }

    std::shared_ptr<CardProbe> SpringCardReaderUnit::createCardProbe()
    {
        return std::make_shared<SpringCardProbe>(this);
    }
}

/**
 * \file pcscreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader unit.
 */

#include <iomanip>
#include <boost/filesystem.hpp>

#include "iso7816readerunit.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "desfirechip.hpp"
#include "iso7816resultchecker.hpp"
#include "commands/desfireiso7816resultchecker.hpp"
#include "commands/samiso7816resultchecker.hpp"

namespace logicalaccess
{
    ISO7816ReaderUnit::ISO7816ReaderUnit()
        : ReaderUnit()
    {
    }

    ISO7816ReaderUnit::~ISO7816ReaderUnit() {}

    std::shared_ptr<Chip> ISO7816ReaderUnit::getSingleChip()
    {
        std::shared_ptr<Chip> ret;
        return ret;
    }

    std::vector<std::shared_ptr<Chip> > ISO7816ReaderUnit::getChipList()
    {
        std::vector<std::shared_ptr<Chip> > ret;
        return ret;
    }

std::shared_ptr<ResultChecker> ISO7816ReaderUnit::createDefaultResultChecker() const
{
    return std::make_shared<ISO7816ResultChecker>();
}
}

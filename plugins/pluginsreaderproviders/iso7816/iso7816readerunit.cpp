/**
 * \file pcscreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader unit.
 */

#include "iso7816readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"

#include "desfirechip.hpp"
#include <boost/filesystem.hpp>

#ifdef UNIX
#include <sys/time.h>
#endif

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
}
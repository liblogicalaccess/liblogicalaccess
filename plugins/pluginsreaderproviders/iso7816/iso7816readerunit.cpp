/**
 * \file pcscreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader unit.
 */

#include <iomanip>
#include <boost/filesystem.hpp>
#include <sstream>
#include <thread>

#include "iso7816readerunit.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "samav2chip.hpp"

#include "desfirechip.hpp"
#include "iso7816resultchecker.hpp"
#include "commands/desfireiso7816resultchecker.hpp"
#include "commands/samiso7816resultchecker.hpp"
#include <boost/filesystem.hpp>

#include "logicalaccess/logs.hpp"

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

bool ISO7816ReaderUnit::reconnect(int action)
{
    try
    {
        auto chip = getSingleChip();
        if (chip
            && chip->getGenericCardType() == CHIP_SAM)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            auto command = std::dynamic_pointer_cast<logicalaccess::SAMCommands<logicalaccess::KeyEntryAV2Information, logicalaccess::SETAV2 >>(chip->getCommands());
            if (command)
                command->lockUnlock(getISO7816Configuration()->getSAMUnLockKey(), logicalaccess::SAMLockUnlock::Unlock, 0, 0, 0);
        }
    }
    catch (std::exception& ex)
    {
        LOG(ERRORS) << "ISO7816ReaderUnit reconnect: " << ex.what();
        return false;
    }

    return true;
}

}

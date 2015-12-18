/**
 * \file pcscreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader unit.
 */

#include "iso7816readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>

#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "samav2chip.hpp"

#include "desfirechip.hpp"
#include <boost/filesystem.hpp>

#include "logicalaccess/logs.hpp"

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
/**
 * \file KeyStorage.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Key storage base class.
 */

#include "logicalaccess/Cards/ComputerMemoryKeyStorage.h"
#include "logicalaccess/Cards/ReaderMemoryKeyStorage.h"
#include "logicalaccess/Cards/SAMKeyStorage.h"


namespace LOGICALACCESS
{
	boost::shared_ptr<KeyStorage> KeyStorage::getKeyStorageFromType(KeyStorageType kst)
	{
		boost::shared_ptr<KeyStorage> ret;

		switch (kst)
		{
		case KST_COMPUTER_MEMORY:
			ret.reset(new ComputerMemoryKeyStorage());
			break;

		case KST_READER_MEMORY:
			ret.reset(new ReaderMemoryKeyStorage());
			break;

		case KST_SAM:
			ret.reset(new SAMKeyStorage());
			break;
		}

		return ret;
	}
}


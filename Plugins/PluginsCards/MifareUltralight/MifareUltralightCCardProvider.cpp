/**
 * \file MifareUltralightCCardProvider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C card provider.
 */

#include "MifareUltralightCCardProvider.h"
#include "MifareUltralightCChip.h"

namespace LOGICALACCESS
{	
	boost::shared_ptr<MifareUltralightCChip> MifareUltralightCCardProvider::getMifareUltralightCChip()
	{
		return boost::dynamic_pointer_cast<MifareUltralightCChip>(getMifareUltralightChip());
	}

	void MifareUltralightCCardProvider::changeKey(boost::shared_ptr<TripleDESKey> key)
	{
		if (key)
		{
			for (unsigned int i = 0; i < key->getLength(); i += 4)
			{
				getMifareUltralightCommands()->writePage(0x2C + (i / 4), key->getData() + i, 4);
			}
		}
	}

	void MifareUltralightCCardProvider::lockPage(int page)
	{
		if (page < 16)
		{
			MifareUltralightCardProvider::lockPage(page);
		}
		else
		{
			unsigned char lockbits[4];
			memset(lockbits, 0x00, sizeof(lockbits));

			if (page >= 44 && page <= 47)
			{
				lockbits[1] |= (1 << 7);
			}
			else if (page == 43)
			{
				lockbits[1] |= (1 << 6);
			}
			else if (page == 42)
			{
				lockbits[1] |= (1 << 5);
			}
			else if (page == 41)
			{
				lockbits[1] |= (1 << 4);
			}
			else if (page >= 36 && page <= 39)
			{
				lockbits[0] |= (1 << 7);
			}
			else if (page >= 32 && page <= 35)
			{
				lockbits[0] |= (1 << 6);
			}
			else if (page >= 28 && page <= 31)
			{
				lockbits[0] |= (1 << 5);
			}
			else if (page >= 24 && page <= 27)
			{
				lockbits[0] |= (1 << 3);
			}
			else if (page >= 20 && page <= 23)
			{
				lockbits[0] |= (1 << 2);
			}
			else if (page >= 16 && page <= 19)
			{
				lockbits[0] |= (1 << 1);
			}

			getMifareUltralightCommands()->writePage(0x28, lockbits, sizeof(lockbits));
		}
	}

	bool MifareUltralightCCardProvider::erase()
	{
		getMifareUltralightCCommands()->authenticate(getMifareUltralightCChip()->getMifareUltralightCProfile()->getKey());

		return MifareUltralightCardProvider::erase();
	}

	void MifareUltralightCCardProvider::authenticate(boost::shared_ptr<AccessInfo> aiToUse)
	{
		if (aiToUse)
		{
			boost::shared_ptr<MifareUltralightCAccessInfo> mAi = boost::dynamic_pointer_cast<MifareUltralightCAccessInfo>(aiToUse);
			EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument, "aiToUse must be a MifareUltralightCAccessInfo.");

			if (mAi->key)
			{
				getMifareUltralightCCommands()->authenticate(mAi->key);
			}
		}
	}

	bool MifareUltralightCCardProvider::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
	{
		authenticate(aiToUse);

		return MifareUltralightCardProvider::erase(location, aiToUse);
	}

	bool MifareUltralightCCardProvider::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		authenticate(aiToUse);

		return MifareUltralightCardProvider::writeData(location, aiToUse, aiToWrite, data, dataLength, behaviorFlags);
	}

	bool MifareUltralightCCardProvider::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		authenticate(aiToUse);

		return MifareUltralightCardProvider::readData(location, aiToUse, data, dataLength, behaviorFlags);
	}
}
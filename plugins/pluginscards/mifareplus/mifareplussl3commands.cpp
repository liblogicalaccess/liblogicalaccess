/**
 * \file mifareplussl3commands.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL3 commands.
 */

#include "mifareplussl3commands.hpp"
#include "mifarepluschip.hpp"
	
namespace logicalaccess
{
	void MifarePlusSL3Commands::writeSector(int sector, int start_block, const void* buf, size_t buflen, boost::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype)
	{
		if (buf == NULL || buflen < MIFARE_PLUS_BLOCK_SIZE || buflen % MIFARE_PLUS_BLOCK_SIZE != 0 || buflen / MIFARE_PLUS_BLOCK_SIZE + start_block > getNbBlocks(sector))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. The minimum buffer's length is 16 bytes.");
		}

		if (authenticate(sector, key, keytype))
		{
			updateBinary(getBlockNo(sector, start_block), false, true, buf, buflen);
		}
	}

	void MifarePlusSL3Commands::writeSectors(int start_sector, int stop_sector, const void* buf, size_t buflen, boost::shared_ptr<AccessInfo> aiToUse)
	{
		if (aiToUse)
		{
			int i;
			size_t test_buflen = 0;
			size_t toWriteLen = 0;
			boost::shared_ptr<MifarePlusAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);
			EXCEPTION_ASSERT(mAiToUse, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo.");

			for (i = start_sector; i < stop_sector; ++i)
			{
				test_buflen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
			}
			if (test_buflen + MIFARE_PLUS_BLOCK_SIZE != buflen)
				THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. The buffer must fit the sectors size.");

			test_buflen = 0;
			for (i = start_sector; i <= stop_sector && test_buflen < buflen; ++i)
			{
				toWriteLen = getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
				if (toWriteLen + test_buflen > buflen)
					toWriteLen = buflen - test_buflen;
				if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
					writeSector(i, 0, reinterpret_cast<const char*>(buf) + test_buflen, toWriteLen, mAiToUse->keyB, KT_KEY_AES_B);
				else if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
					writeSector(i, 0, reinterpret_cast<const char*>(buf) + test_buflen, toWriteLen, mAiToUse->keyA, KT_KEY_AES_A);
				else
					THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "You must set the writing key using the MifarePlusAccessInfo ");
				test_buflen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
			}
		}
	}

	void MifarePlusSL3Commands::readSector(int sector, int start_block, void* buf, size_t buflen, boost::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype)
	{
		if (buf == NULL || buflen < MIFARE_PLUS_BLOCK_SIZE || buflen % MIFARE_PLUS_BLOCK_SIZE != 0 || buflen / MIFARE_PLUS_BLOCK_SIZE + start_block > getNbBlocks(sector))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. The minimum buffer's length is 16 bytes.");
		}

		if (authenticate(sector, key, keytype))
		{
			readBinary(getBlockNo(sector, start_block), static_cast<unsigned char>(buflen / MIFARE_PLUS_BLOCK_SIZE), false, true, true, buf, buflen);
		}
	}

	void MifarePlusSL3Commands::readSectors(int start_sector, int stop_sector, void* buf, size_t buflen, boost::shared_ptr<AccessInfo> aiToUse)
	{
		if (aiToUse)
		{
			int i;
			size_t test_buflen = 0;
			size_t toReadLen = 0;
			boost::shared_ptr<MifarePlusAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);
			EXCEPTION_ASSERT(mAiToUse, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo.");

			for (i = start_sector; i < stop_sector; ++i)
			{
				test_buflen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
			}
			if (test_buflen + MIFARE_PLUS_BLOCK_SIZE > buflen)
				THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. The buffer must fit the sectors size.");

			test_buflen = 0;
			for (i = start_sector; i <= stop_sector && test_buflen < buflen; ++i)
			{
				toReadLen = getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
				if (toReadLen + test_buflen > buflen)
					toReadLen = buflen - test_buflen;
				if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
					readSector(i, 0, reinterpret_cast<char*>(buf) + test_buflen, toReadLen, mAiToUse->keyA, KT_KEY_AES_A);
				else if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
					readSector(i, 0, reinterpret_cast<char*>(buf) + test_buflen, toReadLen, mAiToUse->keyB, KT_KEY_AES_B);
				else
					THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "You must set the writing key using the MifarePlusAccessInfo ");
				test_buflen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
			}
		}
	}

	bool MifarePlusSL3Commands::authenticate(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> ai)
	{
		boost::shared_ptr<MifarePlusAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(ai);

		if (ai)
		{
			EXCEPTION_ASSERT(mAiToUse, std::invalid_argument, "ai must be a MifarePlusAccessInfo.");
		}
		else
		{
			mAiToUse = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(getChip()->getProfile()->createAccessInfo());
		}

		if (mAiToUse->keyConfiguration && !mAiToUse->keyConfiguration->isEmpty())
			authenticate(0, mAiToUse->keyConfiguration, KT_KEY_CONFIGURATION);
		else if (mAiToUse->keyMastercard && !mAiToUse->keyMastercard->isEmpty())
			authenticate(0, mAiToUse->keyMastercard, KT_KEY_MASTERCARD);
		if (mAiToUse->keyOriginality && !mAiToUse->keyOriginality->isEmpty())
			authenticate(0, mAiToUse->keyOriginality, KT_KEY_ORIGINALITY);
		if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
		{
			EXCEPTION_ASSERT(location, std::invalid_argument, "location cannot be null.");
			boost::shared_ptr<MifarePlusLocation> mLocation = boost::dynamic_pointer_cast<MifarePlusLocation>(location);
			EXCEPTION_ASSERT(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");
			
			authenticate(mLocation->sector, mAiToUse->keyA, KT_KEY_AES_A);
		}
		if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
		{
			EXCEPTION_ASSERT(location, std::invalid_argument, "location cannot be null.");
			boost::shared_ptr<MifarePlusLocation> mLocation = boost::dynamic_pointer_cast<MifarePlusLocation>(location);
			EXCEPTION_ASSERT(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");
			
			authenticate(mLocation->sector, mAiToUse->keyB, KT_KEY_AES_B);
		}

		return true;
	}

	unsigned char MifarePlusSL3Commands::getNbBlocks(int sector)
	{
		return ((sector >= 32) ? 15 : 3);
	}

	unsigned short MifarePlusSL3Commands::getBlockNo(int sector, int block)
	{
		unsigned short blockno = 0x0000;
		int i;

		for (i = 0; i < sector; ++i)
		{
			blockno += getNbBlocks(i) + 1;
		}
		blockno += static_cast<unsigned short>(block);

		return blockno;
	}
}
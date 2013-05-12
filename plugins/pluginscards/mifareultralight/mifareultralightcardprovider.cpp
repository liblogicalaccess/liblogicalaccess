/**
 * \file mifareultralightcardprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight card provider.
 */

#include "mifareultralightcardprovider.hpp"
#include "mifareultralightchip.hpp"


namespace logicalaccess
{
	bool MifareUltralightCardProvider::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
	{
		boost::shared_ptr<MifareUltralightLocation> mLocation = boost::dynamic_pointer_cast<MifareUltralightLocation>(location);
		if (!mLocation)
		{
			return false;
		}

		unsigned char zeroblock[4];
		memset(zeroblock, 0x00, sizeof(zeroblock));

		return writeData(location, aiToUse, boost::shared_ptr<AccessInfo>(), zeroblock, sizeof(zeroblock), CB_DEFAULT);
	}	

	bool MifareUltralightCardProvider::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> /*aiToWrite*/, const void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		bool ret = false;

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<MifareUltralightLocation> mLocation = boost::dynamic_pointer_cast<MifareUltralightLocation>(location);
		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareUltralightLocation.");
		boost::shared_ptr<MifareUltralightAccessInfo> mAi = boost::dynamic_pointer_cast<MifareUltralightAccessInfo>(location);

		size_t totaldatalen = dataLength + mLocation->byte;
		int nbPages = 0;
		size_t buflen = 0;
		while (buflen < totaldatalen)
		{
			buflen += 4;
			nbPages++;
		}
	
		if (nbPages >= 1)
		{
			std::vector<unsigned char> dataPages;
			dataPages.resize(buflen, 0x00);
			memcpy(&dataPages[0] + mLocation->byte, data, dataLength);

			size_t reallen;			

			if (behaviorFlags & CB_AUTOSWITCHAREA)
			{
				reallen = writePages(mLocation->page,
					mLocation->page + nbPages - 1,
					&dataPages[0],
					dataPages.size()
				);
			}
			else
			{
				reallen = getMifareUltralightCommands()->writePage(mLocation->page,
					&dataPages[0],
					dataPages.size()
				);
			}

			ret = (reallen >= buflen);

			if (ret && mAi)
			{
				if (mAi->lockPage)
				{
					for (int i = mLocation->page; i < mLocation->page + nbPages; ++i)
					{
						lockPage(i);
					}
				}
			}
		}

		return ret;
	}

	bool MifareUltralightCardProvider::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		bool ret = false;

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<MifareUltralightLocation> mLocation = boost::dynamic_pointer_cast<MifareUltralightLocation>(location);
		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");

		size_t totaldatalen = dataLength + mLocation->byte;
		int nbPages = 0;
		size_t buflen = 0;
		while (buflen < totaldatalen)
		{
			buflen += 4;
			nbPages++;
		}

		if (nbPages >= 1)
		{
			std::vector<unsigned char> dataPages;
			dataPages.resize(buflen, 0x00);
			size_t reallen;

			if (behaviorFlags & CB_AUTOSWITCHAREA)
			{
				reallen = readPages(mLocation->page, mLocation->page + nbPages - 1, dataPages.data(), dataPages.size()); 
			}
			else
			{
				reallen = getMifareUltralightCommands()->readPage(mLocation->page, dataPages.data(), dataPages.size()); 
			}

			if (dataLength <= (reallen - mLocation->byte))
			{
				memcpy(static_cast<char*>(data), &dataPages[0] + mLocation->byte, dataLength);

				ret = true;
			}
		}

		return ret;
	}

	size_t MifareUltralightCardProvider::readDataHeader(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
	{
		
		return 0;
	}	

	boost::shared_ptr<MifareUltralightChip> MifareUltralightCardProvider::getMifareUltralightChip()
	{
		return boost::dynamic_pointer_cast<MifareUltralightChip>(getChip());
	}

	bool MifareUltralightCardProvider::erase()
	{
		unsigned char zeroblock[4];

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasWriteDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		memset(zeroblock, 0x00, sizeof(zeroblock));		
		bool result = false;

		for (unsigned int i = 4; i < 16; ++i)
		{
			bool erased = (getMifareUltralightCommands()->writePage(i, zeroblock, sizeof(zeroblock)) == 4);
			
			if (!erased)
			{
				return false;
			} else
			{
				result = true;
			}
		}

		return result;
	}

	size_t MifareUltralightCardProvider::readPages(int start_page, int stop_page, void* buf, size_t buflen)
	{
		if (start_page > stop_page)
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start page can't be greater than stop page.");
		}

		size_t minsize = 0;
		size_t offset = 0;

		for (int i = start_page; i <= stop_page; ++i)
		{
			minsize += 4;
			if (buflen < minsize)
			{
				THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The buffer is too short.");
			}

			offset += getMifareUltralightCommands()->readPage(i, reinterpret_cast<char*>(buf) + offset, 4);
		}

		return minsize;
	}

	size_t MifareUltralightCardProvider::writePages(int start_page, int stop_page, const void* buf, size_t buflen)
	{
		if (start_page > stop_page)
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start page can't be greater than stop page.");
		}

		size_t minsize = 0;
		size_t offset = 0;

		for (int i = start_page; i <= stop_page; ++i)
		{
			minsize += 4;
			if (buflen < minsize)
			{
				THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The buffer is too short.");
			}

			offset += getMifareUltralightCommands()->writePage(i, reinterpret_cast<const char*>(buf) + offset, 4);
		}

		return minsize;
	}

	void MifareUltralightCardProvider::lockPage(int page)
	{
		unsigned char lockbits[4];
		memset(lockbits, 0x00, sizeof(lockbits));

		if (page >=3 && page <= 7)
		{
			lockbits[2] |= static_cast<unsigned char>(1 << page);
		}
		else if (page >= 8 && page <= 15)
		{
			lockbits[3] |= static_cast<unsigned char>(1 << (page - 8));
		}

		getMifareUltralightCommands()->writePage(2, lockbits, sizeof(lockbits));
	}
}

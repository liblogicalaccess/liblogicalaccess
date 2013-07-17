/**
 * \file mifareprofile.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Mifare card profiles.
 */

#include "mifareprofile.hpp"
#include "mifarecardprovider.hpp"

namespace logicalaccess
{	
	MifareProfile::MifareProfile()
		: Profile()
	{
		d_key = new boost::shared_ptr<MifareKey>[(getNbSectors()+1)*2];

		clearKeys();
	}

	MifareProfile::~MifareProfile()
	{
		delete[] d_key;
	}

	unsigned int MifareProfile::getNbSectors() const
	{
		return 40;
	}

	void MifareProfile::setDefaultKeys()
	{
		for (unsigned int i = 0; i < getNbSectors(); i++)
		{
			setDefaultKeysAt(i);
		}
	}

	void MifareProfile::setDefaultKeysAt(boost::shared_ptr<Location> location)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		boost::shared_ptr<MifareLocation> mLocation = boost::dynamic_pointer_cast<MifareLocation>(location);
		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");

		if (mLocation->sector != -1)
		{
			setDefaultKeysAt(mLocation->sector);
		}
	}

	void MifareProfile::clearKeys()
	{
		for (unsigned int i = 0; i <= getNbSectors(); i++)
		{			
			d_key[i * 2].reset();
			d_key[i * 2 +1].reset();
		}
	}

	boost::shared_ptr<MifareKey> MifareProfile::getKey(int index, MifareKeyType keytype) const
	{
		if (index > static_cast<int>(getNbSectors()))
		{
			return boost::shared_ptr<MifareKey>(new MifareKey());
		}

		switch (keytype)
		{
			case KT_KEY_A:
				{
					return d_key[index * 2];
				}
			case KT_KEY_B:
				{
					return d_key[index * 2 + 1];
				}
			default:
				{
					return boost::shared_ptr<MifareKey>(new MifareKey());
				}
		}
	}

	void MifareProfile::setKey(int index, MifareKeyType keytype, boost::shared_ptr<MifareKey> key)
	{		
		if (index > static_cast<int>(getNbSectors()))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
		}

		switch (keytype)
		{
			case KT_KEY_A:
				{
					d_key[index * 2] = key;

					break;
				}
			case KT_KEY_B:
				{
					d_key[index * 2 + 1] = key;

					break;
				}
		}
	}

	bool MifareProfile::getKeyUsage(int index, MifareKeyType keytype) const
	{
		if (index > static_cast<int>(getNbSectors()))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
		}

		switch (keytype)
		{
			case KT_KEY_A:
				{
					return bool(d_key[index * 2]);
				}
			case KT_KEY_B:
				{
					return bool(d_key[index * 2 + 1]);
				}
			default:
				{
					return false;
				}
		}
	}

	void MifareProfile::setKeyUsage(int index, MifareKeyType keytype, bool used)
	{
		if (index > static_cast<int>(getNbSectors()))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
		}

		boost::shared_ptr<MifareKey> key;
		if (used)
		{
			key.reset(new MifareKey());
		}

		switch (keytype)
		{
			case KT_KEY_A:
				{
					d_key[index * 2] = key;
				}
			case KT_KEY_B:
				{
					d_key[index * 2 + 1] = key;
				}
		}
	}

	void MifareProfile::setDefaultKeysAt(int index)
	{
		d_key[index * 2].reset(new MifareKey());

		d_key[index * 2 +1].reset(new MifareKey());
	}

	void MifareProfile::setKeyAt(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> AccessInfo)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(AccessInfo, std::invalid_argument, "AccessInfo cannot be null.");

		boost::shared_ptr<MifareLocation> mLocation = boost::dynamic_pointer_cast<MifareLocation>(location);
		boost::shared_ptr<MifareAccessInfo> mAi = boost::dynamic_pointer_cast<MifareAccessInfo>(AccessInfo);

		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
		EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument, "AccessInfo must be a MifareAccessInfo.");

		if (!mAi->keyA->isEmpty())
		{
			setKey(mLocation->sector, KT_KEY_A, mAi->keyA);
		}

		if (!mAi->keyB->isEmpty())
		{
			setKey(mLocation->sector, KT_KEY_B, mAi->keyB);
		}
	}

	boost::shared_ptr<AccessInfo> MifareProfile::createAccessInfo() const
	{
		boost::shared_ptr<MifareAccessInfo> ret;
		ret.reset(new MifareAccessInfo());
		return ret;
	}

	boost::shared_ptr<Location> MifareProfile::createLocation() const
	{
		boost::shared_ptr<MifareLocation> ret;
		ret.reset(new MifareLocation());
		return ret;
	}

	FormatList MifareProfile::getSupportedFormatList()
	{
		return Profile::getHIDWiegandFormatList();
	}
}


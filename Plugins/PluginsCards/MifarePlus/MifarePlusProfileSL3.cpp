/**
 * \file MifarePlusProfileSL3.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus card profiles.
 */

#include "MifarePlusProfileSL3.h"


#include "MifarePlusCardProviderSL3.h"

namespace logicalaccess
{	
	MifarePlusProfileSL3::MifarePlusProfileSL3()
		: MifarePlusProfile()
	{
		d_sectorKeys = new boost::shared_ptr<MifarePlusKey>[(getNbSectors()+1)*2];
		for (unsigned int i = 0; i <= getNbSectors(); i++)
		{			
			d_sectorKeys[i * 2] = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
			d_sectorKeys[i * 2 + 1] = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		}
		d_originalityKey = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		d_masterCardKey = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		d_configurationKey = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));

		clearKeys();
	}

	MifarePlusProfileSL3::~MifarePlusProfileSL3()
	{
		delete[] d_sectorKeys;
	}

	void MifarePlusProfileSL3::clearKeys()
	{
		for (unsigned int i = 0; i <= getNbSectors(); i++)
		{			
			d_sectorKeys[i * 2].reset();
			d_sectorKeys[i * 2 +1].reset();
		}
		d_originalityKey.reset();
		d_masterCardKey.reset();
		d_configurationKey.reset();
	}

	boost::shared_ptr<MifarePlusKey> MifarePlusProfileSL3::getKey(int index, MifarePlusKeyType keytype) const
	{
		if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_AES_A || keytype == KT_KEY_AES_B))
		{
			return boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		}

		switch (keytype)
		{
			case KT_KEY_AES_A:
				{
					return d_sectorKeys[index * 2];
				}
			case KT_KEY_AES_B:
				{
					return d_sectorKeys[index * 2 + 1];
				}
			case KT_KEY_ORIGINALITY:
				{
					return d_originalityKey;
				}
			case KT_KEY_MASTERCARD:
				{
					return d_masterCardKey;
				}
			case KT_KEY_CONFIGURATION:
				{
					return d_configurationKey;
				}
			default:
				{
					return boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
				}
		}
	}

	void MifarePlusProfileSL3::setKey(int index, MifarePlusKeyType keytype, boost::shared_ptr<MifarePlusKey> /*key*/)
	{		
		if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_AES_A || keytype == KT_KEY_AES_B))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
		}

		/*		switch (keytype)
		{
			case KT_KEY_AES_A:
				{
					d_sectorKeys[index * 2] = key;

					break;
				}
			case KT_KEY_AES_B:
				{
					d_sectorKeys[index * 2 + 1] = key;

					break;
				}
			case KT_KEY_ORIGINALITY:
				{
					d_originalityKey = key;

					break;
				}
			case KT_KEY_MASTERCARD:
				{
					d_masterCardKey = key;

					break;
				}
			case KT_KEY_CONFIGURATION:
				{
					d_configurationKey = key;

					break;
				}
				}*/
	}

	bool MifarePlusProfileSL3::getKeyUsage(int index, MifarePlusKeyType keytype) const
	{
		if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_AES_A || keytype == KT_KEY_AES_B))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
		}

		switch (keytype)
		{
			case KT_KEY_AES_A:
				{
					return (d_sectorKeys[index * 2]);
				}
			case KT_KEY_AES_B:
				{
					return (d_sectorKeys[index * 2 + 1]);
				}
			case KT_KEY_ORIGINALITY:
				{
					return (d_originalityKey);
				}
			case KT_KEY_MASTERCARD:
				{
					return (d_masterCardKey);
				}
			case KT_KEY_CONFIGURATION:
				{
					return (d_configurationKey);
				}
			default:
				{
					return false;
				}
		}
	}

	void MifarePlusProfileSL3::setKeyUsage(int index, MifarePlusKeyType keytype, bool used)
	{
		if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_AES_A || keytype == KT_KEY_AES_B))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
		}

		boost::shared_ptr<MifarePlusKey> key;

		if (used)
			key = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));

		/*		switch (keytype)
		{
			case KT_KEY_AES_A:
				{
					d_sectorKeys[index * 2] = key;
				}
			case KT_KEY_AES_B:
				{
					d_sectorKeys[index * 2 + 1] = key;
				}
			case KT_KEY_ORIGINALITY:
				{
					d_originalityKey = key;
				}
			case KT_KEY_MASTERCARD:
				{
					d_masterCardKey = key;
				}
			case KT_KEY_CONFIGURATION:
				{
					d_configurationKey = key;
				}
				}*/
	}

	void MifarePlusProfileSL3::setDefaultKeysAt(int index)
	{
		d_sectorKeys[index * 2].reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));

		d_sectorKeys[index * 2 + 1].reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
	}

	void MifarePlusProfileSL3::setDefaultKeys()
	{
		for (unsigned int i = 0; i < getNbSectors(); i++)
		{
			setDefaultKeysAt(i);
		}
		d_originalityKey.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
		d_masterCardKey.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
		d_configurationKey.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
	}

	void MifarePlusProfileSL3::setKeyAt(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> AccessInfo)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(AccessInfo, std::invalid_argument, "AccessInfo cannot be null.");

		boost::shared_ptr<MifarePlusLocation> dfLocation = boost::dynamic_pointer_cast<MifarePlusLocation>(location);
		boost::shared_ptr<MifarePlusAccessInfo> dfAi = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(AccessInfo);

		EXCEPTION_ASSERT_WITH_LOG(dfLocation, std::invalid_argument, "location must be a MifareLocation.");
		EXCEPTION_ASSERT_WITH_LOG(dfAi, std::invalid_argument, "AccessInfo must be a MifareAccessInfo.");

		if (!dfAi->keyA->isEmpty())
		{
			setKey(dfLocation->sector, KT_KEY_AES_A, dfAi->keyA);
		}
		if (!dfAi->keyB->isEmpty())
		{
			setKey(dfLocation->sector, KT_KEY_AES_B, dfAi->keyB);
		}
		if (!dfAi->keyOriginality->isEmpty())
		{
			setKey(0, KT_KEY_ORIGINALITY, dfAi->keyOriginality);
		}
		if (!dfAi->keyMastercard->isEmpty())
		{
			setKey(0, KT_KEY_MASTERCARD, dfAi->keyMastercard);
		}
		if (!dfAi->keyConfiguration->isEmpty())
		{
			setKey(0, KT_KEY_CONFIGURATION, dfAi->keyConfiguration);
		}
	}

	boost::shared_ptr<AccessInfo> MifarePlusProfileSL3::createAccessInfo() const
	{
		boost::shared_ptr<MifarePlusAccessInfo> ret;
		ret.reset(new MifarePlusAccessInfo(MIFARE_PLUS_AES_KEY_SIZE));
		return ret;
	}
}

/**
 * \file mifareplusprofilesl1.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus card profiles.
 */

#include "mifareplusprofilesl1.hpp"


#include "mifarepluscardprovidersl1.hpp"

namespace logicalaccess
{	
	MifarePlusProfileSL1::MifarePlusProfileSL1()
		: MifarePlusProfile()
	{
		d_sectorKeys = new boost::shared_ptr<MifarePlusKey>[(getNbSectors()+1)*2];
		for (unsigned int i = 0; i <= getNbSectors(); i++)
		{			
			d_sectorKeys[i * 2] = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_CRYPTO1_KEY_SIZE));
			d_sectorKeys[i * 2 +1] = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_CRYPTO1_KEY_SIZE));
		}
		d_switchL2Key = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		d_switchL3Key = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		d_SL1AesKey = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));

		clearKeys();
	}

	MifarePlusProfileSL1::~MifarePlusProfileSL1()
	{
		delete[] d_sectorKeys;
	}

	void MifarePlusProfileSL1::clearKeys()
	{
		for (unsigned int i = 0; i <= getNbSectors(); i++)
		{			
			d_sectorKeys[i * 2].reset();
			d_sectorKeys[i * 2 +1].reset();
		}
		d_switchL2Key.reset();
		d_switchL3Key.reset();
		d_SL1AesKey.reset();
	}

	boost::shared_ptr<MifarePlusKey> MifarePlusProfileSL1::getKey(int index, MifarePlusKeyType keytype) const
	{
		if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_CRYPTO1_A || keytype == KT_KEY_CRYPTO1_B))
		{
			return boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_CRYPTO1_KEY_SIZE));
		}

		switch (keytype)
		{
			case KT_KEY_CRYPTO1_A:
				{
					return d_sectorKeys[index * 2];
				}
			case KT_KEY_CRYPTO1_B:
				{
					return d_sectorKeys[index * 2 + 1];
				}
			case KT_KEY_SWITCHL2:
				{
					return d_switchL2Key;
				}
			case KT_KEY_SWITCHL3:
				{
					return d_switchL3Key;
				}
			case KT_KEY_AESSL1:
				{
					return d_SL1AesKey;
				}
			default:
				{
					return boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
				}
		}
	}

	void MifarePlusProfileSL1::setKey(int index, MifarePlusKeyType keytype, boost::shared_ptr<MifarePlusKey> /*key*/)
	{		
		if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_CRYPTO1_A || keytype == KT_KEY_CRYPTO1_B))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
		}

		/*		switch (keytype)
		{
			case KT_KEY_CRYPTO1_A:
				{
					d_sectorKeys[index * 2] = key;

					break;
				}
			case KT_KEY_CRYPTO1_B:
				{
					d_sectorKeys[index * 2 + 1] = key;

					break;
				}
			case KT_KEY_SWITCHL2:
				{
					d_switchL2Key = key;

					break;
				}
			case KT_KEY_SWITCHL3:
				{
					d_switchL3Key = key;

					break;
				}
			case KT_KEY_AESSL1:
				{
					d_SL1AesKey = key;

					break;
				}
				}*/
	}

	bool MifarePlusProfileSL1::getKeyUsage(int index, MifarePlusKeyType keytype) const
	{
		if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_CRYPTO1_A || keytype == KT_KEY_CRYPTO1_B))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
		}

		switch (keytype)
		{
			case KT_KEY_CRYPTO1_A:
				{
					return (d_sectorKeys[index * 2]);
				}
			case KT_KEY_CRYPTO1_B:
				{
					return (d_sectorKeys[index * 2 + 1]);
				}
			case KT_KEY_SWITCHL2:
				{
					return (d_switchL2Key);
				}
			case KT_KEY_SWITCHL3:
				{
					return (d_switchL3Key);
				}
			case KT_KEY_AESSL1:
				{
					return (d_SL1AesKey);
				}
			default:
				{
					return false;
				}
		}
	}

	void MifarePlusProfileSL1::setKeyUsage(int index, MifarePlusKeyType keytype, bool used)
	{
		if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_CRYPTO1_A || keytype == KT_KEY_CRYPTO1_B))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
		}

		boost::shared_ptr<MifarePlusKey> key;
		if (used && (keytype == KT_KEY_CRYPTO1_A || keytype == KT_KEY_CRYPTO1_B))
		{
			key = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_CRYPTO1_KEY_SIZE));
		}
		else if (used)
		{
			key = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		}

		/*		switch (keytype)
		{
			case KT_KEY_CRYPTO1_A:
				{
					d_sectorKeys[index * 2] = key;
				}
			case KT_KEY_CRYPTO1_B:
				{
					d_sectorKeys[index * 2 + 1] = key;
				}
			case KT_KEY_SWITCHL2:
				{
					d_switchL2Key = key;
				}
			case KT_KEY_SWITCHL3:
				{
					d_switchL3Key = key;
				}
			case KT_KEY_AESSL1:
				{
					d_SL1AesKey = key;
				}
				}*/
	}

	void MifarePlusProfileSL1::setDefaultKeysAt(int index)
	{
		d_sectorKeys[index * 2].reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_CRYPTO1KEY, MIFARE_PLUS_CRYPTO1_KEY_SIZE, MIFARE_PLUS_CRYPTO1_KEY_SIZE));

		d_sectorKeys[index * 2 +1].reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_CRYPTO1KEY, MIFARE_PLUS_CRYPTO1_KEY_SIZE, MIFARE_PLUS_CRYPTO1_KEY_SIZE));
	}

	void MifarePlusProfileSL1::setDefaultKeys()
	{
		for (unsigned int i = 0; i < getNbSectors(); i++)
		{
			setDefaultKeysAt(i);
		}
		d_switchL2Key.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
		d_switchL3Key.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
		d_SL1AesKey.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
	}

	void MifarePlusProfileSL1::setKeyAt(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> AccessInfo)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(AccessInfo, std::invalid_argument, "AccessInfo cannot be null.");

		boost::shared_ptr<MifarePlusLocation> dfLocation = boost::dynamic_pointer_cast<MifarePlusLocation>(location);
		boost::shared_ptr<MifarePlusAccessInfo> dfAi = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(AccessInfo);

		EXCEPTION_ASSERT_WITH_LOG(dfLocation, std::invalid_argument, "location must be a MifareLocation.");
		EXCEPTION_ASSERT_WITH_LOG(dfAi, std::invalid_argument, "AccessInfo must be a MifareAccessInfo.");

		if (!dfAi->keyA->isEmpty())
		{
			setKey(dfLocation->sector, KT_KEY_CRYPTO1_A, dfAi->keyA);
		}
		if (!dfAi->keyB->isEmpty())
		{
			setKey(dfLocation->sector, KT_KEY_CRYPTO1_B, dfAi->keyB);
		}
		if (!dfAi->keySwitchL2->isEmpty())
		{
			setKey(0, KT_KEY_SWITCHL2, dfAi->keySwitchL2);
		}
		if (!dfAi->keySwitchL3->isEmpty())
		{
			setKey(0, KT_KEY_SWITCHL3, dfAi->keySwitchL3);
		}
		if (!dfAi->keyAuthenticateSL1AES->isEmpty())
		{
			setKey(0, KT_KEY_AESSL1, dfAi->keyAuthenticateSL1AES);
		}
	}

	boost::shared_ptr<AccessInfo> MifarePlusProfileSL1::createAccessInfo() const
	{
		boost::shared_ptr<MifarePlusAccessInfo> ret;
		ret.reset(new MifarePlusAccessInfo(MIFARE_PLUS_CRYPTO1_KEY_SIZE));
		return ret;
	}
}

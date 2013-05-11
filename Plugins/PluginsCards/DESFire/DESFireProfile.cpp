/**
 * \file DESFireProfile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire card profiles.
 */

#include "DESFireProfile.h"
#include "DESFireCrypto.h"

namespace logicalaccess
{
	DESFireProfile::DESFireProfile() : 
		Profile()
	{
		clearKeys();			
	}

	DESFireProfile::~DESFireProfile()
	{

	}

	void DESFireProfile::setDefaultKeys()
	{
		setKey(0, 0, boost::shared_ptr<DESFireKey>(new DESFireKey(string("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"))));
	}

	void DESFireProfile::setDefaultKeysAt(boost::shared_ptr<Location> location)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		boost::shared_ptr<DESFireLocation> dfLocation = boost::dynamic_pointer_cast<DESFireLocation>(location);
		EXCEPTION_ASSERT_WITH_LOG(dfLocation, std::invalid_argument, "location must be a DESFireLocation.");

		// Application (File keys are Application keys)
		if (dfLocation->aid != -1)
		{
			for (unsigned char i = 0; i < 14; ++i)
			{
				setKey(dfLocation->aid, i, getDefaultKey(DF_KEY_DES));
			}
		}
		// Card
		else
		{
			setKey(0, 0, getDefaultKey(DF_KEY_DES));
		}
	}

	void DESFireProfile::clearKeys()
	{
		d_nbAids = 0;
	}

	bool DESFireProfile::getKeyUsage(size_t index) const
	{	
		size_t nbKeys = sizeof(d_key) / sizeof(boost::shared_ptr<DESFireKey>);
		if (index > nbKeys)
		{
			return false;
		}

		return (d_key[index]);
	}

	void DESFireProfile::setKeyUsage(size_t index, bool used)
	{
		size_t nbKeys = sizeof(d_key) / sizeof(boost::shared_ptr<DESFireKey>);
		if (index > nbKeys)
		{
			return;
		}

		boost::shared_ptr<DESFireKey> key;
		if (used)
		{
			key.reset(new DESFireKey());
		}

		d_key[index] = key;
	}

	bool DESFireProfile::getPosAid(size_t aid, size_t* pos) const
	{
		bool ret = false;		
		size_t i;

		for (i = 0; i < d_nbAids && !ret; i++)
		{
			ret = (d_aids[i] == aid);
		}

		if (ret && pos != NULL)
		{
			*pos = i-1;
		}

		return ret;
	}

	size_t DESFireProfile::addPosAid(size_t aid)
	{
		d_aids[d_nbAids] = aid;
		return d_nbAids++;
	}

	bool DESFireProfile::checkKeyPos(size_t aid, bool defvalue) const
	{
		bool ret = false;

		if (getPosAid(aid))
		{
			ret = true;
		}
		else
		{
			ret = defvalue;
		}

		return ret;
	}

	bool DESFireProfile::checkKeyPos(size_t aid, unsigned char keyno, bool defvalue) const
	{
		if (!checkKeyPos(aid, defvalue) || keyno > 13)
		{
			return false;
		}

		return true;
	}

	boost::shared_ptr<DESFireKey> DESFireProfile::getDefaultKey(DESFireKeyType keyType)
	{
		boost::shared_ptr<DESFireKey> key(new DESFireKey());

		key->setKeyType(keyType);
		std::vector<unsigned char> buf;
		buf.resize(key->getLength(), 0x00);
		key->setData(buf);

		return key;
	}

	boost::shared_ptr<DESFireKey> DESFireProfile::getKey(size_t aid, unsigned char keyno) const
	{
		boost::shared_ptr<DESFireKey> key;

		if (checkKeyPos(aid, keyno, false))
		{
			size_t posAid;
			getPosAid(aid, &posAid);

			size_t keyPos = 1 + posAid * 14 + keyno;
		
			if (getKeyUsage(keyPos))
			{
				key = d_key[keyPos];
			}
		}

		if (!key)
		{
			key = DESFireProfile::getDefaultKey(DF_KEY_DES);
		}

		return key;
	}



	bool DESFireProfile::getKey(size_t aid, unsigned char keyno, unsigned char* diversify, std::vector<unsigned char>& keydiv)
	{
		if (!checkKeyPos(aid, keyno, false))
		{
			keydiv.resize(16, 0x00);

			return false;
		}

		size_t posAid;
		getPosAid(aid, &posAid);

		size_t keyPos = 1 + posAid * 14 + keyno;
		
		if (!getKeyUsage(keyPos))
		{
			keydiv.resize(16, 0x00);

			return false;
		}

		DESFireCrypto::getKey(d_key[keyPos], diversify, keydiv);
		
		return true;
	}	

	void DESFireProfile::setKey(size_t aid, unsigned char keyno, boost::shared_ptr<DESFireKey> key)
	{
		if (!checkKeyPos(aid, keyno, true))
		{
			return;
		}

		size_t posAid;
		if (!getPosAid(aid, &posAid))
		{
			posAid = addPosAid(aid);
		}

		size_t keyPos = 1 + posAid*14 + keyno;

		d_key[keyPos] = key;
	}
	
	void DESFireProfile::setKeyAt(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> AccessInfo)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(AccessInfo, std::invalid_argument, "AccessInfo cannot be null.");

		boost::shared_ptr<DESFireLocation> dfLocation = boost::dynamic_pointer_cast<DESFireLocation>(location);
		boost::shared_ptr<DESFireAccessInfo> dfAi = boost::dynamic_pointer_cast<DESFireAccessInfo>(AccessInfo);

		EXCEPTION_ASSERT_WITH_LOG(dfLocation, std::invalid_argument, "location must be a DESFireLocation.");
		EXCEPTION_ASSERT_WITH_LOG(dfAi, std::invalid_argument, "AccessInfo must be a DESFireAccessInfo.");

		if (!dfAi->readKey->isEmpty())
		{
			setKey(dfLocation->aid, dfAi->readKeyno, dfAi->readKey);
		}

		if (!dfAi->writeKey->isEmpty())
		{
			setKey(dfLocation->aid, dfAi->writeKeyno, dfAi->writeKey);
		}

		if (!dfAi->masterApplicationKey->isEmpty())
		{
			setKey(dfLocation->aid, 0, dfAi->masterApplicationKey);
		}

		if (!dfAi->masterCardKey->isEmpty())
		{
			setKey(0, 0, dfAi->masterCardKey);
		}
	}

	boost::shared_ptr<AccessInfo> DESFireProfile::createAccessInfo() const
	{
		boost::shared_ptr<DESFireAccessInfo> ret;
		ret.reset(new DESFireAccessInfo());
		return ret;
	}

	boost::shared_ptr<Location> DESFireProfile::createLocation() const
	{
		boost::shared_ptr<DESFireLocation> ret;
		ret.reset(new DESFireLocation());
		return ret;
	}	

	FormatList DESFireProfile::getSupportedFormatList()
	{
		return Profile::getHIDWiegandFormatList();
	}
}

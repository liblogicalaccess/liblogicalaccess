/**
 * \file MifareUltralightCProfile.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Mifare Ultralight C card profiles.
 */

#include "MifareUltralightCProfile.h"

#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"
#include "MifareUltralightLocation.h"
#include "MifareUltralightCAccessInfo.h"

namespace LOGICALACCESS
{
	MifareUltralightCProfile::MifareUltralightCProfile()
		: MifareUltralightProfile()
	{
		clearKeys();
	}

	MifareUltralightCProfile::~MifareUltralightCProfile()
	{
	}

	void MifareUltralightCProfile::setDefaultKeys()
	{
		d_key.reset(new TripleDESKey());
	}

	void MifareUltralightCProfile::clearKeys()
	{
		setDefaultKeys();
	}

	void MifareUltralightCProfile::setKeyAt(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> AccessInfo)
	{
		EXCEPTION_ASSERT_WITH_LOG(AccessInfo, std::invalid_argument, "AccessInfo cannot be null.");

		boost::shared_ptr<MifareUltralightCAccessInfo> mAi = boost::dynamic_pointer_cast<MifareUltralightCAccessInfo>(AccessInfo);
		EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument, "AccessInfo must be a MifareUltralightCAccessInfo.");

		d_key = mAi->key;
	}

	void MifareUltralightCProfile::setKey(boost::shared_ptr<TripleDESKey> key)
	{
		d_key = key;
	}

	boost::shared_ptr<TripleDESKey> MifareUltralightCProfile::getKey() const
	{
		return d_key;
	}

	boost::shared_ptr<AccessInfo> MifareUltralightCProfile::createAccessInfo() const
	{
		boost::shared_ptr<MifareUltralightCAccessInfo> ret;
		ret.reset(new MifareUltralightCAccessInfo());
		return ret;
	}
}

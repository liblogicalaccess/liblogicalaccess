/**
 * \file iso7816cardprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 card profiles.
 */

#include "iso7816cardprovider.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "iso7816location.hpp"
#include "iso7816chip.hpp"

namespace logicalaccess
{
	ISO7816CardProvider::ISO7816CardProvider()
		: CardProvider()
	{

	}

	ISO7816CardProvider::~ISO7816CardProvider()
	{

	}

	bool ISO7816CardProvider::erase()
	{
		return false;
	}

	bool ISO7816CardProvider::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/)
	{
		bool ret = false;
		
#ifdef _LICENSE_SYSTEM
		if (!d_license.hasWriteDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		boost::shared_ptr<ISO7816Location> icLocation = boost::dynamic_pointer_cast<ISO7816Location>(location);

		EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a ISO7816Location.");

		if (icLocation->fileid != 0)
		{
			getISO7816Commands()->selectFile(icLocation->fileid);
		}
		else
		{
			unsigned char defaultdf[16];
			memset(defaultdf, 0x00, sizeof(defaultdf));
			if (memcmp(icLocation->dfname, defaultdf, sizeof(defaultdf)))
			{
				getISO7816Commands()->selectFile(icLocation->dfname, icLocation->dfnamelen);
			}
		}

		switch (icLocation->fileType)
		{		
		case IFT_TRANSPARENT:
			{
				getISO7816Commands()->eraseBinay(0);
				ret = true;
			}
			break;

		case IFT_LINEAR_FIXED:
		case IFT_LINEAR_VARIABLE:
		case IFT_CYCLIC:
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented yet.");
			break;

		default:
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Doesn't know how to write on this file.");
			break;
		}

		return ret;
	}
	
	bool ISO7816CardProvider::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> /*aiToWrite*/, const void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
	{
		bool ret = false;

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasWriteDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<ISO7816Location> icLocation = boost::dynamic_pointer_cast<ISO7816Location>(location);

		EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a ISO7816Location.");

		if (icLocation->fileid != 0)
		{
			getISO7816Commands()->selectFile(icLocation->fileid);
		}
		else
		{
			unsigned char defaultdf[16];
			memset(defaultdf, 0x00, sizeof(defaultdf));
			if (memcmp(icLocation->dfname, defaultdf, sizeof(defaultdf)))
			{
				getISO7816Commands()->selectFile(icLocation->dfname, icLocation->dfnamelen);
			}
		}

		switch (icLocation->fileType)
		{
		case IFT_MASTER:
		case IFT_DIRECTORY:
			{
				if (icLocation->dataObject > 0)
				{
					getISO7816Commands()->putData(data, dataLength, icLocation->dataObject);
					ret = true;
				}
				else
				{
					THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Please specify a data object.");
				}
			}
			break;

		case IFT_TRANSPARENT:
			{
				getISO7816Commands()->writeBinay(data, dataLength, 0);
				ret = true;
			}
			break;

		case IFT_LINEAR_FIXED:
		case IFT_LINEAR_VARIABLE:
		case IFT_CYCLIC:
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented yet.");
			break;

		default:
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Doesn't know how to write on this file.");
			break;
		}

		return ret;
	}

	bool ISO7816CardProvider::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
	{
		bool ret = false;

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<ISO7816Location> icLocation = boost::dynamic_pointer_cast<ISO7816Location>(location);

		EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a ISO7816Location.");

		
		if (icLocation->fileid != 0)
		{
			getISO7816Commands()->selectFile(icLocation->fileid);
		}
		else
		{
			unsigned char defaultdf[16];
			memset(defaultdf, 0x00, sizeof(defaultdf));
			if (memcmp(icLocation->dfname, defaultdf, sizeof(defaultdf)))
			{
				getISO7816Commands()->selectFile(icLocation->dfname, icLocation->dfnamelen);
			}
		}

		switch (icLocation->fileType)
		{
		case IFT_MASTER:
		case IFT_DIRECTORY:
			if (icLocation->dataObject > 0)
			{
				ret = getISO7816Commands()->getData(data, dataLength, icLocation->dataObject);
			}
			else
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Please specify a data object.");
			}
			break;

		case IFT_TRANSPARENT:
			ret = getISO7816Commands()->readBinay(data, dataLength, 0);
			break;

		case IFT_LINEAR_FIXED:
		case IFT_LINEAR_VARIABLE:
		case IFT_CYCLIC:
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented yet.");
			break;

		default:
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Doesn't know how to read on this file.");
			break;
		}

		return ret;
	}

	size_t ISO7816CardProvider::readDataHeader(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
	{
		return 0;
	}

	boost::shared_ptr<ISO7816Chip> ISO7816CardProvider::getISO7816Chip()
	{
		return boost::dynamic_pointer_cast<ISO7816Chip>(getChip());
	}
}

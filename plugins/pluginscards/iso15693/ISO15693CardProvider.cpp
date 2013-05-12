/**
 * \file iso15693cardprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO15693 card profiles.
 */

#include "iso15693cardprovider.hpp"
#include "iso15693location.hpp"
#include "iso15693chip.hpp"

namespace logicalaccess
{
	ISO15693CardProvider::ISO15693CardProvider()
		: CardProvider()
	{

	}

	ISO15693CardProvider::~ISO15693CardProvider()
	{

	}

	bool ISO15693CardProvider::erase()
	{
		bool ret = true;

		boost::shared_ptr<LocationNode> rootNode = getISO15693Chip()->getRootLocationNode();
		vector<boost::shared_ptr<LocationNode> > childs = rootNode->getChildrens();

		for (size_t i = 0; i < childs.size() && ret; ++i)
		{
			boost::shared_ptr<LocationNode> blockNode = childs.at(i);
			boost::shared_ptr<AccessInfo> ai;
			ret = erase(blockNode->getLocation(), ai);
		}

		return ret;
	}

	bool ISO15693CardProvider::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
	{
		bool ret = false;
		ISO15693Commands::SystemInformation sysinfo = getISO15693Commands()->getSystemInformation();

		if (sysinfo.hasVICCMemorySize)
		{
			unsigned char* tmp = new unsigned char[sysinfo.blockSize];
			memset(tmp, 0x00, sysinfo.blockSize);

			boost::shared_ptr<AccessInfo> ai;
			ret = writeData(location, aiToUse, ai, tmp, sysinfo.blockSize, CB_DEFAULT);
		}

		return ret;
	}
	
	bool ISO15693CardProvider::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo>, boost::shared_ptr<AccessInfo>, const void* data, size_t dataLength, CardBehavior)
	{
		bool ret = false;

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasWriteDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<ISO15693Location> icLocation = boost::dynamic_pointer_cast<ISO15693Location>(location);

		EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a ISO15693Location.");

		if (getISO15693Commands()->writeBlock(icLocation->block, data, dataLength))
		{
			ret = true;
		}

		return ret;
	}

	bool ISO15693CardProvider::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo>, void* data, size_t dataLength, CardBehavior)
	{
		bool ret = false;

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<ISO15693Location> icLocation = boost::dynamic_pointer_cast<ISO15693Location>(location);

		EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a ISO15693Location.");

		if (getISO15693Commands()->readBlock(icLocation->block, data, dataLength, dataLength))
		{
			ret = true;
		}

		return ret;
	}

	size_t ISO15693CardProvider::readDataHeader(boost::shared_ptr<Location>, boost::shared_ptr<AccessInfo>, void*, size_t)
	{
		return 0;
	}

	boost::shared_ptr<ISO15693Chip> ISO15693CardProvider::getISO15693Chip()
	{
		return boost::dynamic_pointer_cast<ISO15693Chip>(getChip());
	}
}

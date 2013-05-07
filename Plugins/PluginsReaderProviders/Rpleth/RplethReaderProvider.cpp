/**
 * \file RplethReaderProvider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth reader provider.
 */

#include "RplethReaderProvider.h"

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "RplethReaderUnit.h"


namespace LOGICALACCESS
{
	RplethReaderProvider::RplethReaderProvider() :
		ReaderProvider()
	{
	}

	boost::shared_ptr<RplethReaderProvider> RplethReaderProvider::getSingletonInstance()
	{
		static boost::shared_ptr<RplethReaderProvider> instance;
		if (!instance)
		{
			instance = boost::shared_ptr<RplethReaderProvider>(new RplethReaderProvider());
			instance->refreshReaderList();
		}
		return instance;
	}

	boost::shared_ptr<RplethReaderProvider> RplethReaderProvider::createInstance()
	{
		boost::shared_ptr<RplethReaderProvider> instance = boost::shared_ptr<RplethReaderProvider>(new RplethReaderProvider());
		instance->refreshReaderList();

		return instance;
	}

	RplethReaderProvider::~RplethReaderProvider()
	{
	}

	boost::shared_ptr<ReaderUnit> RplethReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit");

		boost::shared_ptr<RplethReaderUnit> ret(new RplethReaderUnit());
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
		d_readers.push_back(ret);

		return ret;
	}	

	bool RplethReaderProvider::refreshReaderList()
	{
		d_readers.clear();		
		
		return true;
	}	
}


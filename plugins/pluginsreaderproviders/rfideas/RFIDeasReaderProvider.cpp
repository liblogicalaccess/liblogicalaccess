/**
 * \file RFIDeasReaderProvider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader Provider RFIDeas.
 */

#include "rfideasreaderprovider.hpp"

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "rfideasreaderunit.hpp"


namespace logicalaccess
{
	RFIDeasReaderProvider::RFIDeasReaderProvider() :
		ReaderProvider()
	{
	}

	boost::shared_ptr<RFIDeasReaderProvider> RFIDeasReaderProvider::getSingletonInstance()
	{
		static boost::shared_ptr<RFIDeasReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new RFIDeasReaderProvider());
			try
			{
				instance->refreshReaderList();
			}
			catch(std::exception&)
			{
			}
		}

		return instance;
	}

	RFIDeasReaderProvider::~RFIDeasReaderProvider()
	{
	}

	boost::shared_ptr<ReaderUnit> RFIDeasReaderProvider::createReaderUnit()
	{
		boost::shared_ptr<RFIDeasReaderUnit> ret = RFIDeasReaderUnit::getSingletonInstance();
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		return ret;
	}	

	bool RFIDeasReaderProvider::refreshReaderList()
	{
		d_readers.clear();
		
		d_readers.push_back(createReaderUnit());

		return true;
	}	
}


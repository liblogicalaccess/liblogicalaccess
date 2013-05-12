/**
 * \file idondemandreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief IdOnDemand reader provider.
 */

#include "idondemandreaderprovider.hpp

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "idondemandreaderunit.hpp"

namespace logicalaccess
{
	IdOnDemandReaderProvider::IdOnDemandReaderProvider() :
		ReaderProvider()
	{
	}

	IdOnDemandReaderProvider::~IdOnDemandReaderProvider()
	{
	}

	boost::shared_ptr<IdOnDemandReaderProvider> IdOnDemandReaderProvider::getSingletonInstance()
	{
		INFO_SIMPLE_("Getting singleton instance...");
		static boost::shared_ptr<IdOnDemandReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new IdOnDemandReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	boost::shared_ptr<ReaderUnit> IdOnDemandReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<IdOnDemandReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new IdOnDemandReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		return ret;
	}	

	bool IdOnDemandReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLOGICALACCESSException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<IdOnDemandReaderUnit> unit(new IdOnDemandReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


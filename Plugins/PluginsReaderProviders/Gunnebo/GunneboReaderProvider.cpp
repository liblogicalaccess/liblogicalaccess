/**
 * \file GunneboReaderProvider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Gunnebo reader provider.
 */

#include "GunneboReaderProvider.h"

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "GunneboReaderUnit.h"


namespace LOGICALACCESS
{
	GunneboReaderProvider::GunneboReaderProvider() :
		ReaderProvider()
	{
	}

	GunneboReaderProvider::~GunneboReaderProvider()
	{
	}

	boost::shared_ptr<GunneboReaderProvider> GunneboReaderProvider::getSingletonInstance()
	{
		INFO_SIMPLE_("Getting singleton instance...");
		static boost::shared_ptr<GunneboReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new GunneboReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	boost::shared_ptr<ReaderUnit> GunneboReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<GunneboReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new GunneboReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		//INFO_("Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

		return ret;
	}	

	bool GunneboReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLOGICALACCESSException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<GunneboReaderUnit> unit(new GunneboReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


/**
 * \file DeisterReaderProvider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader Provider Deister.
 */

#include "DeisterReaderProvider.h"

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "DeisterReaderUnit.h"


namespace logicalaccess
{
	DeisterReaderProvider::DeisterReaderProvider() :
		ReaderProvider()
	{
	}

	boost::shared_ptr<DeisterReaderProvider> DeisterReaderProvider::getSingletonInstance()
	{
		static boost::shared_ptr<DeisterReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new DeisterReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	DeisterReaderProvider::~DeisterReaderProvider()
	{
	}

	boost::shared_ptr<ReaderUnit> DeisterReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<DeisterReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new DeisterReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		//INFO_("Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

		return ret;
	}	

	bool DeisterReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLOGICALACCESSException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<DeisterReaderUnit> unit(new DeisterReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


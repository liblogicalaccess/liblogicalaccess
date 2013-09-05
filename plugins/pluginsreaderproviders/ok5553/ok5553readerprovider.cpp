/**
 * \file ok5553readerprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief OK5553 reader provider.
 */

#include "ok5553readerprovider.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "ok5553readerunit.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"


namespace logicalaccess
{
	OK5553ReaderProvider::OK5553ReaderProvider() :
		ReaderProvider()
	{
	}

	boost::shared_ptr<OK5553ReaderProvider> OK5553ReaderProvider::getSingletonInstance()
	{
		static boost::shared_ptr<OK5553ReaderProvider> instance;
		if (!instance)
		{
			instance = boost::shared_ptr<OK5553ReaderProvider>(new OK5553ReaderProvider());
			instance->refreshReaderList();
		}
		return instance;
	}

	boost::shared_ptr<OK5553ReaderProvider> OK5553ReaderProvider::createInstance()
	{
		boost::shared_ptr<OK5553ReaderProvider> instance = boost::shared_ptr<OK5553ReaderProvider>(new OK5553ReaderProvider());
		instance->refreshReaderList();

		return instance;
	}

	OK5553ReaderProvider::~OK5553ReaderProvider()
	{
		release();
	}

	void OK5553ReaderProvider::release()
	{
	}

	boost::shared_ptr<ReaderUnit> OK5553ReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit");

		boost::shared_ptr<OK5553ReaderUnit> ret(new OK5553ReaderUnit());
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
		d_readers.push_back(ret);

		return ret;
	}	

	bool OK5553ReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<OK5553ReaderUnit> unit(new OK5553ReaderUnit());
			boost::shared_ptr<SerialPortDataTransport> dataTransport = boost::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
			dataTransport->setSerialPort(*i);
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}
		
		return true;
	}	
}


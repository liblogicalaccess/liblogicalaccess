/**
 * \file promagreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Promag reader provider.
 */

#include "promagreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "promagreaderunit.hpp"


namespace logicalaccess
{
	PromagReaderProvider::PromagReaderProvider() :
		ReaderProvider()
	{
	}

	boost::shared_ptr<PromagReaderProvider> PromagReaderProvider::getSingletonInstance()
	{
		static boost::shared_ptr<PromagReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new PromagReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	PromagReaderProvider::~PromagReaderProvider()
	{
		release();
	}

	void PromagReaderProvider::release()
	{
	}

	boost::shared_ptr<ReaderUnit> PromagReaderProvider::createReaderUnit()
	{
		INFO_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<PromagReaderUnit> ret(new PromagReaderUnit());
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		return ret;
	}	

	bool PromagReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<PromagReaderUnit> unit(new PromagReaderUnit());
			boost::shared_ptr<SerialPortDataTransport> dataTransport = boost::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
			dataTransport->setSerialPort(*i);
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


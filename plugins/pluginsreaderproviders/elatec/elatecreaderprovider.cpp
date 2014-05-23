/**
 * \file elatecreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Elatec reader provider.
 */

#include "elatecreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "elatecreaderunit.hpp"


namespace logicalaccess
{
	ElatecReaderProvider::ElatecReaderProvider() :
		ReaderProvider()
	{
	}

	boost::shared_ptr<ElatecReaderProvider> ElatecReaderProvider::getSingletonInstance()
	{
		static boost::shared_ptr<ElatecReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new ElatecReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	ElatecReaderProvider::~ElatecReaderProvider()
	{
		release();
	}

	void ElatecReaderProvider::release()
	{
	}

	boost::shared_ptr<ReaderUnit> ElatecReaderProvider::createReaderUnit()
	{
		LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)";

		boost::shared_ptr<ElatecReaderUnit> ret(new ElatecReaderUnit());
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		return ret;
	}	

	bool ElatecReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<ElatecReaderUnit> unit(new ElatecReaderUnit());
			boost::shared_ptr<SerialPortDataTransport> dataTransport = boost::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
			dataTransport->setSerialPort(*i);
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


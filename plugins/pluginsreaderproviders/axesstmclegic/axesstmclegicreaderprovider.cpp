/**
 * \file axesstmclegicreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC Legic reader provider.
 */

#include "axesstmclegicreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "axesstmclegicreaderunit.hpp"


namespace logicalaccess
{
	AxessTMCLegicReaderProvider::AxessTMCLegicReaderProvider() :
		ReaderProvider()
	{
	}

	boost::shared_ptr<AxessTMCLegicReaderProvider> AxessTMCLegicReaderProvider::getSingletonInstance()
	{
		static boost::shared_ptr<AxessTMCLegicReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new AxessTMCLegicReaderProvider());
			instance->refreshReaderList();
		}
		return instance;
	}

	AxessTMCLegicReaderProvider::~AxessTMCLegicReaderProvider()
	{
		release();
	}

	void AxessTMCLegicReaderProvider::release()
	{
	}

	boost::shared_ptr<ReaderUnit> AxessTMCLegicReaderProvider::createReaderUnit()
	{
		LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)";

		boost::shared_ptr<AxessTMCLegicReaderUnit> ret(new AxessTMCLegicReaderUnit());
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		return ret;
	}	

	bool AxessTMCLegicReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<AxessTMCLegicReaderUnit> unit(new AxessTMCLegicReaderUnit());
			boost::shared_ptr<SerialPortDataTransport> dataTransport = boost::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
			dataTransport->setSerialPort(*i);
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


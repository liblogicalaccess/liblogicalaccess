/**
 * \file axesstmclegicreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC Legic reader provider.
 */

#include "axesstmclegicreaderprovider.hpp"

#ifdef LINUX
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
	}

	boost::shared_ptr<ReaderUnit> AxessTMCLegicReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<AxessTMCLegicReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new AxessTMCLegicReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		//INFO_("Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

		return ret;
	}	

	bool AxessTMCLegicReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLOGICALACCESSException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<AxessTMCLegicReaderUnit> unit(new AxessTMCLegicReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


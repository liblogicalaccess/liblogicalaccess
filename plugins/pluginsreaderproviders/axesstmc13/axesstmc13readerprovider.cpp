/**
 * \file axesstmc13readerprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC 13Mhz reader provider.
 */

#include "axesstmc13readerprovider.hpp"

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "axesstmc13readerunit.hpp"


namespace logicalaccess
{
	AxessTMC13ReaderProvider::AxessTMC13ReaderProvider() :
		ReaderProvider()
	{
	}

	boost::shared_ptr<AxessTMC13ReaderProvider> AxessTMC13ReaderProvider::getSingletonInstance()
	{
		static boost::shared_ptr<AxessTMC13ReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new AxessTMC13ReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	AxessTMC13ReaderProvider::~AxessTMC13ReaderProvider()
	{
	}

	boost::shared_ptr<ReaderUnit> AxessTMC13ReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<AxessTMC13ReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new AxessTMC13ReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		//INFO_("Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

		return ret;
	}	

	bool AxessTMC13ReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<AxessTMC13ReaderUnit> unit(new AxessTMC13ReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


/**
 * \file stidstrreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader Provider STidSTR.
 */

#include "stidstrreaderprovider.hpp"

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "stidstrreaderunit.hpp"

namespace logicalaccess
{
	STidSTRReaderProvider::STidSTRReaderProvider() :
		ReaderProvider()
	{	
		//INFO_SIMPLE_("Constructor");
	}

	STidSTRReaderProvider::~STidSTRReaderProvider()
	{
		//INFO_SIMPLE_("Destructor");
	}

	boost::shared_ptr<STidSTRReaderProvider> STidSTRReaderProvider::getSingletonInstance()
	{
		INFO_SIMPLE_("Getting singleton instance...");
		static boost::shared_ptr<STidSTRReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new STidSTRReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	boost::shared_ptr<ReaderUnit> STidSTRReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<STidSTRReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new STidSTRReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		//INFO_("Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

		return ret;
	}	

	bool STidSTRReaderProvider::refreshReaderList()
	{
		//INFO_SIMPLE_("Refreshing reader list...");
		d_readers.clear();

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<STidSTRReaderUnit> unit(new STidSTRReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
			//INFO_("--> Detected reader unit {%s}...", dynamic_cast<XmlSerializable*>(&(*unit))->serialize().c_str());
		}

		return true;
	}	
}


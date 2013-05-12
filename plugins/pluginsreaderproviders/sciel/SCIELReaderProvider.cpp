/**
 * \file scielreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader Provider SCIEL.
 */

#include "scielreaderprovider.hpp"

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "scielreaderunit.hpp"


namespace logicalaccess
{
	SCIELReaderProvider::SCIELReaderProvider() :
		ReaderProvider()
	{
	}

	SCIELReaderProvider::~SCIELReaderProvider()
	{
	}

	boost::shared_ptr<SCIELReaderProvider> SCIELReaderProvider::getSingletonInstance()
	{
		static boost::shared_ptr<SCIELReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new SCIELReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	boost::shared_ptr<ReaderUnit> SCIELReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<SCIELReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new SCIELReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		//INFO_("Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

		return ret;
	}	

	bool SCIELReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLOGICALACCESSException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<SCIELReaderUnit> unit(new SCIELReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


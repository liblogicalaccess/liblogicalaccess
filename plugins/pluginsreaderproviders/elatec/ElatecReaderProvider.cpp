/**
 * \file elatecreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Elatec reader provider.
 */

#include "elatecreaderprovider.hpp"

#ifdef LINUX
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
	}

	boost::shared_ptr<ReaderUnit> ElatecReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<ElatecReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new ElatecReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		//INFO_("Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

		return ret;
	}	

	bool ElatecReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLOGICALACCESSException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<ElatecReaderUnit> unit(new ElatecReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


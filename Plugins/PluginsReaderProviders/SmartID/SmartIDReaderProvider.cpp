/**
 * \file SmartIDReaderProvider.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>, Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Reader Provider SmartID.
 */

#include "SmartIDReaderProvider.h"

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "SmartIDReaderUnit.h"


namespace LOGICALACCESS
{
	SmartIDReaderProvider::SmartIDReaderProvider() :
		ReaderProvider()
	{
	}

	boost::shared_ptr<SmartIDReaderProvider> SmartIDReaderProvider::getSingletonInstance()
	{
		static boost::shared_ptr<SmartIDReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new SmartIDReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	SmartIDReaderProvider::~SmartIDReaderProvider()
	{
	}

	boost::shared_ptr<ReaderUnit> SmartIDReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<SmartIDReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new SmartIDReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		//INFO_("Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

		return ret;
	}	

	bool SmartIDReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLOGICALACCESSException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<SmartIDReaderUnit> unit(new SmartIDReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


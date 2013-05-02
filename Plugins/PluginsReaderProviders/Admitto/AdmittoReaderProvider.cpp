/**
 * \file AdmittoReaderProvider.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Admitto reader provider.
 */

#include "AdmittoReaderProvider.h"

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "AdmittoReaderUnit.h"


namespace LOGICALACCESS
{
	AdmittoReaderProvider::AdmittoReaderProvider() :
		ReaderProvider()
	{
	}

	AdmittoReaderProvider::~AdmittoReaderProvider()
	{
	}

	boost::shared_ptr<AdmittoReaderProvider> AdmittoReaderProvider::getSingletonInstance()
	{
		INFO_SIMPLE_("Getting singleton instance...");
		static boost::shared_ptr<AdmittoReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new AdmittoReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	boost::shared_ptr<ReaderUnit> AdmittoReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<AdmittoReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new AdmittoReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		//INFO_("Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

		return ret;
	}	

	bool AdmittoReaderProvider::refreshReaderList()
	{
		d_readers.clear();

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLOGICALACCESSException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<AdmittoReaderUnit> unit(new AdmittoReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


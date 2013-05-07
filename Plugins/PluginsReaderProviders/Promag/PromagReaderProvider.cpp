/**
 * \file PromagReaderProvider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Promag reader provider.
 */

#include "PromagReaderProvider.h"

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "PromagReaderUnit.h"


namespace LOGICALACCESS
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
	}

	boost::shared_ptr<ReaderUnit> PromagReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<PromagReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new PromagReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		//INFO_("Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

		return ret;
	}	

	bool PromagReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLOGICALACCESSException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<PromagReaderUnit> unit(new PromagReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}


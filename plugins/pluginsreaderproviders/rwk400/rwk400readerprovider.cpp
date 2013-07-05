/**
 * \file rwk400readerprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rwk400 reader provider.
 */

#include "rwk400readerprovider.hpp"

#ifdef LINUX
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "rwk400readerunit.hpp"
//Rwk400ReaderProvider

namespace logicalaccess
{
	Rwk400ReaderProvider::Rwk400ReaderProvider() :
		ReaderProvider()
	{
	}

	boost::shared_ptr<Rwk400ReaderProvider> Rwk400ReaderProvider::getSingletonInstance()
	{
		static boost::shared_ptr<Rwk400ReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new Rwk400ReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	Rwk400ReaderProvider::~Rwk400ReaderProvider()
	{
	}

	boost::shared_ptr<ReaderUnit> Rwk400ReaderProvider::createReaderUnit()
	{
		INFO_SIMPLE_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<Rwk400ReaderUnit> ret;
		boost::shared_ptr<SerialPortXml> port;
		port.reset(new SerialPortXml(""));
		ret.reset(new Rwk400ReaderUnit(port));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		//INFO_("Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

		return ret;
	}	

	bool Rwk400ReaderProvider::refreshReaderList()
	{
		d_readers.clear();		

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<Rwk400ReaderUnit> unit(new Rwk400ReaderUnit(*i));
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}		
}


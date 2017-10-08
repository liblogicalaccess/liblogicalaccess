/**
 * \file osdpreaderprovider.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief Reader Provider OSDP.
 */

#include "logicalaccess/logs.hpp"
#include "osdpreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "osdpreaderunit.hpp"

namespace logicalaccess
{
	OSDPReaderProvider::OSDPReaderProvider() :
		ReaderProvider()
	{
	}

	OSDPReaderProvider::~OSDPReaderProvider()
	{
		OSDPReaderProvider::release();
	}

	void OSDPReaderProvider::release()
	{
	}

	std::shared_ptr<OSDPReaderProvider> OSDPReaderProvider::getSingletonInstance()
	{
		LOG(LogLevel::INFOS) << "Getting singleton instance...";
		static std::shared_ptr<OSDPReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new OSDPReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	std::shared_ptr<ReaderUnit> OSDPReaderProvider::createReaderUnit()
	{
		LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)";

		std::shared_ptr<OSDPReaderUnit> ret(new OSDPReaderUnit());
		ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

		return ret;
	}	

	bool OSDPReaderProvider::refreshReaderList()
	{
		//LOG(LogLevel::INFOS) << "Refreshing reader list...");
		d_readers.clear();

		std::vector<std::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

		for (std::vector<std::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			std::shared_ptr<OSDPReaderUnit> unit(new OSDPReaderUnit());
			std::shared_ptr<SerialPortDataTransport> dataTransport = std::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
			dataTransport->setSerialPort(*i);
			unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
			//LOG(LogLevel::INFOS) << "--> Detected reader unit {%s}...", dynamic_cast<XmlSerializable*>(&(*unit))->serialize().c_str());
		}

		return true;
	}	
}


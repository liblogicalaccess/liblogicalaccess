/**
 * \file axesstmclegicreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC Legic reader provider.
 */

#include "axesstmclegicreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/myexception.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>
#include <logicalaccess/logs.hpp>

#include "axesstmclegicreaderunit.hpp"

namespace logicalaccess
{
    AxessTMCLegicReaderProvider::AxessTMCLegicReaderProvider() :
        ReaderProvider()
    {
    }

    std::shared_ptr<AxessTMCLegicReaderProvider> AxessTMCLegicReaderProvider::getSingletonInstance()
    {
        static std::shared_ptr<AxessTMCLegicReaderProvider> instance;
        if (!instance)
        {
            instance.reset(new AxessTMCLegicReaderProvider());
            instance->refreshReaderList();
        }
        return instance;
    }

    AxessTMCLegicReaderProvider::~AxessTMCLegicReaderProvider()
    {
	    AxessTMCLegicReaderProvider::release();
    }

    void AxessTMCLegicReaderProvider::release()
    {
    }

    std::shared_ptr<ReaderUnit> AxessTMCLegicReaderProvider::createReaderUnit()
    {
        LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)";

        std::shared_ptr<AxessTMCLegicReaderUnit> ret(new AxessTMCLegicReaderUnit());
        ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

        return ret;
    }

    bool AxessTMCLegicReaderProvider::refreshReaderList()
    {
        d_readers.clear();

        std::vector<std::shared_ptr<SerialPortXml> > ports;
        EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

        for (std::vector<std::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
        {
            std::shared_ptr<AxessTMCLegicReaderUnit> unit(new AxessTMCLegicReaderUnit());
            std::shared_ptr<SerialPortDataTransport> dataTransport = std::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
            dataTransport->setSerialPort(*i);
            unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
            d_readers.push_back(unit);
        }

        return true;
    }
}
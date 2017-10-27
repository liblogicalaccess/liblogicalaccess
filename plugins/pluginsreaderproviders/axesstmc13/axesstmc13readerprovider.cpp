/**
 * \file axesstmc13readerprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC 13Mhz reader provider.
 */

#include "axesstmc13readerprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/myexception.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "axesstmc13readerunit.hpp"
#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
AxessTMC13ReaderProvider::AxessTMC13ReaderProvider()
    : ReaderProvider()
{
}

std::shared_ptr<AxessTMC13ReaderProvider> AxessTMC13ReaderProvider::getSingletonInstance()
{
    static std::shared_ptr<AxessTMC13ReaderProvider> instance;
    if (!instance)
    {
        instance.reset(new AxessTMC13ReaderProvider());
        instance->refreshReaderList();
    }

    return instance;
}

AxessTMC13ReaderProvider::~AxessTMC13ReaderProvider()
{
    AxessTMC13ReaderProvider::release();
}

void AxessTMC13ReaderProvider::release()
{
}

std::shared_ptr<ReaderUnit> AxessTMC13ReaderProvider::createReaderUnit()
{
    LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port "
                            "auto-detect will be used when connecting to reader)";

    std::shared_ptr<AxessTMC13ReaderUnit> ret(new AxessTMC13ReaderUnit());
    ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

    return ret;
}

bool AxessTMC13ReaderProvider::refreshReaderList()
{
    d_readers.clear();

    std::vector<std::shared_ptr<SerialPortXml>> ports;
    EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports),
                              LibLogicalAccessException,
                              "Can't enumerate the serial port list.");

    for (std::vector<std::shared_ptr<SerialPortXml>>::iterator i = ports.begin();
         i != ports.end(); ++i)
    {
        std::shared_ptr<AxessTMC13ReaderUnit> unit(new AxessTMC13ReaderUnit());
        std::shared_ptr<SerialPortDataTransport> dataTransport =
            std::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
        dataTransport->setSerialPort(*i);
        unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
        d_readers.push_back(unit);
    }

    return true;
}
}
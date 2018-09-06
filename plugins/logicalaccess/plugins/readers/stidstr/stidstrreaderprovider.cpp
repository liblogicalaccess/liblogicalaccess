/**
 * \file stidstrreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader Provider STidSTR.
 */

#include <logicalaccess/plugins/readers/stidstr/stidstrreaderprovider.hpp>
#include <logicalaccess/readerproviders/serialportdatatransport.hpp>

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>
#include <logicalaccess/plugins/llacommon/logs.hpp>

#include <logicalaccess/plugins/readers/stidstr/stidstrreaderunit.hpp>
#include <logicalaccess/plugins/readers/stidstr/readercardadapters/stidstrreaderdatatransport.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
STidSTRReaderProvider::STidSTRReaderProvider()
    : ISO7816ReaderProvider()
{
}

STidSTRReaderProvider::~STidSTRReaderProvider()
{
    STidSTRReaderProvider::release();
}

void STidSTRReaderProvider::release()
{
}

std::shared_ptr<STidSTRReaderProvider> STidSTRReaderProvider::getSingletonInstance()
{
    LOG(LogLevel::INFOS) << "Getting singleton instance...";
    static std::shared_ptr<STidSTRReaderProvider> instance;
    if (!instance)
    {
        instance.reset(new STidSTRReaderProvider());
        instance->refreshReaderList();
    }

    return instance;
}

std::shared_ptr<ReaderUnit> STidSTRReaderProvider::createReaderUnit()
{
    LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port "
                            "auto-detect will be used when connecting to reader)";

    std::shared_ptr<STidSTRReaderUnit> ret(new STidSTRReaderUnit());
    ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

    return ret;
}

std::shared_ptr<ISO7816ReaderUnit>
STidSTRReaderProvider::createReaderUnit(std::string readerunitname)
{
    auto ru = std::dynamic_pointer_cast<STidSTRReaderUnit>(createReaderUnit());
    auto dt = std::make_shared<STidSTRDataTransport>(readerunitname);
    dt->setPortBaudRate(38400);
    ru->setDataTransport(dt);
    return ru;
}

bool STidSTRReaderProvider::refreshReaderList()
{
    // LOG(LogLevel::INFOS) << "Refreshing reader list...");
    d_readers.clear();

    std::vector<std::shared_ptr<SerialPortXml>> ports;
    EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports),
                              LibLogicalAccessException,
                              "Can't enumerate the serial port list.");

    for (std::vector<std::shared_ptr<SerialPortXml>>::iterator i = ports.begin();
         i != ports.end(); ++i)
    {
        std::shared_ptr<STidSTRReaderUnit> unit(new STidSTRReaderUnit());
        std::shared_ptr<SerialPortDataTransport> dataTransport =
            std::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
        dataTransport->setSerialPort(*i);
        unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
        d_readers.push_back(unit);
        // LOG(LogLevel::INFOS) << "--> Detected reader unit {%s}...",
        // dynamic_cast<XmlSerializable*>(&(*unit))->serialize().c_str());
    }

    return true;
}
}
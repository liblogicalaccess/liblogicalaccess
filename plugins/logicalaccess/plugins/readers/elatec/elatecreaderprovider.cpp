/**
 * \file elatecreaderprovider.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Elatec reader provider.
 */

#include <logicalaccess/plugins/readers/elatec/elatecreaderprovider.hpp>
#include <logicalaccess/readerproviders/serialportdatatransport.hpp>
#include <logicalaccess/myexception.hpp>

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>
#include <logicalaccess/plugins/llacommon/logs.hpp>

#include <logicalaccess/plugins/readers/elatec/elatecreaderunit.hpp>

namespace logicalaccess
{
ElatecReaderProvider::ElatecReaderProvider()
    : ReaderProvider()
{
}

std::shared_ptr<ElatecReaderProvider> ElatecReaderProvider::getSingletonInstance()
{
    static std::shared_ptr<ElatecReaderProvider> instance;
    if (!instance)
    {
        instance.reset(new ElatecReaderProvider());
        instance->refreshReaderList();
    }

    return instance;
}

ElatecReaderProvider::~ElatecReaderProvider()
{
    ElatecReaderProvider::release();
}

void ElatecReaderProvider::release()
{
}

std::shared_ptr<ReaderUnit> ElatecReaderProvider::createReaderUnit()
{
    LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port "
                            "auto-detect will be used when connecting to reader)";

    std::shared_ptr<ElatecReaderUnit> ret(new ElatecReaderUnit());
    ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

    return ret;
}

bool ElatecReaderProvider::refreshReaderList()
{
    d_readers.clear();

    std::vector<std::shared_ptr<SerialPortXml>> ports;
    EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports),
                              LibLogicalAccessException,
                              "Can't enumerate the serial port list.");

    for (std::vector<std::shared_ptr<SerialPortXml>>::iterator i = ports.begin();
         i != ports.end(); ++i)
    {
        std::shared_ptr<ElatecReaderUnit> unit(new ElatecReaderUnit());
        std::shared_ptr<SerialPortDataTransport> dataTransport =
            std::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
        dataTransport->setSerialPort(*i);
        unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
        d_readers.push_back(unit);
    }

    return true;
}
}
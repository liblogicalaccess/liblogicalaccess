/**
 * \file scielreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader Provider SCIEL.
 */

#include <logicalaccess/plugins/readers/sciel/scielreaderprovider.hpp>
#include <logicalaccess/readerproviders/serialportdatatransport.hpp>
#include <logicalaccess/myexception.hpp>

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include <logicalaccess/plugins/readers/sciel/scielreaderunit.hpp>

namespace logicalaccess
{
SCIELReaderProvider::SCIELReaderProvider()
    : ReaderProvider()
{
}

SCIELReaderProvider::~SCIELReaderProvider()
{
    SCIELReaderProvider::release();
}

void SCIELReaderProvider::release()
{
}

std::shared_ptr<SCIELReaderProvider> SCIELReaderProvider::getSingletonInstance()
{
    static std::shared_ptr<SCIELReaderProvider> instance;
    if (!instance)
    {
        instance.reset(new SCIELReaderProvider());
        instance->refreshReaderList();
    }

    return instance;
}

std::shared_ptr<ReaderUnit> SCIELReaderProvider::createReaderUnit()
{
    LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port "
                            "auto-detect will be used when connecting to reader)";

    std::shared_ptr<SCIELReaderUnit> ret(new SCIELReaderUnit());
    ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

    return ret;
}

bool SCIELReaderProvider::refreshReaderList()
{
    d_readers.clear();

    std::vector<std::shared_ptr<SerialPortXml>> ports;
    EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports),
                              LibLogicalAccessException,
                              "Can't enumerate the serial port list.");

    for (std::vector<std::shared_ptr<SerialPortXml>>::iterator i = ports.begin();
         i != ports.end(); ++i)
    {
        std::shared_ptr<SCIELReaderUnit> unit(new SCIELReaderUnit());
        std::shared_ptr<SerialPortDataTransport> dataTransport =
            std::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
        dataTransport->setSerialPort(*i);
        unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
        d_readers.push_back(unit);
    }

    return true;
}
}
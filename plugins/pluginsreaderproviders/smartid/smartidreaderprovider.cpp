/**
 * \file smartidreaderprovider.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Reader Provider SmartID.
 */

#include "smartidreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "smartidreaderunit.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    SmartIDReaderProvider::SmartIDReaderProvider() :
        ReaderProvider()
    {
    }

    std::shared_ptr<SmartIDReaderProvider> SmartIDReaderProvider::getSingletonInstance()
    {
        static std::shared_ptr<SmartIDReaderProvider> instance;
        if (!instance)
        {
            instance.reset(new SmartIDReaderProvider());
            instance->refreshReaderList();
        }

        return instance;
    }

    SmartIDReaderProvider::~SmartIDReaderProvider()
    {
        release();
    }

    void SmartIDReaderProvider::release()
    {
    }

    std::shared_ptr<ReaderUnit> SmartIDReaderProvider::createReaderUnit()
    {
        LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)";

        std::shared_ptr<SmartIDReaderUnit> ret(new SmartIDReaderUnit());
        ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

        return ret;
    }

    bool SmartIDReaderProvider::refreshReaderList()
    {
        d_readers.clear();

        std::vector<std::shared_ptr<SerialPortXml> > ports;
        EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

        for (std::vector<std::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
        {
            std::shared_ptr<SmartIDReaderUnit> unit(new SmartIDReaderUnit());
            std::shared_ptr<SerialPortDataTransport> dataTransport = std::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
            dataTransport->setSerialPort(*i);
            unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
            d_readers.push_back(unit);
        }

        return true;
    }
}
/**
 * \file gunneboreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Gunnebo reader provider.
 */

#include "gunneboreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "gunneboreaderunit.hpp"

namespace logicalaccess
{
    GunneboReaderProvider::GunneboReaderProvider() :
        ReaderProvider()
    {
    }

    GunneboReaderProvider::~GunneboReaderProvider()
    {
        release();
    }

    void GunneboReaderProvider::release()
    {
    }

    std::shared_ptr<GunneboReaderProvider> GunneboReaderProvider::getSingletonInstance()
    {
        LOG(LogLevel::INFOS) << "Getting singleton instance...";
        static std::shared_ptr<GunneboReaderProvider> instance;
        if (!instance)
        {
            instance.reset(new GunneboReaderProvider());
            instance->refreshReaderList();
        }

        return instance;
    }

    std::shared_ptr<ReaderUnit> GunneboReaderProvider::createReaderUnit()
    {
        LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)";

        std::shared_ptr<GunneboReaderUnit> ret(new GunneboReaderUnit());
        ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

        //LOG(LogLevel::INFOS) << "Created reader unit {%s}", dynamic_cast<XmlSerializable*>(&(*ret))->serialize().c_str());

        return ret;
    }

    bool GunneboReaderProvider::refreshReaderList()
    {
        d_readers.clear();

        std::vector<std::shared_ptr<SerialPortXml> > ports;
        EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

        for (std::vector<std::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
        {
            std::shared_ptr<GunneboReaderUnit> unit(new GunneboReaderUnit());
            std::shared_ptr<SerialPortDataTransport> dataTransport = std::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
            dataTransport->setSerialPort(*i);
            unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
            d_readers.push_back(unit);
        }

        return true;
    }
}
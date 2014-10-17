/**
 * \file idondemandreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief IdOnDemand reader provider.
 */

#include "idondemandreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "idondemandreaderunit.hpp"

namespace logicalaccess
{
    IdOnDemandReaderProvider::IdOnDemandReaderProvider() :
        ReaderProvider()
    {
    }

    IdOnDemandReaderProvider::~IdOnDemandReaderProvider()
    {
        release();
    }

    void IdOnDemandReaderProvider::release()
    {
    }

    std::shared_ptr<IdOnDemandReaderProvider> IdOnDemandReaderProvider::getSingletonInstance()
    {
        LOG(LogLevel::INFOS) << "Getting singleton instance...";
        static std::shared_ptr<IdOnDemandReaderProvider> instance;
        if (!instance)
        {
            instance.reset(new IdOnDemandReaderProvider());
            instance->refreshReaderList();
        }

        return instance;
    }

    std::shared_ptr<ReaderUnit> IdOnDemandReaderProvider::createReaderUnit()
    {
        LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)";

        std::shared_ptr<IdOnDemandReaderUnit> ret(new IdOnDemandReaderUnit());
        ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

        return ret;
    }

    bool IdOnDemandReaderProvider::refreshReaderList()
    {
        d_readers.clear();

        std::vector<std::shared_ptr<SerialPortXml> > ports;
        EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

        for (std::vector<std::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
        {
            std::shared_ptr<IdOnDemandReaderUnit> unit(new IdOnDemandReaderUnit());
            std::shared_ptr<SerialPortDataTransport> dataTransport = std::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
            dataTransport->setSerialPort(*i);
            unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
            d_readers.push_back(unit);
        }

        return true;
    }
}
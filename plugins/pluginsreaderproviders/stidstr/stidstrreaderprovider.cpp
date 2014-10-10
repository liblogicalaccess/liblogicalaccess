/**
 * \file stidstrreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader Provider STidSTR.
 */

#include "stidstrreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "stidstrreaderunit.hpp"

namespace logicalaccess
{
    STidSTRReaderProvider::STidSTRReaderProvider() :
        ReaderProvider()
    {
    }

    STidSTRReaderProvider::~STidSTRReaderProvider()
    {
        release();
    }

    void STidSTRReaderProvider::release()
    {
    }

    boost::shared_ptr<STidSTRReaderProvider> STidSTRReaderProvider::getSingletonInstance()
    {
        LOG(LogLevel::INFOS) << "Getting singleton instance...";
        static boost::shared_ptr<STidSTRReaderProvider> instance;
        if (!instance)
        {
            instance.reset(new STidSTRReaderProvider());
            instance->refreshReaderList();
        }

        return instance;
    }

    boost::shared_ptr<ReaderUnit> STidSTRReaderProvider::createReaderUnit()
    {
        LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)";

        boost::shared_ptr<STidSTRReaderUnit> ret(new STidSTRReaderUnit());
        ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

        return ret;
    }

    bool STidSTRReaderProvider::refreshReaderList()
    {
        //LOG(LogLevel::INFOS) << "Refreshing reader list...");
        d_readers.clear();

        std::vector<boost::shared_ptr<SerialPortXml> > ports;
        EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

        for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
        {
            boost::shared_ptr<STidSTRReaderUnit> unit(new STidSTRReaderUnit());
            boost::shared_ptr<SerialPortDataTransport> dataTransport = boost::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
            dataTransport->setSerialPort(*i);
            unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
            d_readers.push_back(unit);
            //LOG(LogLevel::INFOS) << "--> Detected reader unit {%s}...", dynamic_cast<XmlSerializable*>(&(*unit))->serialize().c_str());
        }

        return true;
    }
}
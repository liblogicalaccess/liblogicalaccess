/**
 * \file scielreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader Provider SCIEL.
 */

#include "scielreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "scielreaderunit.hpp"

namespace logicalaccess
{
    SCIELReaderProvider::SCIELReaderProvider() :
        ReaderProvider()
    {
    }

    SCIELReaderProvider::~SCIELReaderProvider()
    {
        release();
    }

    void SCIELReaderProvider::release()
    {
    }

    boost::shared_ptr<SCIELReaderProvider> SCIELReaderProvider::getSingletonInstance()
    {
        static boost::shared_ptr<SCIELReaderProvider> instance;
        if (!instance)
        {
            instance.reset(new SCIELReaderProvider());
            instance->refreshReaderList();
        }

        return instance;
    }

    boost::shared_ptr<ReaderUnit> SCIELReaderProvider::createReaderUnit()
    {
        LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)";

        boost::shared_ptr<SCIELReaderUnit> ret(new SCIELReaderUnit());
        ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

        return ret;
    }

    bool SCIELReaderProvider::refreshReaderList()
    {
        d_readers.clear();

        std::vector<boost::shared_ptr<SerialPortXml> > ports;
        EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

        for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
        {
            boost::shared_ptr<SCIELReaderUnit> unit(new SCIELReaderUnit());
            boost::shared_ptr<SerialPortDataTransport> dataTransport = boost::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
            dataTransport->setSerialPort(*i);
            unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
            d_readers.push_back(unit);
        }

        return true;
    }
}
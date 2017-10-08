#include "stidprgreaderprovider.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/logs.hpp"
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/readerproviders/serialportxml.hpp"
#include "stidprgreaderunit.hpp"
#include "stidprgresultchecker.hpp"


using namespace logicalaccess;

std::shared_ptr<STidPRGReaderProvider>
STidPRGReaderProvider::getSingletonInstance()
{
    static std::shared_ptr<STidPRGReaderProvider> instance =
        std::make_shared<STidPRGReaderProvider>();
    static bool first = true;

    if (first)
    {
        first = false;
        instance->refreshReaderList();
    }
    return instance;
}

std::shared_ptr<ReaderUnit> STidPRGReaderProvider::createReaderUnit()
{
    std::shared_ptr<STidPRGReaderUnit> ret(new STidPRGReaderUnit());
    ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

    return ret;
}

bool STidPRGReaderProvider::refreshReaderList()
{
    // This is copy-pasted from STidSTR;
    d_readers.clear();

    std::vector<std::shared_ptr<SerialPortXml>> ports;
    EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports),
                              LibLogicalAccessException,
                              "Can't enumerate the serial port list.");

    for (std::vector<std::shared_ptr<SerialPortXml>>::iterator i = ports.begin();
         i != ports.end(); ++i)
    {
        std::shared_ptr<STidPRGReaderUnit> unit(new STidPRGReaderUnit());
        std::shared_ptr<SerialPortDataTransport> dataTransport =
            std::dynamic_pointer_cast<SerialPortDataTransport>(
                unit->getDataTransport());
        assert(dataTransport);
        dataTransport->setSerialPort(*i);
        unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

        unit->getDefaultReaderCardAdapter()->setResultChecker(
            std::make_shared<STidPRGResultChecker>());
        d_readers.push_back(unit);
    }

    return true;
}

void STidPRGReaderProvider::release()
{
}

const ReaderList &STidPRGReaderProvider::getReaderList()
{
    return d_readers;
}

std::string STidPRGReaderProvider::getRPType() const
{
    return "STidPRG";
}

std::string STidPRGReaderProvider::getRPName() const
{
    return "STidPRG";
}

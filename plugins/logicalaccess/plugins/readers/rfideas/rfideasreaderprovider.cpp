/**
 * \file RFIDeasReaderProvider.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Reader Provider RFIDeas.
 */

#include <logicalaccess/plugins/readers/rfideas/rfideasreaderprovider.hpp>

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include <logicalaccess/plugins/readers/rfideas/rfideasreaderunit.hpp>

namespace logicalaccess
{
RFIDeasReaderProvider::RFIDeasReaderProvider()
    : ReaderProvider()
{
}

std::shared_ptr<RFIDeasReaderProvider> RFIDeasReaderProvider::getSingletonInstance()
{
    static std::shared_ptr<RFIDeasReaderProvider> instance;
    if (!instance)
    {
        instance.reset(new RFIDeasReaderProvider());
        try
        {
            instance->refreshReaderList();
        }
        catch (std::exception &)
        {
        }
    }

    return instance;
}

RFIDeasReaderProvider::~RFIDeasReaderProvider()
{
    RFIDeasReaderProvider::release();
}

void RFIDeasReaderProvider::release()
{
}

std::shared_ptr<ReaderUnit> RFIDeasReaderProvider::createReaderUnit()
{
    std::shared_ptr<RFIDeasReaderUnit> ret = RFIDeasReaderUnit::getSingletonInstance();
    ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

    return ret;
}

bool RFIDeasReaderProvider::refreshReaderList()
{
    d_readers.clear();

    d_readers.push_back(createReaderUnit());

    return true;
}
}
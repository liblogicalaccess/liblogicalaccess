/**
 * \file rplethreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth reader provider.
 */

#include "rplethreaderprovider.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "rplethreaderunit.hpp"

namespace logicalaccess
{
    RplethReaderProvider::RplethReaderProvider() :
        ReaderProvider()
    {
    }

    std::shared_ptr<RplethReaderProvider> RplethReaderProvider::getSingletonInstance()
    {
        static std::shared_ptr<RplethReaderProvider> instance;
        if (!instance)
        {
            instance = std::shared_ptr<RplethReaderProvider>(new RplethReaderProvider());
            instance->refreshReaderList();
        }
        return instance;
    }

    std::shared_ptr<RplethReaderProvider> RplethReaderProvider::createInstance()
    {
        std::shared_ptr<RplethReaderProvider> instance = std::shared_ptr<RplethReaderProvider>(new RplethReaderProvider());
        instance->refreshReaderList();

        return instance;
    }

    RplethReaderProvider::~RplethReaderProvider()
    {
        release();
    }

    void RplethReaderProvider::release()
    {
    }

    std::shared_ptr<ReaderUnit> RplethReaderProvider::createReaderUnit()
    {
        LOG(LogLevel::INFOS) << "Creating new reader unit";

        std::shared_ptr<RplethReaderUnit> ret(new RplethReaderUnit());
        ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
        d_readers.push_back(ret);

        return ret;
    }

    bool RplethReaderProvider::refreshReaderList()
    {
        d_readers.clear();

        return true;
    }
}
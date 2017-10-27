/**
 * \file a3mlgm5600readerprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader Provider A3MLGM5600.
 */

#include "a3mlgm5600readerprovider.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

namespace logicalaccess
{
A3MLGM5600ReaderProvider::A3MLGM5600ReaderProvider()
    : ReaderProvider()
{
}

std::shared_ptr<A3MLGM5600ReaderProvider> A3MLGM5600ReaderProvider::getSingletonInstance()
{
    static std::shared_ptr<A3MLGM5600ReaderProvider> instance;
    if (!instance)
    {
        instance.reset(new A3MLGM5600ReaderProvider());
        instance->refreshReaderList();
    }

    return instance;
}

void A3MLGM5600ReaderProvider::release()
{
}

A3MLGM5600ReaderProvider::~A3MLGM5600ReaderProvider()
{
    A3MLGM5600ReaderProvider::release();
}

std::shared_ptr<ReaderUnit> A3MLGM5600ReaderProvider::createReaderUnit()
{
    // std::shared_ptr<A3MLGM5600ReaderUnit> ret =
    // A3MLGM5600ReaderUnit::getSingletonInstance();
    std::shared_ptr<A3MLGM5600ReaderUnit> ret(new A3MLGM5600ReaderUnit());
    ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

    return ret;
}

bool A3MLGM5600ReaderProvider::refreshReaderList()
{
    d_readers.clear();

    d_readers.push_back(createReaderUnit());

    return true;
}
}
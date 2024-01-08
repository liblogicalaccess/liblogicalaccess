/**
 * \file readerprovider.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime@leosac.com>
 * \brief Reader provider.
 */

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <map>
#include <time.h>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>

namespace logicalaccess
{
ReaderProvider::ReaderProvider() {}

ReaderProvider::~ReaderProvider() {}

std::shared_ptr<ReaderProvider>
ReaderProvider::getReaderProviderFromRPType(std::string rpt)
{
    return LibraryManager::getInstance()->getReaderProvider(rpt);
}

const ReaderList ReaderProvider::waitForReaders(std::vector<std::string> readers,
                                                double maxwait, bool all)
{
    ReaderList ret;
    bool notfound = true;
    time_t timer, current_timer;

    time(&timer);

    while (notfound)
    {
        ret.clear();

        time(&current_timer);
        if (maxwait != 0 && difftime(current_timer, timer) > maxwait)
            return ret;

        refreshReaderList();
        ReaderList rl = getReaderList();
        for (const auto &it : rl)
        {
            if (std::find(readers.begin(), readers.end(), it->getName()) !=
                readers.end())
            {
                ret.push_back(it);
            }
        }

        if ((all == false && ret.size() != 0) ||
            (all == true && ret.size() == readers.size()))
            notfound = false;
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return ret;
}
}
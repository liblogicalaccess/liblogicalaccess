/**
 * \file readerprovider.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Reader provider.
 */

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include <boost/filesystem.hpp>
#include <map>
#include <time.h>
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
    ReaderProvider::ReaderProvider()
    {
    }

    ReaderProvider::~ReaderProvider()
    {
    }

    boost::shared_ptr<ReaderProvider> ReaderProvider::getReaderProviderFromRPType(std::string rpt)
    {
        return LibraryManager::getInstance()->getReaderProvider(rpt);
    }

    const std::vector<boost::shared_ptr<ReaderUnit> > ReaderProvider::waitForReaders(std::vector<std::string> readers, double maxwait, bool all)
    {
        std::vector<boost::shared_ptr<ReaderUnit> > ret;
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
            for (ReaderList::iterator it = rl.begin(); it != rl.end(); ++it)
            {
                if (std::find(readers.begin(), readers.end(), (*it)->getName()) != readers.end())
                    ret.push_back(*it);
            }

            if ((all == false && ret.size() != 0) || (all == true && ret.size() == readers.size()))
                notfound = false;
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        return ret;
    }
}
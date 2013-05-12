/**
 * \file readerprovider.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Reader provider.
 */

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include <boost/filesystem.hpp>
#include <map>
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
}

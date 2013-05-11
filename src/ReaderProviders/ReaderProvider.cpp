/**
 * \file ReaderProvider.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Reader provider.
 */

#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "logicalaccess/DynLibrary/IDynLibrary.h"
#include <boost/filesystem.hpp>
#include <map>
#include "logicalaccess/DynLibrary/LibraryManager.h"
#include "logicalaccess/logs.h"

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

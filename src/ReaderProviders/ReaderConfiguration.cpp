/**
 * \file ReaderConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader configuration.
 */

#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "logicalaccess/ReaderProviders/ReaderConfiguration.h"
#include "logicalaccess/Cards/AccessInfo.h"
#include "logicalaccess/DynLibrary/IDynLibrary.h"
#include <boost/filesystem.hpp>
#include "logicalaccess/DynLibrary/LibraryManager.h"

#include "logicalaccess/logs.h"

namespace logicalaccess
{
	ReaderConfiguration::ReaderConfiguration()
	{
		Settings config = Settings::getInstance();
	
		d_readerProvider = LibraryManager::getInstance()->getReaderProvider(config.DefaultReader);				
	}

	ReaderConfiguration::~ReaderConfiguration()
	{
	}	

	void ReaderConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		if (d_readerProvider)
		{
			node.put("ReaderProvider", d_readerProvider->getRPType());
			if (d_ReaderUnit)
			{
				d_ReaderUnit->serialize(node);
			}
		}

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void ReaderConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		INFO_SIMPLE_("Unserializing reader configuration...");
		std::string rpType = static_cast<std::string>(node.get_child("ReaderProvider").get_value<std::string>());

		INFO_("Reader provider type {%s}", rpType.c_str());

		d_readerProvider = ReaderProvider::getReaderProviderFromRPType(rpType);

		EXCEPTION_ASSERT_WITH_LOG(d_readerProvider, LibLOGICALACCESSException, "Unknown reader provider type.");

		d_ReaderUnit = d_readerProvider->createReaderUnit();
		if (!d_ReaderUnit->unSerialize(node, ""))
		{
			d_ReaderUnit.reset();
		}
	}

	std::string ReaderConfiguration::getDefaultXmlNodeName() const
	{
		return "ReaderConfiguration";
	}

	boost::shared_ptr<ReaderProvider> ReaderConfiguration::getReaderProvider() const
	{
		return d_readerProvider;
	}

	void ReaderConfiguration::setReaderProvider(boost::shared_ptr<ReaderProvider> provider)
	{
		d_readerProvider = provider;
	}

	boost::shared_ptr<ReaderUnit> ReaderConfiguration::getReaderUnit() const
	{
		return d_ReaderUnit;
	}

	void ReaderConfiguration::setReaderUnit(boost::shared_ptr<ReaderUnit> unit)
	{
		d_ReaderUnit = unit;
	}
}


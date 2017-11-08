/**
 * \file readerconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader configuration.
 */

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>
#include <logicalaccess/cards/accessinfo.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>

#include <logicalaccess/logs.hpp>
#include <logicalaccess/settings.hpp>

namespace logicalaccess
{
ReaderConfiguration::ReaderConfiguration()
{
    Settings *config = Settings::getInstance();

    try
    {
        d_readerProvider =
            LibraryManager::getInstance()->getReaderProvider(config->DefaultReader);
    }
    catch (std::exception &ex)
    {
        LOG(LogLevel::ERRORS) << "Cannot create reader provider instance `"
                              << config->DefaultReader << "`: " << ex.what();
        d_readerProvider.reset();
    }
}

ReaderConfiguration::~ReaderConfiguration()
{
}

void ReaderConfiguration::serialize(boost::property_tree::ptree &parentNode)
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

void ReaderConfiguration::unSerialize(boost::property_tree::ptree &node)
{
    LOG(LogLevel::INFOS) << "Unserializing reader configuration...";
    std::string rpType = static_cast<std::string>(
        node.get_child("ReaderProvider").get_value<std::string>());

    LOG(LogLevel::INFOS) << "Reader provider type " << rpType;

    d_readerProvider = ReaderProvider::getReaderProviderFromRPType(rpType);

    EXCEPTION_ASSERT_WITH_LOG(d_readerProvider, LibLogicalAccessException,
                              "Unknown reader provider type.");

    d_ReaderUnit = d_readerProvider->createReaderUnit();
    try
    {
        d_ReaderUnit->unSerialize(node, "");
    }
    catch (std::exception &e)
    {
        LOG(LogLevel::WARNINGS) << "Failed to unserialize reader unit: " << e.what();
        d_ReaderUnit.reset();
    }
}

std::string ReaderConfiguration::getDefaultXmlNodeName() const
{
    return "ReaderConfiguration";
}

std::shared_ptr<ReaderProvider> ReaderConfiguration::getReaderProvider() const
{
    return d_readerProvider;
}

void ReaderConfiguration::setReaderProvider(std::shared_ptr<ReaderProvider> provider)
{
    d_readerProvider = provider;
}

std::shared_ptr<ReaderUnit> ReaderConfiguration::getReaderUnit() const
{
    return d_ReaderUnit;
}

void ReaderConfiguration::setReaderUnit(std::shared_ptr<ReaderUnit> unit)
{
    d_ReaderUnit = unit;
}
}
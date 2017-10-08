/**
 * \file ok5553readerunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  OK5553 reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include "ok5553readerunitconfiguration.hpp"
#include "ok5553readerprovider.hpp"

namespace logicalaccess
{
    OK5553ReaderUnitConfiguration::OK5553ReaderUnitConfiguration()
        : ReaderUnitConfiguration(READER_OK5553)
    {
	    OK5553ReaderUnitConfiguration::resetConfiguration();
    }

    OK5553ReaderUnitConfiguration::~OK5553ReaderUnitConfiguration()
    {
    }

    void OK5553ReaderUnitConfiguration::resetConfiguration()
    {
    }

    void OK5553ReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void OK5553ReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
    {
    }

    std::string OK5553ReaderUnitConfiguration::getDefaultXmlNodeName() const
    {
        return "OK5553ReaderUnitConfiguration";
    }
}
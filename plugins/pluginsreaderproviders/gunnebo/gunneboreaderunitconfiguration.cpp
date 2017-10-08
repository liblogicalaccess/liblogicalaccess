/**
 * \file gunneboreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  Gunnebo reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include "gunneboreaderunitconfiguration.hpp"
#include "gunneboreaderprovider.hpp"

namespace logicalaccess
{
    GunneboReaderUnitConfiguration::GunneboReaderUnitConfiguration()
        : ReaderUnitConfiguration(READER_GUNNEBO)
    {
	    GunneboReaderUnitConfiguration::resetConfiguration();
    }

    GunneboReaderUnitConfiguration::~GunneboReaderUnitConfiguration()
    {
    }

    void GunneboReaderUnitConfiguration::resetConfiguration()
    {
        
    }

    void GunneboReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void GunneboReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
    {
        
    }

    std::string GunneboReaderUnitConfiguration::getDefaultXmlNodeName() const
    {
        return "GunneboReaderUnitConfiguration";
    }
}
/**
 * \file promagreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  Promag reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include "promagreaderunitconfiguration.hpp"
#include "promagreaderprovider.hpp"

namespace logicalaccess
{
    PromagReaderUnitConfiguration::PromagReaderUnitConfiguration()
        : ReaderUnitConfiguration(READER_PROMAG)
    {
        resetConfiguration();
    }

    PromagReaderUnitConfiguration::~PromagReaderUnitConfiguration()
    {
    }

    void PromagReaderUnitConfiguration::resetConfiguration()
    {
    }

    void PromagReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void PromagReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
    {
        //TODO: Implement this.
    }

    std::string PromagReaderUnitConfiguration::getDefaultXmlNodeName() const
    {
        return "PromagReaderUnitConfiguration";
    }
}
/**
 * \file elatecreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  Elatec reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include "elatecreaderunitconfiguration.hpp"
#include "elatecreaderprovider.hpp"

namespace logicalaccess
{
    ElatecReaderUnitConfiguration::ElatecReaderUnitConfiguration()
        : ReaderUnitConfiguration(READER_ELATEC)
    {
        resetConfiguration();
    }

    ElatecReaderUnitConfiguration::~ElatecReaderUnitConfiguration()
    {
    }

    void ElatecReaderUnitConfiguration::resetConfiguration()
    {
    }

    void ElatecReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void ElatecReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
    {
        //TODO: Implement this.
    }

    std::string ElatecReaderUnitConfiguration::getDefaultXmlNodeName() const
    {
        return "ElatecReaderUnitConfiguration";
    }
}
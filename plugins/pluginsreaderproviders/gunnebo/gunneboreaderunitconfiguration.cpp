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
        resetConfiguration();
    }

    GunneboReaderUnitConfiguration::~GunneboReaderUnitConfiguration()
    {
    }

    void GunneboReaderUnitConfiguration::resetConfiguration()
    {
        d_checksum = true;
    }

    void GunneboReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        node.put("Checksum", d_checksum);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void GunneboReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
    {
        d_checksum = node.get_child("Checksum").get_value<bool>();
    }

    std::string GunneboReaderUnitConfiguration::getDefaultXmlNodeName() const
    {
        return "GunneboReaderUnitConfiguration";
    }

    bool GunneboReaderUnitConfiguration::getChecksum() const
    {
        return d_checksum;
    }

    void GunneboReaderUnitConfiguration::setChecksum(bool checksum)
    {
        d_checksum = checksum;
    }
}
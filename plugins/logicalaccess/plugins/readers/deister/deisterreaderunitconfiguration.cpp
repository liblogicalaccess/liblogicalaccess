/**
 * \file deisterreaderunitconfiguration.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Deister reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/readers/deister/deisterreaderunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/deister/deisterreaderprovider.hpp>

namespace logicalaccess
{
DeisterReaderUnitConfiguration::DeisterReaderUnitConfiguration()
    : ReaderUnitConfiguration(READER_DEISTER)
{
    DeisterReaderUnitConfiguration::resetConfiguration();
}

DeisterReaderUnitConfiguration::~DeisterReaderUnitConfiguration()
{
}

void DeisterReaderUnitConfiguration::resetConfiguration()
{
}

void DeisterReaderUnitConfiguration::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void DeisterReaderUnitConfiguration::unSerialize(boost::property_tree::ptree & /*node*/)
{
    // TODO: Implement this.
}

std::string DeisterReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "DeisterReaderUnitConfiguration";
}
}
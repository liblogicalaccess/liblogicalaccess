/**
 * \file axesstmclegicreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  AxessTMCLegic reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include "axesstmclegicreaderunitconfiguration.hpp"
#include "axesstmclegicreaderprovider.hpp"

namespace logicalaccess
{
AxessTMCLegicReaderUnitConfiguration::AxessTMCLegicReaderUnitConfiguration()
    : ReaderUnitConfiguration(READER_AXESSTMCLEGIC)
{
    AxessTMCLegicReaderUnitConfiguration::resetConfiguration();
}

AxessTMCLegicReaderUnitConfiguration::~AxessTMCLegicReaderUnitConfiguration()
{
}

void AxessTMCLegicReaderUnitConfiguration::resetConfiguration()
{
}

void AxessTMCLegicReaderUnitConfiguration::serialize(
    boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void AxessTMCLegicReaderUnitConfiguration::unSerialize(
    boost::property_tree::ptree & /*node*/)
{
    // TODO: Implement this.
}

std::string AxessTMCLegicReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "AxessTMCLegicReaderUnitConfiguration";
}
}
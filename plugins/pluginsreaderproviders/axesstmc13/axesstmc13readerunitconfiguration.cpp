/**
 * \file axesstmc13readerunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  AxessTMC 13Mhz reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include "axesstmc13readerunitconfiguration.hpp"
#include "axesstmc13readerprovider.hpp"

namespace logicalaccess
{
AxessTMC13ReaderUnitConfiguration::AxessTMC13ReaderUnitConfiguration()
    : ReaderUnitConfiguration(READER_AXESSTMC13)
{
    AxessTMC13ReaderUnitConfiguration::resetConfiguration();
}

AxessTMC13ReaderUnitConfiguration::~AxessTMC13ReaderUnitConfiguration()
{
}

void AxessTMC13ReaderUnitConfiguration::resetConfiguration()
{
}

void AxessTMC13ReaderUnitConfiguration::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void AxessTMC13ReaderUnitConfiguration::unSerialize(
    boost::property_tree::ptree & /*node*/)
{
    // TODO: Implement this.
}

std::string AxessTMC13ReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "AxessTMC13ReaderUnitConfiguration";
}
}
/**
 * \file smartidreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SmartID reader unit configuration.
 */

#include <logicalaccess/plugins/readers/smartid/smartidreaderunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/smartid/smartidreaderprovider.hpp>
#include <boost/property_tree/ptree.hpp>

#ifdef UNIX
#include <termios.h>
#include <unistd.h>

#endif

namespace logicalaccess
{
SmartIDReaderUnitConfiguration::SmartIDReaderUnitConfiguration()
    : ReaderUnitConfiguration(READER_SMARTID)
{
    SmartIDReaderUnitConfiguration::resetConfiguration();
}

SmartIDReaderUnitConfiguration::~SmartIDReaderUnitConfiguration()
{
}

void SmartIDReaderUnitConfiguration::resetConfiguration()
{
}

void SmartIDReaderUnitConfiguration::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void SmartIDReaderUnitConfiguration::unSerialize(boost::property_tree::ptree & /*node*/)
{
}

std::string SmartIDReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "SmartIDReaderUnitConfiguration";
}
}
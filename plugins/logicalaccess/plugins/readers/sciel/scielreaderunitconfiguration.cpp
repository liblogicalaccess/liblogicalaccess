/**
 * \file scielreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SCIEL reader unit configuration.
 */

#include <logicalaccess/plugins/readers/sciel/scielreaderunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/sciel/scielreaderprovider.hpp>

#include <boost/property_tree/ptree.hpp>

#ifdef UNIX
#include <termios.h>
#include <unistd.h>
#endif

namespace logicalaccess
{
SCIELReaderUnitConfiguration::SCIELReaderUnitConfiguration()
    : ReaderUnitConfiguration(READER_SCIEL)
{
    SCIELReaderUnitConfiguration::resetConfiguration();
}

SCIELReaderUnitConfiguration::~SCIELReaderUnitConfiguration()
{
}

void SCIELReaderUnitConfiguration::resetConfiguration()
{
    d_tagOutThreshold  = 150;
    d_tagInThreshold   = 140;
    d_safetyThreshold  = 120;
    d_timeBeforeTagIn  = 1000;
    d_timeBeforeTagOut = 2000;
    d_timeBeforeSafety = 2000;
    d_timeRemoval      = 2;
}

void SCIELReaderUnitConfiguration::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("TagOutThreshold", d_tagOutThreshold);
    node.put("TagInThreshold", d_tagInThreshold);
    node.put("SafetyThreshold", d_safetyThreshold);
    node.put("TimeBeforeTagIn", d_timeBeforeTagIn);
    node.put("TimeBeforeTagOut", d_timeBeforeTagOut);
    node.put("TimeBeforeSafety", d_timeBeforeSafety);
    node.put("TimeRemoval", d_timeRemoval);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void SCIELReaderUnitConfiguration::unSerialize(boost::property_tree::ptree &node)
{
    d_tagOutThreshold  = node.get_child("TagOutThreshold").get_value<unsigned char>();
    d_tagInThreshold   = node.get_child("TagInThreshold").get_value<unsigned char>();
    d_safetyThreshold  = node.get_child("SafetyThreshold").get_value<unsigned char>();
    d_timeBeforeTagIn  = node.get_child("TimeBeforeTagIn").get_value<unsigned short>();
    d_timeBeforeTagOut = node.get_child("TimeBeforeTagOut").get_value<unsigned short>();
    d_timeBeforeSafety = node.get_child("TimeBeforeSafety").get_value<unsigned short>();
    d_timeRemoval      = node.get_child("TimeRemoval").get_value<unsigned char>();
}

std::string SCIELReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "SCIELReaderUnitConfiguration";
}
}
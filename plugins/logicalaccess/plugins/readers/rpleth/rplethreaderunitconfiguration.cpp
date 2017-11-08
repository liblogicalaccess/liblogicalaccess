/**
 * \file rplethreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  Rpleth reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/readers/rpleth/rplethreaderunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/rpleth/rplethreaderprovider.hpp>

namespace logicalaccess
{
RplethReaderUnitConfiguration::RplethReaderUnitConfiguration()
    : ISO7816ReaderUnitConfiguration(READER_RPLETH)
{
    RplethReaderUnitConfiguration::resetConfiguration();
}

RplethReaderUnitConfiguration::~RplethReaderUnitConfiguration()
{
}

void RplethReaderUnitConfiguration::resetConfiguration()
{
    d_offset = 1;
    d_length = 16;
    d_mode   = PROXY;
}

void RplethReaderUnitConfiguration::setWiegandConfiguration(unsigned char offset,
                                                            unsigned char length)
{
    d_offset = offset;
    d_length = length;
}

void RplethReaderUnitConfiguration::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    node.put("Offset", d_offset);
    node.put("Length", d_length);
    node.put("Mode", d_mode);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void RplethReaderUnitConfiguration::unSerialize(boost::property_tree::ptree &node)
{
    d_offset = node.get_child("Offset").get_value<unsigned char>();
    d_length = node.get_child("Length").get_value<unsigned char>();
    d_mode   = static_cast<RplethMode>(node.get_child("Mode").get_value<unsigned char>());
}

std::string RplethReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "RplethReaderUnitConfiguration";
}

unsigned char RplethReaderUnitConfiguration::getOffset() const
{
    return d_offset;
}

void RplethReaderUnitConfiguration::setOffset(unsigned char offset)
{
    d_offset = offset;
}

unsigned char RplethReaderUnitConfiguration::getLength() const
{
    return d_length;
}

void RplethReaderUnitConfiguration::setLength(unsigned char length)
{
    d_length = length;
}

RplethMode RplethReaderUnitConfiguration::getMode() const
{
    return d_mode;
}

void RplethReaderUnitConfiguration::setMode(RplethMode mode)
{
    d_mode = mode;
}
}
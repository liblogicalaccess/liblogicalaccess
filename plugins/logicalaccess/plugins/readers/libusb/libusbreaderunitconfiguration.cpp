/**
 * \file libusbreaderunitconfiguration.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief  LibUSB reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderprovider.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderunitconfiguration.hpp>

namespace logicalaccess
{
LibUSBReaderUnitConfiguration::LibUSBReaderUnitConfiguration()
    : ReaderUnitConfiguration(READER_LIBUSB), d_slot(0)
{
    LibUSBReaderUnitConfiguration::resetConfiguration();
}

LibUSBReaderUnitConfiguration::~LibUSBReaderUnitConfiguration()
{
}

void LibUSBReaderUnitConfiguration::resetConfiguration()
{
}

void LibUSBReaderUnitConfiguration::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    node.put("Slot", d_slot);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void LibUSBReaderUnitConfiguration::unSerialize(boost::property_tree::ptree & node)
{
    d_slot = node.get_child("Slot").get_value<unsigned char>();
}

std::string LibUSBReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "LibUSBReaderUnitConfiguration";
}

unsigned char LibUSBReaderUnitConfiguration::getSlot() const
{
    return d_slot;
}

void LibUSBReaderUnitConfiguration::setSlot(unsigned char slot)
{
    d_slot = slot;
}
}

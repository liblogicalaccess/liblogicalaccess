/**
 * \file keyboardreaderunitconfiguration.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Keyboard reader unit configuration.
 */

#include <logicalaccess/plugins/readers/keyboard/keyboardreaderunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/keyboard/keyboardreaderprovider.hpp>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
KeyboardReaderUnitConfiguration::KeyboardReaderUnitConfiguration()
    : ReaderUnitConfiguration(READER_KEYBOARD)
{
    KeyboardReaderUnitConfiguration::resetConfiguration();
}

KeyboardReaderUnitConfiguration::~KeyboardReaderUnitConfiguration()
{
}

void KeyboardReaderUnitConfiguration::resetConfiguration()
{
    d_prefix          = "";
    d_suffix          = "";
    d_isDecimalNumber = true;
    d_klayout         = "";
}

void KeyboardReaderUnitConfiguration::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("Prefix", d_prefix);
    node.put("Suffix", d_suffix);
    node.put("IsDecimalNumber", d_isDecimalNumber);
    node.put("KeyboardLayout", d_klayout);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void KeyboardReaderUnitConfiguration::unSerialize(boost::property_tree::ptree &node)
{
    d_prefix          = node.get_child("Prefix").get_value<std::string>();
    d_suffix          = node.get_child("Suffix").get_value<std::string>();
    d_isDecimalNumber = node.get_child("IsDecimalNumber").get_value<bool>();
    d_klayout         = node.get_child("KeyboardLayout").get_value<std::string>();
}

std::string KeyboardReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "KeyboardReaderUnitConfiguration";
}

std::string KeyboardReaderUnitConfiguration::getPrefix() const
{
    return d_prefix;
}

void KeyboardReaderUnitConfiguration::setPrefix(const std::string &prefix)
{
    d_prefix = prefix;
}

std::string KeyboardReaderUnitConfiguration::getSuffix() const
{
    return d_suffix;
}

void KeyboardReaderUnitConfiguration::setSuffix(const std::string &suffix)
{
    d_suffix = suffix;
}

bool KeyboardReaderUnitConfiguration::getIsDecimalNumber() const
{
    return d_isDecimalNumber;
}

void KeyboardReaderUnitConfiguration::setIsDecimalNumber(bool isDecimal)
{
    d_isDecimalNumber = isDecimal;
}

std::string KeyboardReaderUnitConfiguration::getKeyboardLayout() const
{
    return d_klayout;
}

void KeyboardReaderUnitConfiguration::setKeyboardLayout(const std::string &klayout)
{
    d_klayout = klayout;
}
}
/**
 * \file osdpReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR reader unit configuration.
 */

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <logicalaccess/plugins/readers/osdp/osdpreaderunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/osdp/osdpreaderprovider.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
OSDPReaderUnitConfiguration::OSDPReaderUnitConfiguration()
    : ReaderUnitConfiguration(READER_OSDP), d_visualFeedback(false)
{
    OSDPReaderUnitConfiguration::resetConfiguration();

    // default keys
    setSCBKKey(
        std::make_shared<AES128Key>("C1 8A 12 50 7E F2 71 FE D4 82 0E D7 51 28 EF 4E"));
    setSCBKDKey(
        std::make_shared<AES128Key>("30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F"));
}

OSDPReaderUnitConfiguration::~OSDPReaderUnitConfiguration()
{
}

void OSDPReaderUnitConfiguration::resetConfiguration()
{
    d_rs485Address = 0;
    d_visualFeedback = true;
    d_master_key_aes.reset(new AES128Key(""));
    d_scbk_d_key_aes.reset(new AES128Key(""));
    d_scbk_key_aes.reset(new AES128Key(""));
}

void OSDPReaderUnitConfiguration::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("RS485Address", d_rs485Address);
    node.put("VisualFeedback", d_visualFeedback);
    d_scbk_d_key_aes->serialize(node);
    d_scbk_key_aes->serialize(node);
    d_master_key_aes->serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void OSDPReaderUnitConfiguration::unSerialize(boost::property_tree::ptree &node)
{
    LOG(LogLevel::INFOS) << "Unserializing reader unit configuration...";

    d_rs485Address = node.get_child("RS485Address").get_value<unsigned char>();
    d_visualFeedback = node.get_child("VisualFeedback").get_value<bool>();
    d_scbk_d_key_aes->unSerialize(
        node.get_child(d_scbk_d_key_aes->getDefaultXmlNodeName()));
    d_scbk_key_aes->unSerialize(node.get_child(d_scbk_key_aes->getDefaultXmlNodeName()));
    d_master_key_aes->unSerialize(
        node.get_child(d_master_key_aes->getDefaultXmlNodeName()));
}

std::string OSDPReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "OSDPReaderUnitConfiguration";
}

unsigned char OSDPReaderUnitConfiguration::getRS485Address() const
{
    LOG(LogLevel::INFOS) << "RS485 Address {0x" << std::hex << d_rs485Address << std::dec
                         << "(" << d_rs485Address << ")}";
    return d_rs485Address;
}

void OSDPReaderUnitConfiguration::setRS485Address(unsigned char address)
{
    LOG(LogLevel::INFOS) << "RS485 Address {0x" << std::hex << address << std::dec << "("
                         << address << ")}";
    d_rs485Address = address;
}

bool OSDPReaderUnitConfiguration::getVisualFeedback() const
{
    return d_visualFeedback;
}

void OSDPReaderUnitConfiguration::setVisualFeedback(bool enabled)
{
    d_visualFeedback = enabled;
}

std::shared_ptr<AES128Key> OSDPReaderUnitConfiguration::getMasterKey() const
{
    return d_master_key_aes;
}

void OSDPReaderUnitConfiguration::setMasterKey(std::shared_ptr<AES128Key> key)
{
    d_master_key_aes = key;
}

std::shared_ptr<AES128Key> OSDPReaderUnitConfiguration::getSCBKKey() const
{
    return d_scbk_key_aes;
}

void OSDPReaderUnitConfiguration::setSCBKKey(std::shared_ptr<AES128Key> key)
{
    d_scbk_key_aes = key;
}

std::shared_ptr<AES128Key> OSDPReaderUnitConfiguration::getSCBKDKey() const
{
    return d_scbk_d_key_aes;
}

void OSDPReaderUnitConfiguration::setSCBKDKey(std::shared_ptr<AES128Key> key)
{
    d_scbk_d_key_aes = key;
}
}

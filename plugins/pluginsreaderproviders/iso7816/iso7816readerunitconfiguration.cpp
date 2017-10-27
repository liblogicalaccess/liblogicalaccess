/**
 * \file pcscreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/logs.hpp>
#include "iso7816readerunitconfiguration.hpp"
#include "iso7816readerunit.hpp"
#include "desfire/desfirekey.hpp"

namespace logicalaccess
{
ISO7816ReaderUnitConfiguration::ISO7816ReaderUnitConfiguration(std::string rpt)
    : ReaderUnitConfiguration(rpt)
{
    ISO7816ReaderUnitConfiguration::resetConfiguration();
}

ISO7816ReaderUnitConfiguration::ISO7816ReaderUnitConfiguration()
    : ReaderUnitConfiguration(READER_ISO7816)
{
    ISO7816ReaderUnitConfiguration::resetConfiguration();
}

ISO7816ReaderUnitConfiguration::~ISO7816ReaderUnitConfiguration()
{
}

void ISO7816ReaderUnitConfiguration::resetConfiguration()
{
    d_sam_type                   = "SAM_NONE";
    d_sam_reader_name            = "";
    d_keyno_unlock               = 0;
    d_check_sam_reader_available = true;
    d_auto_connect_sam_reader    = true;
}

void ISO7816ReaderUnitConfiguration::serialize(boost::property_tree::ptree &node)
{
    node.put("SAMType", d_sam_type);
    node.put("SAMReaderName", d_sam_reader_name);
    boost::property_tree::ptree knode;
    knode.put("KeyNo", d_keyno_unlock);
    if (d_sam_key_unlock)
    {
        d_sam_key_unlock->serialize(knode);
    }
    node.add_child("SAMKey", knode);
    node.put("CheckSAMReaderIsAvailable", d_check_sam_reader_available);
    node.put("AutoConnectToSAMReader", d_auto_connect_sam_reader);
}

void ISO7816ReaderUnitConfiguration::unSerialize(boost::property_tree::ptree &node)
{
    d_sam_type =
        static_cast<std::string>(node.get_child("SAMType").get_value<std::string>());
    d_sam_reader_name = node.get_child("SAMReaderName").get_value<std::string>();
    boost::property_tree::ptree knode = node.get_child("SAMKey");
    d_keyno_unlock = knode.get_child("KeyNo").get_value<unsigned char>();
    try
    {
        d_sam_key_unlock.reset(new DESFireKey());
        d_sam_key_unlock->unSerialize(knode, "");
    }
    catch (std::exception &ex)
    {
        d_sam_key_unlock.reset();
        LOG(LogLevel::ERRORS) << "Cannot unserialize SAM authentication key: "
                              << ex.what();
    }
    d_check_sam_reader_available =
        node.get_child("CheckSAMReaderIsAvailable").get_value<bool>();
    d_auto_connect_sam_reader =
        node.get_child("AutoConnectToSAMReader").get_value<bool>();
}

std::string ISO7816ReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "ISO7816ReaderUnitConfiguration";
}

void ISO7816ReaderUnitConfiguration::setSAMType(std::string t)
{
    d_sam_type = t;
}

std::string ISO7816ReaderUnitConfiguration::getSAMType() const
{
    return d_sam_type;
}

void ISO7816ReaderUnitConfiguration::setSAMReaderName(std::string t)
{
    d_sam_reader_name = t;
}

std::string ISO7816ReaderUnitConfiguration::getSAMReaderName() const
{
    return d_sam_reader_name;
}
}
/**
 * \file STidSTRReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR reader unit configuration.
 */

#include "stidstrreaderunitconfiguration.hpp"
#include "stidstrreaderprovider.hpp"

namespace logicalaccess
{
    STidSTRReaderUnitConfiguration::STidSTRReaderUnitConfiguration()
        : ReaderUnitConfiguration(READER_STIDSTR)
    {
        resetConfiguration();
    }

    STidSTRReaderUnitConfiguration::~STidSTRReaderUnitConfiguration()
    {
    }

    void STidSTRReaderUnitConfiguration::resetConfiguration()
    {
        d_rs485Address = 0;
        d_communicationType = STID_RS232;
        d_communicationMode = STID_CM_PLAIN;
        d_key_hmac.reset(new HMAC1Key(""));
        d_key_aes.reset(new AES128Key(""));
    }

    void STidSTRReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        node.put("RS485Address", d_rs485Address);
        node.put("CommunicationType", d_communicationType);
        node.put("CommunicationMode", d_communicationMode);
        d_key_hmac->serialize(node);
        d_key_aes->serialize(node);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void STidSTRReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
    {
        LOG(LogLevel::INFOS) << "Unserializing reader unit configuration...";

        d_rs485Address = node.get_child("RS485Address").get_value<unsigned char>();
        d_communicationType = static_cast<STidCommunicationType>(node.get_child("CommunicationType").get_value<unsigned int>());
        d_communicationMode = static_cast<STidCommunicationMode>(node.get_child("CommunicationMode").get_value<unsigned int>());
        d_key_hmac->unSerialize(node.get_child(d_key_hmac->getDefaultXmlNodeName()));
        d_key_aes->unSerialize(node.get_child(d_key_aes->getDefaultXmlNodeName()));
    }

    std::string STidSTRReaderUnitConfiguration::getDefaultXmlNodeName() const
    {
        return "STidSTRReaderUnitConfiguration";
    }

    unsigned char STidSTRReaderUnitConfiguration::getRS485Address() const
    {
        LOG(LogLevel::INFOS) << "RS485 Address {0x" << std::hex << d_rs485Address << std::dec << "(" << d_rs485Address << ")}";
        return d_rs485Address;
    }

    void STidSTRReaderUnitConfiguration::setRS485Address(unsigned char address)
    {
        LOG(LogLevel::INFOS) << "RS485 Address {0x" << std::hex << address << std::dec << "(" << address << ")}";
        d_rs485Address = address;
    }

    STidCommunicationType STidSTRReaderUnitConfiguration::getCommunicationType() const
    {
        switch (d_communicationType)
        {
        case STID_RS232: LOG(LogLevel::INFOS) << "Get Communication type {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_RS232}"; break;
        case STID_RS485: LOG(LogLevel::INFOS) << "Get Communication type {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_RS485}"; break;
        }
        return d_communicationType;
    }

    void STidSTRReaderUnitConfiguration::setCommunicationType(STidCommunicationType ctype)
    {
        d_communicationType = ctype;
        switch (d_communicationType)
        {
        case STID_RS232: LOG(LogLevel::INFOS) << "Get Communication type {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_RS232}"; break;
        case STID_RS485: LOG(LogLevel::INFOS) << "Get Communication type {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_RS485}"; break;
        }
    }

    STidCommunicationMode STidSTRReaderUnitConfiguration::getCommunicationMode() const
    {
        switch (d_communicationMode)
        {
        case STID_CM_PLAIN: LOG(LogLevel::INFOS) << "Get Communication mode {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_CM_PLAIN}"; break;
        case STID_CM_SIGNED: LOG(LogLevel::INFOS) << "Get Communication mode {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_CM_SIGNED}"; break;
        case STID_CM_CIPHERED: LOG(LogLevel::INFOS) << "Get Communication mode {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_CM_CIPHERED}"; break;
        case STID_CM_CIPHERED_AND_SIGNED: LOG(LogLevel::INFOS) << "Get Communication mode {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_CM_CIPHERED_AND_SIGNED}"; break;
        case STID_CM_RESERVED: LOG(LogLevel::INFOS) << "Get Communication mode {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_CM_RESERVED}"; break;
        }
        return d_communicationMode;
    }

    void STidSTRReaderUnitConfiguration::setCommunicationMode(STidCommunicationMode cmode)
    {
        d_communicationMode = cmode;
        switch (d_communicationMode)
        {
        case STID_CM_PLAIN: LOG(LogLevel::INFOS) << "Get Communication mode {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_CM_PLAIN}"; break;
        case STID_CM_SIGNED: LOG(LogLevel::INFOS) << "Get Communication mode {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_CM_SIGNED}"; break;
        case STID_CM_CIPHERED: LOG(LogLevel::INFOS) << "Get Communication mode {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_CM_CIPHERED}"; break;
        case STID_CM_CIPHERED_AND_SIGNED: LOG(LogLevel::INFOS) << "Get Communication mode {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_CM_CIPHERED_AND_SIGNED}"; break;
        case STID_CM_RESERVED: LOG(LogLevel::INFOS) << "Get Communication mode {0x" << std::hex << d_communicationType << std::dec << "(" << d_communicationType << ")} {STID_CM_RESERVED}"; break;
        }
    }

    boost::shared_ptr<HMAC1Key> STidSTRReaderUnitConfiguration::getHMACKey() const
    {
        LOG(LogLevel::DEBUGS) << "Get HMAC key data {" << d_key_hmac->toString() << "}";
        return d_key_hmac;
    }

    void STidSTRReaderUnitConfiguration::setHMACKey(boost::shared_ptr<HMAC1Key> key)
    {
        if (key)
        {
            LOG(LogLevel::DEBUGS) << "Set HMAC key data {" << key->toString() << "}";
            d_key_hmac = key;
        }
    }

    boost::shared_ptr<AES128Key> STidSTRReaderUnitConfiguration::getAESKey() const
    {
        LOG(LogLevel::DEBUGS) << "Get AES 128 key data {" << d_key_aes->toString() << "}";
        return d_key_aes;
    }

    void STidSTRReaderUnitConfiguration::setAESKey(boost::shared_ptr<AES128Key> key)
    {
        if (key)
        {
            LOG(LogLevel::DEBUGS) << "Set AES 128 key data {" << key->toString() << "}";
            d_key_aes = key;
        }
    }
}
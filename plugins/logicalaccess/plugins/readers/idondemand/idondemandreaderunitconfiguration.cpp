/**
 * \file idondemandreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  IdOnDemand reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/plugins/readers/idondemand/idondemandreaderunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/idondemand/idondemandreaderprovider.hpp>

namespace logicalaccess
{
IdOnDemandReaderUnitConfiguration::IdOnDemandReaderUnitConfiguration()
    : ReaderUnitConfiguration(READER_IDONDEMAND)
{
    IdOnDemandReaderUnitConfiguration::resetConfiguration();
}

IdOnDemandReaderUnitConfiguration::~IdOnDemandReaderUnitConfiguration()
{
}

void IdOnDemandReaderUnitConfiguration::resetConfiguration()
{
    d_authCode = "8112";
}

void IdOnDemandReaderUnitConfiguration::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("AuthenticateCode", d_authCode);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void IdOnDemandReaderUnitConfiguration::unSerialize(boost::property_tree::ptree &node)
{
    LOG(LogLevel::INFOS) << "Unserializing reader unit configuration...";

    d_authCode = node.get_child("AuthenticateCode").get_value<std::string>();

    LOG(LogLevel::INFOS) << "Authenticate Code unSerialized {" << d_authCode.c_str()
                         << "}";
}

std::string IdOnDemandReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "IdOnDemandReaderUnitConfiguration";
}

std::string IdOnDemandReaderUnitConfiguration::getAuthCode() const
{
    LOG(LogLevel::INFOS) << "Get Authenticate Code {" << d_authCode << "}";
    return d_authCode;
}

void IdOnDemandReaderUnitConfiguration::setAuthCode(std::string authCode)
{
    LOG(LogLevel::INFOS) << "Set Authenticate Code {" << authCode << "}";
    d_authCode = authCode;
}
}
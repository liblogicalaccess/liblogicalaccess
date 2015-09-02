//
// Created by xaqq on 9/2/15.
//

#include <boost/property_tree/ptree.hpp>
#include "acsacr1222lreaderunitconfiguration.hpp"

using namespace logicalaccess;

ACSACR1222LReaderUnitConfiguration::ACSACR1222LReaderUnitConfiguration()
{
    resetConfiguration();
}

void ACSACR1222LReaderUnitConfiguration::resetConfiguration()
{
    PCSCReaderUnitConfiguration::resetConfiguration();
#ifdef UNIX
    user_feedback_reader_ = "ACS ACR1222 3S PICC Reader 00 03";
#else
    user_feedback_reader = "ACS ACR1222 3S PICC Reader SAM 2";
#endif
}

void ACSACR1222LReaderUnitConfiguration::serialize(
        boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    PCSCReaderUnitConfiguration::serialize(node);

    node.put("UserFeedbackReader", user_feedback_reader_);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void ACSACR1222LReaderUnitConfiguration::unSerialize(boost::property_tree::ptree &node)
{
    PCSCReaderUnitConfiguration::unSerialize(
            node.get_child(PCSCReaderUnitConfiguration::getDefaultXmlNodeName()));
    user_feedback_reader_ = node.get_child("UserFeedbackReader").get_value<std::string>();
}

PCSCReaderUnitType ACSACR1222LReaderUnitConfiguration::getPCSCType() const
{
	return PCSC_RUT_ACS_ACR_1222L;
}

const std::string &ACSACR1222LReaderUnitConfiguration::getUserFeedbackReader() const
{
    return user_feedback_reader_;
}

void ACSACR1222LReaderUnitConfiguration::setUserFeedbackReader(const std::string &name)
{
    user_feedback_reader_ = name;
}

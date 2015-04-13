//
// Created by xaqq on 4/13/15.
//

#include <logicalaccess/cards/tripledeskey.hpp>
#include "omnikey5427readerunitconfiguration.hpp"

using namespace logicalaccess;

Omnikey5427ReaderUnitConfiguration::Omnikey5427ReaderUnitConfiguration() :
        useSecureMode_(false),
        masterKey_(new AES128Key())
{

}

bool Omnikey5427ReaderUnitConfiguration::getUseSecureMode() const
{
    return useSecureMode_;
}

void Omnikey5427ReaderUnitConfiguration::setUseSecureMode(bool use_sm)
{
    useSecureMode_ = use_sm;
}

std::shared_ptr<AES128Key> Omnikey5427ReaderUnitConfiguration::getSecureMasterKey()
{
    return masterKey_;
}

void Omnikey5427ReaderUnitConfiguration::setSecureMasterKey(
        std::shared_ptr<logicalaccess::AES128Key> ptr)
{
    masterKey_ = ptr;
}

void Omnikey5427ReaderUnitConfiguration::resetConfiguration()
{
    PCSCReaderUnitConfiguration::resetConfiguration();
    useSecureMode_ = false;
    masterKey_.reset(new AES128Key());
}

void Omnikey5427ReaderUnitConfiguration::serialize(
        boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    PCSCReaderUnitConfiguration::serialize(node);

    node.put("UseSecureMode", useSecureMode_);
    boost::property_tree::ptree masterKeyNode;
    masterKey_->serialize(masterKeyNode);
    node.add_child("MasterKey", masterKeyNode);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void Omnikey5427ReaderUnitConfiguration::unSerialize(boost::property_tree::ptree &node)
{
    PCSCReaderUnitConfiguration::unSerialize(
            node.get_child(PCSCReaderUnitConfiguration::getDefaultXmlNodeName()));
    useSecureMode_ = node.get_child("UseSecureMode").get_value<bool>();
    masterKey_->unSerialize(node.get_child("MasterKey"), "");
}

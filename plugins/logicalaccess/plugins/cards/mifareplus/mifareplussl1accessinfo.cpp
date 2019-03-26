#include <logicalaccess/plugins/cards/mifareplus/mifareplussl1accessinfo.hpp>

using namespace logicalaccess;

MifarePlusSL1AccessInfo::MifarePlusSL1AccessInfo()
    : aesAuthKey(new AES128Key())
{
}

void MifarePlusSL1AccessInfo::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    MifareAccessInfo::serialize(node);

    boost::property_tree::ptree aes_key;
    aesAuthKey->serialize(aes_key);
    node.add_child("AESAuthKey", aes_key);

    parentNode.add_child(MifarePlusSL1AccessInfo::getDefaultXmlNodeName(), node);
}

void MifarePlusSL1AccessInfo::unSerialize(boost::property_tree::ptree &parentNode)
{
    MifareAccessInfo::unSerialize(
        parentNode.get_child(MifareAccessInfo::getDefaultXmlNodeName()));
    aesAuthKey->unSerialize(parentNode.get_child("AESAuthKey"), "");
}

std::string MifarePlusSL1AccessInfo::getDefaultXmlNodeName() const
{
    return "MifarePlusSL1AccessInfo";
}

std::string MifarePlusSL1AccessInfo::getCardType() const
{
    return "MifarePlusSL1";
}

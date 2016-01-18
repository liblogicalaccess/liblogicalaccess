#include <assert.h>
#include "logicalaccess/cards/IKSStorage.hpp"
#include <boost/property_tree/ptree.hpp>

using namespace logicalaccess;

KeyStorageType IKSStorage::getType() const
{
        return KST_SERVER;
}

void IKSStorage::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    node.put("KeyIdentity", key_identity_);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void IKSStorage::unSerialize(boost::property_tree::ptree &node)
{
    key_identity_ = node.get_child("KeyIdentity").get_value<std::string>();
}

std::string IKSStorage::getDefaultXmlNodeName() const
{
	return "IKSStorage";
}

const std::string &IKSStorage::getKeyIdentity() const
{
        return key_identity_;
}

IKSStorage::IKSStorage(const std::string &key_identity)
    : key_identity_(key_identity)
{
}

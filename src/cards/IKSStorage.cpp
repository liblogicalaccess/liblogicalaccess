#include <assert.h>
#include "logicalaccess/cards/IKSStorage.hpp"

using namespace logicalaccess;

KeyStorageType IKSStorage::getType() const
{
        return KST_SERVER;
}

void IKSStorage::serialize(boost::property_tree::ptree &parentNode)
{
        assert(0 && "Not implemented");
}

void IKSStorage::unSerialize(boost::property_tree::ptree &node)
{
        assert(0 && "Not implemented");
}

std::string IKSStorage::getDefaultXmlNodeName() const
{
        assert(0 && "Not implemented");
}

const std::string &IKSStorage::getKeyIdentity() const
{
        return key_identity_;
}

IKSStorage::IKSStorage(const std::string &key_identity)
    : key_identity_(key_identity)
{
}

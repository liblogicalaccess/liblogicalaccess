/**
 * \file keystorage.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Key storage base class.
 */

#include <logicalaccess/myexception.hpp>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/cards/computermemorykeystorage.hpp>
#include <logicalaccess/cards/readermemorykeystorage.hpp>
#include <logicalaccess/cards/samkeystorage.hpp>
#include <logicalaccess/cards/pkcskeystorage.hpp>

namespace logicalaccess
{
std::shared_ptr<KeyStorage> KeyStorage::getKeyStorageFromType(KeyStorageType kst)
{
    std::shared_ptr<KeyStorage> ret;

    switch (kst)
    {
    case KST_COMPUTER_MEMORY: ret.reset(new ComputerMemoryKeyStorage()); break;

    case KST_READER_MEMORY: ret.reset(new ReaderMemoryKeyStorage()); break;

    case KST_SAM: ret.reset(new SAMKeyStorage()); break;

    case KST_PKCS: ret.reset(new PKCSKeyStorage());
    }

    return ret;
}

std::string KeyStorage::getMetadata(const std::string &key)
{
    if (!hasMetadata(key))
        throw LibLogicalAccessException("Cannot find metadata named " + key);
    return metadata_[key];
}

bool KeyStorage::hasMetadata(const std::string &key) const
{
    return (metadata_.count(key) > 0);
}

void KeyStorage::addMetadata(const std::string &key, const std::string &value)
{
    metadata_[key] = value;
}

void KeyStorage::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    boost::property_tree::ptree m;

    for (auto itr = metadata_.begin(); itr != metadata_.end(); ++itr)
    {
        m.put(itr->first, itr->second);
    }
    node.add_child("metadata", m);
    parentNode.add_child("KeyStorage", node);
}

void KeyStorage::unSerialize(boost::property_tree::ptree &node)
{
    boost::property_tree::ptree key_storage = node.get_child("KeyStorage");

    auto n = key_storage.get_child_optional("metadata");
    if (n)
    {
        for (auto const &current_node : *n)
        {
            const std::string pname  = current_node.first;
            const std::string pvalue = current_node.second.data();

            addMetadata(pname, pvalue);
        }
    }
}
}

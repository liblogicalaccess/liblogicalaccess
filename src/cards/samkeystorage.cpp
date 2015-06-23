/**
 * \file samkeystorage.cpp
 * \author Maxime C. <maxime@islog.com>
 * \brief SAM key storage.
 */

#include "logicalaccess/cards/samkeystorage.hpp"
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
    SAMKeyStorage::SAMKeyStorage()
    {
        d_key_slot = 0x00;
    }

    KeyStorageType SAMKeyStorage::getType() const
    {
        return KST_SAM;
    }

    void SAMKeyStorage::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        node.put("KeySlot", d_key_slot);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void SAMKeyStorage::unSerialize(boost::property_tree::ptree& node)
    {
        d_key_slot = node.get_child("KeySlot").get_value<unsigned char>();
    }

    std::string SAMKeyStorage::getDefaultXmlNodeName() const
    {
        return "SAMKeyStorage";
    }
}
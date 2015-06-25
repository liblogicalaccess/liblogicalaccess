/**
 * \file readermemorykeystorage.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader memory key storage.
 */

#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
    ReaderMemoryKeyStorage::ReaderMemoryKeyStorage()
    {
        d_key_slot = 0x00;
        d_volatile = true;
    }

    KeyStorageType ReaderMemoryKeyStorage::getType() const
    {
        return KST_READER_MEMORY;
    }

    void ReaderMemoryKeyStorage::setKeySlot(unsigned char key_slot)
    {
        d_key_slot = key_slot;
    }

    unsigned char ReaderMemoryKeyStorage::getKeySlot() const
    {
        return d_key_slot;
    }

    void ReaderMemoryKeyStorage::setVolatile(bool vol)
    {
        d_volatile = vol;
    }

    bool ReaderMemoryKeyStorage::getVolatile() const
    {
        return d_volatile;
    }

    void ReaderMemoryKeyStorage::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        node.put("KeySlot", d_key_slot);
        node.put("Volatile", d_volatile);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void ReaderMemoryKeyStorage::unSerialize(boost::property_tree::ptree& node)
    {
        d_key_slot = node.get_child("KeySlot").get_value<unsigned char>();
        d_volatile = node.get_child("Volatile").get_value<bool>();
    }

    std::string ReaderMemoryKeyStorage::getDefaultXmlNodeName() const
    {
        return "ReaderMemoryKeyStorage";
    }
}
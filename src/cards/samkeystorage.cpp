/**
 * \file samkeystorage.cpp
 * \author Maxime C. <maxime@islog.com>
 * \brief SAM key storage.
 */

#include <logicalaccess/cards/samkeystorage.hpp>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
SAMKeyStorage::SAMKeyStorage()
{
    d_key_slot = 0x00;
    d_dumpKey  = false;
}

KeyStorageType SAMKeyStorage::getType() const
{
    return KST_SAM;
}

void SAMKeyStorage::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    node.put("KeySlot", d_key_slot);
	node.put("DumpKey", d_dumpKey);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void SAMKeyStorage::unSerialize(boost::property_tree::ptree &node)
{
    d_key_slot = node.get_child("KeySlot").get_value<unsigned char>();
	boost::optional<boost::property_tree::ptree&> dumpKeyChild = node.get_child_optional("DumpKey");
	if (dumpKeyChild)
	{
		d_dumpKey = dumpKeyChild.get().get_value<bool>();
	}
}

std::string SAMKeyStorage::getDefaultXmlNodeName() const
{
    return "SAMKeyStorage";
}
}

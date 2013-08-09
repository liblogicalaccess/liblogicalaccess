/**
 * \file samkeystorage.cpp
 * \author Maxime C. <maxime@islog.com>
 * \brief SAM key storage.
 */

#include "logicalaccess/cards/samkeystorage.hpp"

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
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void SAMKeyStorage::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		
	}

	std::string SAMKeyStorage::getDefaultXmlNodeName() const
	{
		return "SAMKeyStorage";
	}
}


/**
 * \file ComputerMemoryKeyStorage.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Computer memory key storage.
 */

#include "logicalaccess/Cards/ComputerMemoryKeyStorage.h"

namespace logicalaccess
{
	KeyStorageType ComputerMemoryKeyStorage::getType() const
	{
		return KST_COMPUTER_MEMORY;
	}

	void ComputerMemoryKeyStorage::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void ComputerMemoryKeyStorage::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		
	}

	std::string ComputerMemoryKeyStorage::getDefaultXmlNodeName() const
	{
		return "ComputerMemoryKeyStorage";
	}
}

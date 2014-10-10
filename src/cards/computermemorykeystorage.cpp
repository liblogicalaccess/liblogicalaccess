/**
 * \file computermemorykeystorage.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Computer memory key storage.
 */

#include "logicalaccess/cards/computermemorykeystorage.hpp"

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
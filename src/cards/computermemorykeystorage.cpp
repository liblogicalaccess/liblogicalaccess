/**
 * \file computermemorykeystorage.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Computer memory key storage.
 */

#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
    ComputerMemoryKeyStorage::ComputerMemoryKeyStorage()
    {
        d_locked = false;
        d_random = false;
    }

    KeyStorageType ComputerMemoryKeyStorage::getType() const
    {
        return KST_COMPUTER_MEMORY;
    }

    void ComputerMemoryKeyStorage::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        node.put("Locked", d_locked);
        node.put("Random", d_random);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void ComputerMemoryKeyStorage::unSerialize(boost::property_tree::ptree& node)
    {
        boost::optional<boost::property_tree::ptree&> lockedChild = node.get_child_optional("Locked");
        if (lockedChild)
            d_locked = lockedChild.get().get_value<bool>();
        boost::optional<boost::property_tree::ptree&> randomChild = node.get_child_optional("Random");
        if (randomChild)
            d_random = randomChild.get().get_value<bool>();
    }

    std::string ComputerMemoryKeyStorage::getDefaultXmlNodeName() const
    {
        return "ComputerMemoryKeyStorage";
    }

    void ComputerMemoryKeyStorage::setLocked(bool locked)
    {
        d_locked = locked;
    }

    bool ComputerMemoryKeyStorage::getLocked() const
    {
        return d_locked;
    }

    void ComputerMemoryKeyStorage::setRandom(bool random)
    {
        d_random = random;
    }

    bool ComputerMemoryKeyStorage::getRandom() const
    {
        return d_random;
    }
}
/**
 * \file iso15693chip.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ISO15693 chip.
 */

#include <logicalaccess/plugins/cards/iso15693/iso15693chip.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/plugins/cards/iso15693/iso15693storagecardservice.hpp>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
ISO15693Chip::ISO15693Chip(std::string ct)
    : Chip(ct)
{
}

ISO15693Chip::ISO15693Chip()
    : Chip(CHIP_ISO15693)
{
}

ISO15693Chip::~ISO15693Chip()
{
}

std::shared_ptr<LocationNode> ISO15693Chip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("ISO 15693");
    rootNode->setHasProperties(true);

    std::shared_ptr<ISO15693Location> rootLocation;
    rootLocation.reset(new ISO15693Location());
    // FIXME: We need to add a specific property for this
    rootLocation->block = static_cast<int>(-1);
    rootNode->setLocation(rootLocation);

    if (getCommands())
    {
        ISO15693Commands::SystemInformation sysinfo =
            getISO15693Commands()->getSystemInformation();
        if (sysinfo.hasVICCMemorySize)
        {
            char tmpName[255];
            for (int i = 0; i < sysinfo.nbBlocks; i++)
            {
                std::shared_ptr<LocationNode> blockNode;
                blockNode.reset(new LocationNode());

                sprintf(tmpName, "Block %d", i);
                blockNode->setName(tmpName);
                blockNode->setNeedAuthentication(true);
                blockNode->setHasProperties(true);
                blockNode->setLength(sysinfo.blockSize);

                std::shared_ptr<ISO15693Location> blockLocation;
                blockLocation.reset(new ISO15693Location());
                blockLocation->block = i;
                blockNode->setLocation(blockLocation);

                blockNode->setParent(rootNode);
                rootNode->getChildrens().push_back(blockNode);
            }
        }
    }

    return rootNode;
}

std::shared_ptr<CardService> ISO15693Chip::getService(CardServiceType serviceType)
{
    std::shared_ptr<CardService> service;

    switch (serviceType)
    {
    case CST_STORAGE:
        service.reset(new ISO15693StorageCardService(shared_from_this()));
        break;
    default: break;
    }

    return service;
}

std::shared_ptr<Location> ISO15693Chip::createLocation() const
{
    std::shared_ptr<ISO15693Location> ret;
    ret.reset(new ISO15693Location());
    return ret;
}
}
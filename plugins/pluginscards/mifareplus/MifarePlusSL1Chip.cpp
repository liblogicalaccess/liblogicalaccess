//
// Created by xaqq on 7/1/15.
//

#include <logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp>
#include "MifarePlusSL1Chip.hpp"
#include "mifareplusstoragecardservice_sl1.hpp"
#include "mifareplusaccessinfo_sl1.hpp"

using namespace logicalaccess;


MifarePlusSL1Chip::MifarePlusSL1Chip(const std::string &cardType, int nb_sectors)
    : MifareChip(cardType, nb_sectors)
{
}

MifarePlusSL1_2kChip::MifarePlusSL1_2kChip()
    : MifarePlusSL1Chip("MifarePlus_SL1_2K", 32)
{
}

MifarePlusSL1_4kChip::MifarePlusSL1_4kChip()
    : MifarePlusSL1Chip("MifarePlus_SL1_4K", 40)
{
}

std::shared_ptr<LocationNode> MifarePlusSL1_4kChip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode = MifareChip::getRootLocationNode();

    for (int i = 16; i < 40; i++)
    {
        addSectorNode(rootNode, i);
    }

    return rootNode;
}

std::shared_ptr<CardService> MifarePlusSL1Chip::getService(CardServiceType serviceType)
{
    std::shared_ptr<CardService> service;

    switch (serviceType)
    {
    case CST_ACCESS_CONTROL:
        service.reset(new AccessControlCardService(shared_from_this()));
        break;
    case CST_STORAGE:
        service.reset(new MifarePlusStorageCardServiceSL1(shared_from_this()));
        break;
    default: break;
    }
    return service;
}

int MifarePlusSL1Chip::getSecurityLevel() const
{
    return 1;
}

std::shared_ptr<AccessInfo> MifarePlusSL1Chip::createAccessInfo() const
{
    std::shared_ptr<MifarePlusSL1AccessInfo> ret;
    ret.reset(new MifarePlusSL1AccessInfo());
    return ret;
}
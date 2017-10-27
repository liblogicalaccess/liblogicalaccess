/**
 * \file mifareultralightcchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C chip.
 */

#include "mifareultralightcchip.hpp"
#include "mifareultralightcstoragecardservice.hpp"
#include "nfctag2cardservice.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
MifareUltralightCChip::MifareUltralightCChip()
    : MifareUltralightChip(CHIP_MIFAREULTRALIGHTC)
{
    d_nbblocks = 48;
}

MifareUltralightCChip::~MifareUltralightCChip()
{
}

std::shared_ptr<LocationNode> MifareUltralightCChip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("Mifare Ultralight C");
    checkRootLocationNodeName(rootNode);

    for (unsigned int i = 0; i < getNbBlocks(); ++i)
    {
        addBlockNode(rootNode, i);
    }

    return rootNode;
}

std::shared_ptr<CardService>
MifareUltralightCChip::getService(CardServiceType serviceType)
{
    std::shared_ptr<CardService> service;

    switch (serviceType)
    {
    case CST_ACCESS_CONTROL:
        service.reset(new AccessControlCardService(shared_from_this()));
        break;
    case CST_STORAGE:
        service.reset(new MifareUltralightCStorageCardService(shared_from_this()));
        break;
    case CST_NFC_TAG: service.reset(new NFCTag2CardService(shared_from_this())); break;
    default: break;
    }

    return service;
}

std::shared_ptr<AccessInfo> MifareUltralightCChip::createAccessInfo() const
{
    std::shared_ptr<MifareUltralightCAccessInfo> ret;
    ret.reset(new MifareUltralightCAccessInfo());
    return ret;
}

std::shared_ptr<TripleDESKey> MifareUltralightCChip::getDefaultKey() const
{
    return std::make_shared<TripleDESKey>(
        "49 45 4D 4B 41 45 52 42 21 4E 41 43 55 4F 59 46");
}
}
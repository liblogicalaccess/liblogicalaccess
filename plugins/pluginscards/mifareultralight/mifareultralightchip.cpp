/**
 * \file mifareultralightchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight chip.
 */

#include "mifareultralightchip.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "mifareultralightstoragecardservice.hpp"
#include "logicalaccess/cards/locationnode.hpp"
#include "nfctag2cardservice.hpp"
#include "mifareultralightuidchangerservice.hpp"

namespace logicalaccess
{
MifareUltralightChip::MifareUltralightChip(std::string ct)
    : Chip(ct)
    , d_nbblocks(16)
{
}

MifareUltralightChip::MifareUltralightChip()
    : Chip(CHIP_MIFAREULTRALIGHT)
{
    d_nbblocks = 16;
}

MifareUltralightChip::~MifareUltralightChip()
{
}

unsigned short MifareUltralightChip::getNbBlocks(bool checkOnCard)
{
    if (checkOnCard)
    {
        try
        {
            std::shared_ptr<MifareUltralightCommands> mfucmd =
                getMifareUltralightCommands();
            d_nbblocks = 16; // Mifare Ultralight
            mfucmd->readPage(31);
            d_nbblocks = 32; // NTAG212
            mfucmd->readPage(41);
            d_nbblocks = 42; // NTAG203F
            mfucmd->readPage(43);
            d_nbblocks = 44; // NTAG213
            mfucmd->readPage(47);
            d_nbblocks = 48; // Mifare Ultralight C
            mfucmd->readPage(125);
            d_nbblocks = 126; // NTAG215
            mfucmd->readPage(221);
            d_nbblocks = 222; // NTAG216
        }
        catch (std::exception &)
        {
        }
    }

    return d_nbblocks;
}

void MifareUltralightChip::checkRootLocationNodeName(
    std::shared_ptr<LocationNode> rootNode)
{
    unsigned short nbblocks = getNbBlocks(true);

    switch (nbblocks)
    {
    case 32: rootNode->setName("NTAG212"); break;
    case 42: rootNode->setName("NTAG203"); break;
    case 44: rootNode->setName("NTAG213"); break;
    case 126: rootNode->setName("NTAG215"); break;
    case 222: rootNode->setName("NTAG216"); break;
    default:;
    }
}

std::shared_ptr<LocationNode> MifareUltralightChip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("Mifare Ultralight");
    checkRootLocationNodeName(rootNode);

    for (unsigned short i = 0; i < getNbBlocks(); ++i)
    {
        addBlockNode(rootNode, i);
    }

    return rootNode;
}

void MifareUltralightChip::addBlockNode(std::shared_ptr<LocationNode> rootNode, int block)
{
    char tmpName[255];
    std::shared_ptr<LocationNode> blockNode;
    blockNode.reset(new LocationNode());

    sprintf(tmpName, "Block %d", block);
    blockNode->setName(tmpName);
    blockNode->setLength(4);
    blockNode->setNeedAuthentication(true);
    blockNode->setCanWrite(block > 2);

    std::shared_ptr<MifareUltralightLocation> location;
    location.reset(new MifareUltralightLocation());
    location->page  = block;
    location->byte_ = 0;

    blockNode->setLocation(location);
    blockNode->setParent(rootNode);
    rootNode->getChildrens().push_back(blockNode);
}

std::shared_ptr<CardService> MifareUltralightChip::getService(CardServiceType serviceType)
{
    std::shared_ptr<CardService> service;

    switch (serviceType)
    {
    case CST_ACCESS_CONTROL:
        service.reset(new AccessControlCardService(shared_from_this()));
        break;
    case CST_STORAGE:
        service.reset(new MifareUltralightStorageCardService(shared_from_this()));
        break;
    case CST_NFC_TAG: service.reset(new NFCTag2CardService(shared_from_this())); break;
    case CST_UID_CHANGER:
    {
        auto storage =
            std::make_shared<MifareUltralightStorageCardService>(shared_from_this());
        service.reset(new MifareUltralightUIDChangerService(storage));
        break;
    }
    default: break;
    }

    return service;
}

std::shared_ptr<AccessInfo> MifareUltralightChip::createAccessInfo() const
{
    std::shared_ptr<MifareUltralightAccessInfo> ret;
    ret.reset(new MifareUltralightAccessInfo());
    return ret;
}

std::shared_ptr<Location> MifareUltralightChip::createLocation() const
{
    std::shared_ptr<MifareUltralightLocation> ret;
    ret.reset(new MifareUltralightLocation());
    return ret;
}
}
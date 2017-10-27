/**
* \file nfctag1cardservice.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief NFC Tag Type 1 card service.
*/

#include <logicalaccess/logs.hpp>
#include "nfctag1cardservice.hpp"
#include "logicalaccess/services/nfctag/ndefmessage.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
void NFCTag1CardService::writeNDEF(std::shared_ptr<NdefMessage> records)
{
    std::shared_ptr<StorageCardService> storage =
        std::dynamic_pointer_cast<StorageCardService>(
            getTopazChip()->getService(CST_STORAGE));
    std::shared_ptr<TopazLocation> location(new TopazLocation);
    unsigned short nbblocks = getTopazChip()->getNbBlocks();
    location->page          = 1;
    ByteVector data;

    // Capability Container
    data.push_back(0xE1);                                     // NDEF present on tag
    data.push_back(0x10);                                     // Support version 1.0
    data.push_back(static_cast<unsigned char>(nbblocks - 1)); // Tag Type
    data.push_back(0x00);                                     // No Lock

    ByteVector tlv = NdefMessage::NdefMessageToTLV(records);
    data.insert(data.end(), tlv.begin(), tlv.end());

    storage->writeData(location, std::shared_ptr<AccessInfo>(),
                       std::shared_ptr<AccessInfo>(), data, CB_AUTOSWITCHAREA);
}

std::shared_ptr<NdefMessage> NFCTag1CardService::readNDEF()
{
    std::shared_ptr<TopazCommands> tcmd(getTopazChip()->getTopazCommands());
    std::shared_ptr<NdefMessage> ndef;

    ByteVector CC = tcmd->readPage(1);
    // Only take care if NDEF is present
    if (CC.size() >= 4 && CC[0] == 0xE1)
    {
        if (CC[2] > 0)
        {
            // Read all available data from data blocks
            // Limited to block 0xE for now
            ByteVector data = tcmd->readPages(1, 1 + ((CC[2] <= 14) ? CC[2] : 14));
            data            = ByteVector(data.begin() + 4, data.end());
            ndef            = NdefMessage::TLVToNdefMessage(data);
        }
    }

    return ndef;
}

void NFCTag1CardService::eraseNDEF()
{
    std::shared_ptr<TopazCommands> tcmd(getTopazChip()->getTopazCommands());

    // Overwrite capability container
    ByteVector CC(8, 0x00);
    tcmd->writePage(1, CC);
}
}
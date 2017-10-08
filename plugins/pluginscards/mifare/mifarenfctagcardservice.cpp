/**
* \file mifarenfctagcardservice.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Mifare NFC Tag Type card service.
*/

#include <logicalaccess/logs.hpp>
#include "mifarenfctagcardservice.hpp"
#include "mifarelocation.hpp"
#include "logicalaccess/services/nfctag/ndefmessage.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    void MifareNFCTagCardService::writeNDEF(std::shared_ptr<NdefMessage> records)
    {
        std::shared_ptr<StorageCardService> storage = std::dynamic_pointer_cast<StorageCardService>(getMifareChip()->getService(CST_STORAGE));
        std::shared_ptr<MifareLocation> location(new MifareLocation());
        std::shared_ptr<MifareAccessInfo> aiToWrite(new MifareAccessInfo());
        location->sector = 1;
        location->useMAD = true;
        location->aid = 0x03E1;
        aiToWrite->keyA.reset(new MifareKey("ff ff ff ff ff ff"));
        aiToWrite->sab.setTransportConfiguration();
        //aiToWrite->keyB.reset(new MifareKey("ff ff ff ff ff ff"));
        //aiToWrite->sab.setAReadBWriteConfiguration();
        aiToWrite->gpb = 0x40;
        aiToWrite->madKeyA.reset(new MifareKey("ff ff ff ff ff ff"));
        aiToWrite->madGPB = 0xC1;
        
        storage->writeData(location, std::shared_ptr<AccessInfo>(), aiToWrite, NdefMessage::NdefMessageToTLV(records), CB_AUTOSWITCHAREA);
    }

    std::shared_ptr<NdefMessage> MifareNFCTagCardService::readNDEF()
    {
        std::shared_ptr<StorageCardService> storage = std::dynamic_pointer_cast<StorageCardService>(getMifareChip()->getService(CST_STORAGE));
        std::shared_ptr<MifareLocation> location(new MifareLocation());

        location->sector = 1;
        location->useMAD = true;
        location->aid = 0x03E1;

        // Read all available data from sector
        ByteVector data = storage->readData(location, std::shared_ptr<MifareAccessInfo>(), 48, CB_AUTOSWITCHAREA);
        try
        {
            location->sector = 2;
            ByteVector data2 = storage->readData(location, std::shared_ptr<MifareAccessInfo>(), 48, CB_AUTOSWITCHAREA);
            data.insert(data.end(), data2.begin(), data2.end());
        }
        catch (const std::exception&) { }

        return NdefMessage::TLVToNdefMessage(data);
    }

    void MifareNFCTagCardService::eraseNDEF()
    {
        // TODO: read MAD and format appropriate sector if setup?
    }
}
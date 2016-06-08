/**
* \file nfctag3cardservice.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief NFC Tag Type 3 card service.
*/

#include <logicalaccess/logs.hpp>
#include "nfctag3cardservice.hpp"
#include "logicalaccess/services/nfctag/ndefmessage.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    void NFCTag3CardService::writeNDEF(std::shared_ptr<logicalaccess::NdefMessage> records)
    {
        std::shared_ptr<logicalaccess::StorageCardService> storage = std::dynamic_pointer_cast<StorageCardService>(getFeliCaChip()->getService(CST_STORAGE));
        std::shared_ptr<logicalaccess::FeliCaCommands> cmd = getFeliCaChip()->getFeliCaCommands();
        EXCEPTION_ASSERT_WITH_LOG(cmd, CardException, "FeliCa commands not implemented on this reader.");
        std::shared_ptr<logicalaccess::FeliCaLocation> location(new logicalaccess::FeliCaLocation);
        location->code = FELICA_CODE_NDEF_WRITE;
        location->block = 1;
        std::vector<unsigned char> data;
        if (records)
            data = records->encode();
        std::vector<unsigned char> data0 = cmd->read(location->code, 0);
        EXCEPTION_ASSERT(data0.size() >= 16, CardException, "Wrong Attribute Information block length.");
        // Attribute Information block
        data0[9] = 0x00; // WriteFlag OFF
        // Len
        data0[11] = static_cast<unsigned char>((data.size() >> 16) & 0xFF);
        data0[12] = static_cast<unsigned char>((data.size() >> 8) & 0xFF);
        data0[13] = static_cast<unsigned char>(data.size() & 0xFF);
        unsigned short crc = 0;
        for (unsigned char i = 0; i < 14; ++i)
        {
            crc += data0[i];
        }
        data0[14] = static_cast<unsigned char>((crc >> 8) & 0xFF);
        data0[15] = static_cast<unsigned char>(crc & 0xFF);
        
        cmd->write(location->code, 0, data0);
        if (data.size() > 0)
            storage->writeData(location, std::shared_ptr<logicalaccess::AccessInfo>(), std::shared_ptr<logicalaccess::AccessInfo>(), data, CB_AUTOSWITCHAREA);
    }

    std::shared_ptr<logicalaccess::NdefMessage> NFCTag3CardService::readNDEF()
    {
        std::shared_ptr<logicalaccess::StorageCardService> storage = std::dynamic_pointer_cast<StorageCardService>(getFeliCaChip()->getService(CST_STORAGE));
        std::shared_ptr<logicalaccess::FeliCaCommands> cmd = getFeliCaChip()->getFeliCaCommands();
        EXCEPTION_ASSERT_WITH_LOG(cmd, CardException, "FeliCa commands not implemented on this reader.");
        std::shared_ptr<logicalaccess::FeliCaLocation> location(new logicalaccess::FeliCaLocation);
        location->code = FELICA_CODE_NDEF_READ;
        location->block = 1;
        std::shared_ptr<logicalaccess::NdefMessage> ndef;

        std::vector<unsigned char> data0 = cmd->read(location->code, 0);
        EXCEPTION_ASSERT(data0.size() >= 16, CardException, "Wrong Attribute Information block length.");
        unsigned short rcrc = (data0[14] << 8) | data0[15];
        unsigned short crc = 0;
        for (unsigned char i = 0; i < 14; ++i)
        {
            crc += data0[i];
        }
        EXCEPTION_ASSERT(crc == rcrc, CardException, "Wrong FeliCa Attribute Information CRC.");

        unsigned int ndeflen = (data0[11] << 16) | (data0[12] << 8) | data0[13];
        if (ndeflen > 0)
        {
            std::vector<unsigned char> data = storage->readData(location, std::shared_ptr<logicalaccess::AccessInfo>(), ndeflen, CB_AUTOSWITCHAREA);
            ndef.reset(new NdefMessage(data));
        }

        return ndef;
    }

    void NFCTag3CardService::eraseNDEF()
    {
        writeNDEF(std::shared_ptr<logicalaccess::NdefMessage>());
    }
}
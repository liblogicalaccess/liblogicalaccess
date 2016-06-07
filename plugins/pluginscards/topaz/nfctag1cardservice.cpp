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
	void NFCTag1CardService::writeNDEF(std::shared_ptr<logicalaccess::NdefMessage> records)
	{
        std::shared_ptr<logicalaccess::StorageCardService> storage = std::dynamic_pointer_cast<StorageCardService>(getTopazChip()->getService(CST_STORAGE));
        std::shared_ptr<logicalaccess::TopazLocation> location(new logicalaccess::TopazLocation);
        unsigned short nbblocks = getTopazChip()->getNbBlocks();
        location->page = 1;
		std::vector<unsigned char> data;

        // Capability Container
        data.push_back(0xE1); // NDEF present on tag
        data.push_back(0x10); // Support version 1.0
        data.push_back(static_cast<unsigned char>(nbblocks)); // Tag Type
        data.push_back(0x00); // No Lock

        std::vector<unsigned char> tlv = NdefMessage::NdefMessageToTLV(records);
        data.insert(data.end(), tlv.begin(), tlv.end());

        storage->writeData(location, std::shared_ptr<logicalaccess::AccessInfo>(), std::shared_ptr<logicalaccess::AccessInfo>(), data, CB_AUTOSWITCHAREA);
	}

	std::shared_ptr<logicalaccess::NdefMessage> NFCTag1CardService::readNDEF()
	{
		std::shared_ptr<logicalaccess::TopazCommands> tzmd(getTopazChip()->getTopazCommands());
		std::shared_ptr<logicalaccess::NdefMessage> ndef;
		
        std::vector<unsigned char> CC = tzmd->readPage(1);
		// Only take care if NDEF is present
		if (CC.size() == 4 && CC[0] == 0xE1)
		{
			if (CC[2] > 0)
			{
				// Read all available data from data blocks
                std::vector<unsigned char> data = tzmd->readPages(1, CC[2]);
                data = std::vector<unsigned char>(data.begin() + 4, data.end());
                ndef = NdefMessage::TLVToNdefMessage(data);
			}
		}

		return ndef;
	}

	void NFCTag1CardService::eraseNDEF()
	{
        std::shared_ptr<logicalaccess::StorageCardService> storage = std::dynamic_pointer_cast<StorageCardService>(getTopazChip()->getService(CST_STORAGE));
        storage->erase();
	}
}
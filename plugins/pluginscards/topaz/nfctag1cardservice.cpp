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
        data.push_back(0xE1);	// NDEF present on tag
        data.push_back(0x10); // Support version 1.0
        data.push_back(static_cast<unsigned char>(nbblocks)); // Tag Type
        data.push_back(0x00);   // No Lock

		data.push_back(0x03); // T = NDEF
		std::vector<unsigned char> recordsData = records->encode();
		data.push_back(static_cast<unsigned char>(recordsData.size()));
		data.insert(data.end(), recordsData.begin(), recordsData.end());
		data.push_back(0xFE); // T = Terminator
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
				unsigned short i = 4;
				while (i + 1u < data.size())
				{
					switch (data[i++])
					{
					case 0x00: // Null
						break;
					case 0x01: // Lock
					case 0x02: // Memory control
					case 0xFD: // Proprietary
						i += data[i];
						break;
					case 0x03: // Ndef message
						if (data.size() >= i + 1u + data[i])
						{
							ndef.reset(new NdefMessage(std::vector<unsigned char>(data.begin() + i + 1, data.begin() + i + 1 + data[i])));
							i += data[i];
						}
						// TODO: support multiple ndef message
						// Ndef found, leave
                        i = static_cast<unsigned short>(data.size());
						break;
					case 0xFE: // Terminator
						// Just leave
                        i = static_cast<unsigned short>(data.size());
						break;
					}
				}
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
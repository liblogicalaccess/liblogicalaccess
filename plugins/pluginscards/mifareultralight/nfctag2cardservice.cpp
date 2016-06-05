/**
* \file nfctag4cardservice.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief NFC Tag Type 2 card service.
*/

#include <logicalaccess/logs.hpp>
#include "nfctag2cardservice.hpp"
#include "logicalaccess/services/nfctag/ndefmessage.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
	void NFCTag2CardService::writeCapabilityContainer()
	{
		std::shared_ptr<logicalaccess::MifareUltralightCommands> mfucmd(getMifareUltralightChip()->getMifareUltralightCommands());
		unsigned short nbblocks = getMifareUltralightChip()->getNbBlocks();

		std::vector<unsigned char> CC;
		CC.push_back(0xE1);	// NDEF present on tag
		CC.push_back(0x10); // Support version 1.0
		CC.push_back((nbblocks - 4) / 2); // Tag Type (= nb 8-bytes data blocks)
		CC.push_back(0x00);   // No Lock
		mfucmd->writePage(3, CC);
	}

	void NFCTag2CardService::writeNDEF(std::shared_ptr<logicalaccess::NdefMessage> records)
	{
		writeCapabilityContainer();

		std::vector<unsigned char> data;
		data.push_back(0x03); // T = NDEF
		std::vector<unsigned char> recordsData = records->encode();
		data.push_back(static_cast<unsigned char>(recordsData.size()));
		data.insert(data.end(), recordsData.begin(), recordsData.end());
		data.push_back(0xFE); // T = Terminator
	}

	std::shared_ptr<logicalaccess::NdefMessage> NFCTag2CardService::readNDEF()
	{
		std::shared_ptr<logicalaccess::MifareUltralightCommands> mfucmd(getMifareUltralightChip()->getMifareUltralightCommands());
		std::shared_ptr<logicalaccess::NdefMessage> ndef;

		// TODO: support Dynamic Memory Structure
		// See NFC Forum Tag Type 2, Chapter 2.2
		
		std::vector<unsigned char> CC = mfucmd->readPage(0x03);
		// Only take care if NDEF is present
		if (CC.size() == 4 && CC[0] == 0xE1)
		{
			if (CC[2] > 0)
			{
				// Read all available data from data blocks
				std::vector<unsigned char> data = mfucmd->readPages(4, 4 + (CC[2] * 2));
				unsigned short i = 0;
				while (i + 1 < data.size())
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
						if (data.size() >= i + 1 + data[i])
						{
							ndef.reset(new NdefMessage(std::vector<unsigned char>(data.begin() + i + 1, data.begin() + i + 1 + data[i])));
							i += data[i];
						}
						// TODO: support multiple ndef message
						// Ndef found, leave
						i = data.size();
						break;
					case 0xFE: // Terminator
						// Just leave
						i = data.size();
						break;
					}
				}
			}
		}

		return ndef;
	}

	void NFCTag2CardService::eraseNDEF()
	{
		std::shared_ptr<logicalaccess::MifareUltralightCommands> mfucmd(getMifareUltralightChip()->getMifareUltralightCommands());
		std::vector<unsigned char> empty(4, 0x00);

		for (unsigned short i = 4; i < getMifareUltralightChip()->getNbBlocks(); ++i)
		{			
			mfucmd->writePage(i, empty);
		}
	}
}
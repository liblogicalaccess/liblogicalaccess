/**
* \file nfctag2cardservice.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief NFC Tag Type 2 card service.
*/

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/nfctag2cardservice.hpp>
#include <logicalaccess/services/nfctag/ndefmessage.hpp>
#include <logicalaccess/services/storage/storagecardservice.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
void NFCTag2CardService::writeCapabilityContainer() const
{
    std::shared_ptr<MifareUltralightCommands> mfucmd(
        getMifareUltralightChip()->getMifareUltralightCommands());
    unsigned short nbblocks = getMifareUltralightChip()->getNbBlocks();

    ByteVector CC;
    CC.push_back(0xE1); // NDEF present on tag
    CC.push_back(0x10); // Support version 1.0
    CC.push_back(static_cast<unsigned char>((nbblocks - 4) /
                                            2)); // Tag Type (= nb 8-bytes data blocks)
    CC.push_back(0x00);                          // No Lock
    mfucmd->writePage(3, CC);
}

void NFCTag2CardService::writeNDEF(std::shared_ptr<NdefMessage> records)
{
    std::shared_ptr<StorageCardService> storage =
        std::dynamic_pointer_cast<StorageCardService>(
            getMifareUltralightChip()->getService(CST_STORAGE));
    std::shared_ptr<MifareUltralightLocation> location(new MifareUltralightLocation());
    location->page = 4;
    writeCapabilityContainer();

    storage->writeData(location, std::shared_ptr<AccessInfo>(),
                       std::shared_ptr<AccessInfo>(),
                       NdefMessage::NdefMessageToTLV(records), CB_AUTOSWITCHAREA);
}

void NFCTag2CardService::writeData(std::shared_ptr<NfcData> records, int addr)
{
    std::shared_ptr<StorageCardService> storage =
        std::dynamic_pointer_cast<StorageCardService>(
            getMifareUltralightChip()->getService(CST_STORAGE));
    ByteVector data = NfcData::dataToTLV(records);
    ByteVector tmp = {};
    unsigned int size = data.size();
    std::shared_ptr<MifareUltralightLocation> location(new MifareUltralightLocation());

    readData();
    addr += 16;
    location->page = addr / 4;
    location->byte_ = addr % 4;
    writeCapabilityContainer();
    for (unsigned int i = 0; i != size; i++)
    {
      int c = checkForReservedArea(i + addr);
      if (c != -1)
      {
        if (tmp.size() != 0)
        {
          location->page = addr / 4;
          location->byte_ = addr % 4;
          writeCapabilityContainer();
          storage->writeData(location, std::shared_ptr<AccessInfo>(),
                             std::shared_ptr<AccessInfo>(),
                             tmp, CB_AUTOSWITCHAREA);
        }
        addr += c + tmp.size() + 1;
        tmp.clear();
      }
      tmp.push_back(data[i]);
    }
    if (tmp.size() != 0)
    {
      location->page = addr / 4;
      location->byte_ = addr % 4;
      writeCapabilityContainer();
      storage->writeData(location, std::shared_ptr<AccessInfo>(),
                         std::shared_ptr<AccessInfo>(),
                         tmp, CB_AUTOSWITCHAREA);
    }
  }

std::shared_ptr<NdefMessage> NFCTag2CardService::readNDEF()
{
    std::shared_ptr<MifareUltralightCommands> mfucmd(
        getMifareUltralightChip()->getMifareUltralightCommands());
    std::vector<std::shared_ptr<NfcData>> dataList;
    ByteVector res;

    ByteVector CC = mfucmd->readPage(3);
    // Only take care if NDEF is present
    if (CC.size() >= 4 && CC[0] == 0xE1)
    {
        if (CC[2] > 0)
        {
            // Read all available data from data blocks
            ByteVector data = mfucmd->readPages(4, 4 + (CC[2] * 2) - 1);
            //ndef            = NdefMessage::TLVToNdefMessage(data);
            fillMemoryList(data);
            unsigned int size = data.size();
            unsigned int r;
            for (unsigned int i = 0; i != size; i++)
            {
              if ((r = checkForReservedArea(i)) != 0)
                i += r;
              else
                res.push_back(data[i]);
            }
            dataList = NfcData::tlvToData(res);
        }
    }

    return  nullptr;
}

std::vector<std::shared_ptr<NfcData>> NFCTag2CardService::readData()
{
    std::shared_ptr<MifareUltralightCommands> mfucmd(
        getMifareUltralightChip()->getMifareUltralightCommands());
    std::vector<std::shared_ptr<NfcData>> dataList;
    ByteVector res;

    ByteVector CC = mfucmd->readPage(3);
    // Only take care if NDEF is present
    if (CC.size() >= 4 && CC[0] == 0xE1)
    {
        if (CC[2] > 0)
        {
            // Read all available data from data blocks
            ByteVector data = mfucmd->readPages(4, 4 + (CC[2] * 2) - 1);
            fillMemoryList(data);
            unsigned int size = data.size();
            int r;
            for (unsigned int i = 0; i != size; i++)
            {
              if ((r = checkForReservedArea(i + 16)) != -1)
                i += r;
              else
                res.push_back(data[i]);
            }
            dataList = NfcData::tlvToData(res);
        }
    }
    return  dataList;
}

void NFCTag2CardService::eraseNDEF()
{
    std::shared_ptr<MifareUltralightCommands> mfucmd(
        getMifareUltralightChip()->getMifareUltralightCommands());

    // Overwrite capability container
    ByteVector CC(4, 0x00);
    mfucmd->writePage(3, CC);
}

int NFCTag2CardService::checkForReservedArea(unsigned int i)
{
  unsigned int res = -1;
  unsigned int size = _memoryList.size();

  for (unsigned int j = 0; j != size; j++)
  {
    if (i >= _memoryList[j].byteAddr && i < (_memoryList[j].byteAddr + _memoryList[j].size))
    {
      return _memoryList[j].byteAddr + _memoryList[j].size - i - 1;
    }
  }
  return res;
}

void NFCTag2CardService::fillMemoryList(ByteVector data)
{
  _memoryList.clear();
  size_t size =  data.size();
  for (size_t i = 0; i != size; i++)
  {
    if (data[i] == 0x02 && i + 5 < size)
    {
      MemoryData md;
      md.byteAddr = data[i + 2];
      md.size = data[i + 3];
      _memoryList.push_back(md);
      i+=4;
    }
  }
}
}

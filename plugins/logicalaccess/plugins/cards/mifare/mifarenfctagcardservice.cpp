/**
* \file mifarenfctagcardservice.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Mifare NFC Tag Type card service.
*/

#define MAD_KEY "a0 a1 a2 a3 a4 a5"
#define EMPTY_SECTOR_KEY "ff ff ff ff ff ff"
#define FULL_SECTOR_KEY "d3 f7 d3 f7 d3 f7"

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarenfctagcardservice.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarelocation.hpp>
#include <logicalaccess/services/nfctag/ndefmessage.hpp>
#include <logicalaccess/myexception.hpp>
#include <unordered_map>

namespace logicalaccess
{
void MifareNFCTagCardService::writeInfo(int baseAddr, std::shared_ptr<MifareKey> keyB, ByteVector tmpBuffer, bool useMad, std::shared_ptr<StorageCardService> storage)
{
  std::shared_ptr<MifareLocation> location(new MifareLocation());
  std::shared_ptr<MifareAccessInfo> aiToWrite(new MifareAccessInfo());
  std::shared_ptr<MifareKey> madKey = std::make_shared<MifareKey>(MAD_KEY);
  MifareAccessInfo::SectorAccessBits sab;

  sab.setNfcConfiguration();
  location->sector = baseAddr / 64;
  location->block = (baseAddr / 16) % 4;
  if ((baseAddr / 16) % 4 == 3)
  {
    location->sector++;
    location->block = 0;
  }
  location->useMAD = useMad;
  location->aid    = 0x03E1;
  aiToWrite->keyA.reset(new MifareKey(FULL_SECTOR_KEY));
  aiToWrite->keyB.reset(new MifareKey(keyB->getData(), keyB->getLength()));
  aiToWrite->sab.setNfcConfiguration();
  aiToWrite->gpb = 0x40;
  aiToWrite->madKeyA.reset(new MifareKey(madKey->getData(), madKey->getLength()));
  aiToWrite->madKeyB.reset(new MifareKey(keyB->getData(), keyB->getLength()));
  aiToWrite->madGPB = 0xC1;

  int endSector = (baseAddr + tmpBuffer.size()) / 64;
  if (((baseAddr + tmpBuffer.size()) / 16) % 4 == 3)
    endSector++;
  if  ((baseAddr / 16) % 4 == 3)
    endSector++;
  while (endSector >= location->sector)
  {
    /*
    ** if the card is not formated for ndef messages we format it with default ndef keys
    */
    try
    {
      storage->writeData(location, aiToWrite, std::shared_ptr<AccessInfo>(),
                        tmpBuffer, CB_AUTOSWITCHAREA);
    }
    catch (std::exception &)
    {
      getMifareChip()->getMifareCommands()->changeKeys(KT_KEY_A ,std::make_shared<MifareKey>(EMPTY_SECTOR_KEY), aiToWrite->keyA, keyB, endSector, &sab, 0x40);
      if (endSector == location->sector)
      {
        storage->writeData(location, aiToWrite, std::shared_ptr<AccessInfo>(),
                        tmpBuffer, CB_AUTOSWITCHAREA);
      }
    }
    endSector--;
  }
}

void MifareNFCTagCardService::writeInfo(int baseAddr, std::shared_ptr<MifareAccessInfo> aiToWrite, ByteVector tmpBuffer, std::shared_ptr<StorageCardService> storage)
{
  std::shared_ptr<MifareLocation> location(new MifareLocation());
  MifareAccessInfo::SectorAccessBits sab;
  std::shared_ptr<MifareAccessInfo> newAiToWrite =  std::make_shared<MifareAccessInfo>();
  std::shared_ptr<MifareKey> nfcKey = std::make_shared<MifareKey>(FULL_SECTOR_KEY);

  sab.setNfcConfiguration();
  location->sector = baseAddr / 64;
  location->block = (baseAddr / 16) % 4;
  if ((baseAddr / 16) % 4 == 3)
  {
    location->sector++;
    location->block = 0;
  }
  location->useMAD = false;
  location->aid    = 0x03E1;
  newAiToWrite->keyA.reset(new MifareKey(nfcKey->getData(), nfcKey->getLength()));
  newAiToWrite->keyB.reset(new MifareKey(aiToWrite->keyB->getData(), aiToWrite->keyB->getLength()));
  newAiToWrite->sab.setNfcConfiguration();
  newAiToWrite->gpb = 0x40;
  newAiToWrite->madKeyA.reset(new MifareKey(aiToWrite->madKeyA->getData(), aiToWrite->madKeyA->getLength()));
  newAiToWrite->madKeyB.reset(new MifareKey(aiToWrite->madKeyB->getData(), aiToWrite->madKeyB->getLength()));
  newAiToWrite->madGPB = 0xC1;

  int endSector = (baseAddr + tmpBuffer.size()) / 64;
  if (((baseAddr + tmpBuffer.size()) / 16) % 4 == 3)
    endSector++;
  if  ((baseAddr / 16) % 4 == 3)
    endSector++;
  while (endSector >= location->sector)
  {
    /*
    ** if the card is not formated for ndef messages we format it with default ndef keys
    */
    try
    {
      storage->writeData(location, newAiToWrite, std::shared_ptr<AccessInfo>(),
                        tmpBuffer, CB_AUTOSWITCHAREA);
    }
    catch (std::exception &)
    {
      getMifareChip()->getMifareCommands()->changeKeys(KT_KEY_A , aiToWrite->keyA, nfcKey, aiToWrite->keyB, endSector, &sab, 0x40);
      if (endSector == location->sector)
      {
        storage->writeData(location, newAiToWrite, std::shared_ptr<AccessInfo>(),
                        tmpBuffer, CB_AUTOSWITCHAREA);
      }
    }
    endSector--;
  }
}

void MifareNFCTagCardService::writeNFC(std::shared_ptr<NfcData> records, int sector, std::shared_ptr<AccessInfo> ai, MifareAccessInfo::SectorAccessBits madSab)
{
  std::shared_ptr<StorageCardService> storage =
      std::dynamic_pointer_cast<StorageCardService>(
          getMifareChip()->getService(CST_STORAGE));
  std::shared_ptr<MifareLocation> location(new MifareLocation());
  std::shared_ptr<MifareAccessInfo> aiToWrite = std::dynamic_pointer_cast<MifareAccessInfo>(ai);
  MifareAccessInfo::SectorAccessBits sab;
  std::shared_ptr<MifareKey> madKey = std::make_shared<MifareKey>(MAD_KEY);
  std::shared_ptr<logicalaccess::MifareKey> keyNdefSector = std::make_shared<logicalaccess::MifareKey>(FULL_SECTOR_KEY);
  unsigned char mad;
  ByteVector madSector;
  std::map<int, ByteVector > mapBuffer;

  sab.setAReadBWriteConfiguration();
  if (*(aiToWrite->madKeyA.get()) != *(madKey.get()))
  {
    aiToWrite->madGPB = 0xC1;
    try
    {
      madSector = getMifareChip()->getMifareCommands()->readSector(0, 1 ,madKey, aiToWrite->madKeyB, sab, true);
    }
    catch (std::exception &)
    {
      getMifareChip()->getMifareCommands()->changeKeys(KT_KEY_A ,aiToWrite->madKeyA , madKey, aiToWrite->madKeyB, 0, &sab, aiToWrite->madGPB);
      madSector = getMifareChip()->getMifareCommands()->readSector(0, 1 ,madKey, aiToWrite->madKeyB, sab, true);
    }
    aiToWrite->madKeyA = madKey;
  }
  else
    madSector = getMifareChip()->getMifareCommands()->readSector(0, 1 ,aiToWrite->madKeyA, aiToWrite->madKeyB, sab, true);

  for (unsigned int i = 2; i + 1 < madSector.size(); i += 2)
  {
    if (madSector[i] == 0x03 && madSector[i + 1] == 0xE1)
    {
      MifareAccessInfo::SectorAccessBits sab2;
      sab2.setAReadBWriteConfiguration();
      try
      {
        ByteVector smallBuffer = getMifareChip()->getMifareCommands()->readSector(i / 2, 0, keyNdefSector, aiToWrite->madKeyB, sab2);
        mapBuffer[i / 2] = smallBuffer;
      }
      catch (std::exception &)
      {

      }
    }
  }
  _memoryList.clear();
  for (auto it = mapBuffer.begin(); it != mapBuffer.end(); ++it)
  {
    if (it->second[0] != 0x03) // if the sector doesn't begin with a ndef message
      fillMemoryList(it->second);
  }
  ByteVector bigBuffer = NfcData::dataToTLV(records);
  int additionalSectors = bigBuffer.size() / 48;
  while (additionalSectors >= 0)
  {
    madSector[(sector + additionalSectors) * 2] = 0x03;
    madSector[(sector + additionalSectors) * 2 + 1] = 0xE1;
    additionalSectors--;
  }
  unsigned char *buffer = new unsigned char[madSector.size()];
  for (unsigned int i = 0; i < madSector.size() - 1; ++i)
    buffer[i] = madSector[i + 1];
  buffer[madSector.size() - 1] = 0;
  mad = getMifareChip()->getMifareCommands()->calculateMADCrc(buffer, madSector.size());
  delete[] buffer;
  madSector[0] = mad;
  getMifareChip()->getMifareCommands()->writeSector(0, 1, madSector, aiToWrite->madKeyA, aiToWrite->madKeyB, sab, true);
  ByteVector tmpBuffer;
  int byteAddr = sector * 64;
  int baseAddr = byteAddr;
  sab.setNfcConfiguration();
  for (unsigned int i = 0; i < bigBuffer.size(); i++)
  {
    int c = checkForReservedArea(byteAddr);
    if (c != -1)
    {
      writeInfo(baseAddr, aiToWrite, tmpBuffer, storage);
      byteAddr += c + 1;
      int mod = byteAddr % 16;
      if (mod != 0)
      {
        mod = 16 - mod;
        byteAddr += mod;
      }
      baseAddr = byteAddr;
      tmpBuffer.clear();
    }
    tmpBuffer.push_back(bigBuffer[i]);
    byteAddr++;
  }
  writeInfo(baseAddr, aiToWrite, tmpBuffer, storage);
}

void MifareNFCTagCardService::writeNFC(std::shared_ptr<NfcData> records, int sector, std::shared_ptr<MifareKey> keyA, std::shared_ptr<MifareKey> keyB, bool useMad)
{
    std::shared_ptr<logicalaccess::MifareAccessInfo> aiToWrite(
        new logicalaccess::MifareAccessInfo());
    aiToWrite->keyA = keyA;
    aiToWrite->keyB = keyB;
    aiToWrite->gpb = 0x40;
    aiToWrite->madKeyA.reset(new logicalaccess::MifareKey(MAD_KEY));
    aiToWrite->madKeyB = keyB;
    aiToWrite->madGPB = 0xC1;

    writeNFC(records, sector, aiToWrite);
  }

std::vector<std::shared_ptr<NfcData> > MifareNFCTagCardService::readNFC()
{
    std::shared_ptr<logicalaccess::MifareAccessInfo> aiToWrite(
        new logicalaccess::MifareAccessInfo());
    aiToWrite->keyA.reset(new logicalaccess::MifareKey(FULL_SECTOR_KEY));
    aiToWrite->keyB.reset(new logicalaccess::MifareKey(MAD_KEY));
    aiToWrite->gpb = 0x40;
    aiToWrite->madKeyA.reset(new logicalaccess::MifareKey(FULL_SECTOR_KEY));
    aiToWrite->madKeyB.reset(new logicalaccess::MifareKey(MAD_KEY));
    aiToWrite->madGPB = 0xC1;

	return readNFC(aiToWrite, 2);
}

std::vector<std::shared_ptr<NfcData> > MifareNFCTagCardService::readNFC(std::shared_ptr<AccessInfo> ai, unsigned int sector)
{
    std::vector<std::shared_ptr<NfcData> > stock;
    std::shared_ptr<StorageCardService> storage =
        std::dynamic_pointer_cast<StorageCardService>(
            getMifareChip()->getService(CST_STORAGE));
    std::shared_ptr<MifareLocation> location(new MifareLocation());
    MifareAccessInfo::SectorAccessBits sab;
    std::map<int, ByteVector > bigBuffer;
    std::shared_ptr<MifareAccessInfo> aiToWrite = std::dynamic_pointer_cast<MifareAccessInfo>(ai);
    std::shared_ptr<MifareKey> keyNdefSector = std::make_shared<MifareKey>(FULL_SECTOR_KEY);

    sab.setAReadBWriteConfiguration();

    ByteVector madSector = getMifareChip()->getMifareCommands()->readSector(0, 1, std::make_shared<MifareKey>(MAD_KEY), aiToWrite->madKeyB, sab);
    MifareAccessInfo::SectorAccessBits sab2;
    sab2.setNfcConfiguration();
    if (sector != 0)
    {
      ByteVector smallBuffer = getMifareChip()->getMifareCommands()->readSector(sector, 0, keyNdefSector, aiToWrite->keyB, sab, true);
      ByteVector trailer(smallBuffer.begin() + 54, smallBuffer.end() - 6 );
      smallBuffer.erase(smallBuffer.begin() + 48, smallBuffer.end());
      if (trailer[0] == 0x7F && trailer[1] == 0x07 && trailer[2] == 0x88 && trailer[3] == 0x40)
        bigBuffer[sector] = smallBuffer;
    }
    else
    {
      for (unsigned int i = 2; i + 1 < madSector.size(); i += 2)
      {
        if (madSector[i] == 0x03 && madSector[i + 1] == 0xE1)
        {
          ByteVector smallBuffer = getMifareChip()->getMifareCommands()->readSector(i / 2, 0,keyNdefSector, aiToWrite->keyB, sab, true);
          ByteVector trailer(smallBuffer.begin() + 54, smallBuffer.end() - 6 );
          smallBuffer.erase(smallBuffer.begin() + 48, smallBuffer.end());
          if (trailer[0] == 0x7F && trailer[1] == 0x07 && trailer[2] == 0x88 && trailer[3] == 0x40)
            bigBuffer[i / 2] = smallBuffer;
        }
      }
    }
    _memoryList.clear();
    for (auto it = bigBuffer.begin(); it != bigBuffer.end(); ++it)
    {
      if (it->second[0] != 0x03) // if the sector doesn't begin with a ndef message
        fillMemoryList(it->second);
    }
    for (unsigned int i = 0; i != _memoryList.size(); i++)
    {
      int sector = _memoryList[i].byteAddr / 64;
      int byte = _memoryList[i].byteAddr % 64;
      int size = _memoryList[i].size;
      if (bigBuffer.count(sector) > 0)
      {
        bigBuffer[sector].erase(bigBuffer[sector].begin() + byte, bigBuffer[sector].begin() + byte + size);
      }
    }
    ByteVector finalBuffer;
    for (auto c : bigBuffer)
      finalBuffer.insert(finalBuffer.end(),c.second.begin(), c.second.end());
    stock = NfcData::tlvToData(finalBuffer);
    return stock;
}

std::shared_ptr<NdefMessage> MifareNFCTagCardService::readNDEF()
{
  return std::dynamic_pointer_cast<NdefMessage>(readNFC().back());
}

void MifareNFCTagCardService::eraseNfc(std::shared_ptr<MifareKey> sectorKeyB, std::shared_ptr<MifareKey> madKeyB)
{
  MifareAccessInfo::SectorAccessBits sab;
  std::shared_ptr<logicalaccess::MifareKey> factoryKey = std::make_shared<logicalaccess::MifareKey>(EMPTY_SECTOR_KEY);


  ByteVector madSector = getMifareChip()->getMifareCommands()->readSector(0, 1,std::make_shared<MifareKey>(MAD_KEY), madKeyB, sab);
  for (unsigned int i = 2; i + 1 < madSector.size(); i += 2)
  {
    if (madSector[i] == 0x03 && madSector[i + 1] == 0xE1)
    {
      eraseNfc(i / 2, sectorKeyB, madKeyB);
    }
  }
}

void MifareNFCTagCardService::eraseNfc(int sector, std::shared_ptr<MifareKey> sectorKeyB, std::shared_ptr<MifareKey> madKeyB)
{
  MifareAccessInfo::SectorAccessBits sab;
  std::shared_ptr<logicalaccess::MifareKey> factoryKey = std::make_shared<logicalaccess::MifareKey>(EMPTY_SECTOR_KEY);
  ByteVector n;

  n.resize(48);
  std::fill(n.begin(), n.end(), 0);
  ByteVector madSector = getMifareChip()->getMifareCommands()->readSector(0, 1,std::make_shared<MifareKey>(MAD_KEY), madKeyB, sab);
  if (sector * 2 + 1 < madSector.size() && madSector[sector * 2] == 0x03  &&  madSector[sector * 2 + 1] == 0xE1)
  {
    getMifareChip()->getMifareCommands()->changeKeys(logicalaccess::KT_KEY_B ,sectorKeyB, factoryKey, factoryKey, sector, &sab, 0x00);
    getMifareChip()->getMifareCommands()->writeSector(sector, 0, n, factoryKey, factoryKey, sab);
    madSector[sector * 2] = 0x00;
    madSector[sector * 2 + 1] = 0x00;
    unsigned char *buffer = new unsigned char[madSector.size()];
    for (unsigned int i = 0; i < madSector.size() - 1; ++i)
      buffer[i] = madSector[i + 1];
    buffer[madSector.size() - 1] = 0;
    int mad = getMifareChip()->getMifareCommands()->calculateMADCrc(buffer, madSector.size());
    delete[] buffer;
    madSector[0] = mad;
    sab.setAReadBWriteConfiguration();
    getMifareChip()->getMifareCommands()->writeSector(0, 1, madSector, std::make_shared<MifareKey>(MAD_KEY), madKeyB, sab);
  }
}

ByteVector MifareNFCTagCardService::getCardData()
{
  ByteVector data;
  std::shared_ptr<MifareKey> key = std::make_shared<logicalaccess::MifareKey>(EMPTY_SECTOR_KEY);
  MifareAccessInfo::SectorAccessBits sab;
  std::shared_ptr<MifareCommands> cmd = getMifareChip()->getMifareCommands();

  data = cmd->readSectors(0, 15, 0, key, key, sab);
  data.erase(data.begin(), data.begin() + 16);
  return data;
}

void MifareNFCTagCardService::fillMemoryList(ByteVector data)
{
  size_t size =  data.size();
  for (size_t i = 0; i != size && data[i] != 0x03; i++)
  {
    if (data[i] == 0x02 && i + 5 < size && data[i + 1]  == 3)
    {
      MemoryData md;
      md.byteAddr = data[i + 2];
      md.size = data[i + 3];
      _memoryList.push_back(md);
      i+=4;
    }
  }
}

int MifareNFCTagCardService::checkForReservedArea(unsigned int i)
{
  int res = -1;
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
}

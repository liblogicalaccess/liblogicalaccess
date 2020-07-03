/**
* \file mifarenfctagcardservice.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Mifare Classic NFC Tag card service.
*/

#ifndef LOGICALACCESS_MIFARENFCTAGCARDSERVICE_HPP
#define LOGICALACCESS_MIFARENFCTAGCARDSERVICE_HPP

#include <logicalaccess/services/nfctag/nfctagcardservice.hpp>
#include <logicalaccess/services/nfctag/ndefmessage.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarechip.hpp>
#include <logicalaccess/services/nfctag/memorycontroltlv.hpp>
#include <logicalaccess/services/storage/storagecardservice.hpp>

namespace logicalaccess
{
#define MIFARE_NFC_CARDSERVICE "MifareNFCTag"

/**
* \brief The Mifare Classic NFC Tag storage card service base class.
*/
class LLA_CARDS_MIFARE_API MifareNFCTagCardService : public NFCTagCardService
{
  public:
    /**
    * \brief Constructor.
    * \param chip The chip.
    */
    explicit MifareNFCTagCardService(std::shared_ptr<Chip> chip)
        : NFCTagCardService(chip)
    {
    }

    virtual ~MifareNFCTagCardService()
    {
    }

    std::string getCSType() override
    {
        return MIFARE_NFC_CARDSERVICE;
    }

    std::shared_ptr<NdefMessage> readNDEF() override;

    void eraseNfc(std::shared_ptr<MifareKey> sectorKeyB = std::make_shared<MifareKey>("ff ff ff ff ff ff"), std::shared_ptr<MifareKey> madKeyB = std::make_shared<MifareKey>("ff ff ff ff ff ff"));
    void eraseNfc(int sector, std::shared_ptr<MifareKey> sectorKeyB = std::make_shared<MifareKey>("ff ff ff ff ff ff"), std::shared_ptr<MifareKey> madKeyB = std::make_shared<MifareKey>("ff ff ff ff ff ff"));
    std::vector<std::shared_ptr<NfcData> > readNFC();
    std::vector<std::shared_ptr<NfcData> > readNFC(std::shared_ptr<AccessInfo> ai, unsigned int sector = 0);
    void writeNFC(std::shared_ptr<NfcData> records, int sector, std::shared_ptr<AccessInfo> ai, MifareAccessInfo::SectorAccessBits madSab = MifareAccessInfo::SectorAccessBits());
    void writeNFC(std::shared_ptr<NfcData> records,int sector, std::shared_ptr<MifareKey> keyA, std::shared_ptr<MifareKey> keyB, bool useMad = false);
    void fillMemoryList(ByteVector data);
    ByteVector getCardData();
    int checkForReservedArea(unsigned int i);
  protected:
    void writeInfo(int baseAddr, std::shared_ptr<MifareKey> keyB, ByteVector tmpBuff, bool useMad, std::shared_ptr<StorageCardService> storage);
    void writeInfo(int baseAddr, std::shared_ptr<MifareAccessInfo> aiToWrite, ByteVector tmpBuffer, std::shared_ptr<StorageCardService> storage);
    std::shared_ptr<MifareChip> getMifareChip() const
    {
        return std::dynamic_pointer_cast<MifareChip>(getChip());
    }
    std::vector<MemoryData> _memoryList;
};
}

#endif /* LOGICALACCESS_MIFARENFCTAGCARDSERVICE_HPP */

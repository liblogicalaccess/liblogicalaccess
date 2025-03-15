#pragma once

#include <logicalaccess/plugins/readers/iso7816/commands/desfireev2iso7816commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev3commands.hpp>

namespace logicalaccess
{
#define CMD_DESFIREEV3ISO7816 "DESFireEV3ISO7816"

class LLA_READERS_ISO7816_API DESFireEV3ISO7816Commands
    : public DESFireEV2ISO7816Commands
#ifndef SWIG
    ,
      public DESFireEV3Commands
#endif
{
  public:
    DESFireEV3ISO7816Commands();

    std::shared_ptr<Chip> getChip() const override;

    std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const override;

    void changeKeyEV2(uint8_t keyset, uint8_t keyno,
                      std::shared_ptr<DESFireKey> key) override;

    void authenticateEV2First(uint8_t keyno, std::shared_ptr<DESFireKey> key) override;

    void sam_authenticateEV2First(uint8_t keyno,
                                  std::shared_ptr<DESFireKey> key) override;

    void createApplication(
        unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys,
        DESFireKeyType cryptoMethod, FidSupport fidSupported = FIDS_NO_ISO_FID,
        unsigned short isoFID = 0x00, ByteVector isoDFName = ByteVector(),
        unsigned char numberKeySets = 0, unsigned char maxKeySize = 0,
        unsigned char actualkeySetVersion = 0, unsigned char rollkeyno = 0,
        bool specificCapabilityData = false, bool specificVCKeys = false) override;

    void createDelegatedApplication(
        std::pair<ByteVector, ByteVector> damInfo, unsigned int aid,
        unsigned short DAMSlotNo, unsigned char DAMSlotVersion,
        unsigned short quotatLimit, DESFireKeySettings settings, unsigned char maxNbKeys,
        DESFireKeyType cryptoMethod, FidSupport fidSupported, unsigned short isoFID,
        ByteVector isoDFName, unsigned char numberKeySets, unsigned char maxKeySize,
        unsigned char actualkeySetVersion, unsigned char rollkeyno,
        bool specificCapabilityData, bool specificVCKeys) override;

    std::pair<ByteVector, ByteVector> createDAMChallenge(
        std::shared_ptr<DESFireKey> DAMMACKey, std::shared_ptr<DESFireKey> DAMENCKey,
        std::shared_ptr<DESFireKey> DAMDefaultKey, unsigned int aid,
        unsigned short DAMSlotNo, unsigned char DAMSlotVersion,
        unsigned short quotatLimit, DESFireKeySettings settings, unsigned char maxNbKeys,
        DESFireKeyType cryptoMethod, FidSupport fidSupported, unsigned short isoFID,
        ByteVector isoDFName, unsigned char numberKeySets, unsigned char maxKeySize,
        unsigned char actualkeySetVersion, unsigned char rollkeyno,
        bool specificCapabilityData, bool specificVCKeys) override;

    void initializeKeySet(uint8_t keySetNo, DESFireKeyType keySetType) override;

    void rollKeySet(uint8_t keySetNo) override;

    void finalizeKeySet(uint8_t keySetNo, uint8_t keySetVersion) override;

    void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                           const DESFireAccessRights &accessRights,
                           unsigned int fileSize, unsigned short isoFID,
                           bool multiAccessRights) override;

    void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                           const DESFireAccessRights &accessRights,
                           unsigned int fileSize,
                           unsigned short isoFID,
                           bool multiAccessRights,
                           bool sdmAndMirroring) override;

    void createBackupFile(unsigned char fileno, EncryptionMode comSettings,
                          const DESFireAccessRights &accessRights,
                          unsigned int fileSize, unsigned short isoFID,
                          bool multiAccessRights) override;

    void createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                const DESFireAccessRights &accessRights,
                                unsigned int fileSize,
                                unsigned int maxNumberOfRecords,
                                unsigned short isoFID, bool multiAccessRights) override;

    void createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                const DESFireAccessRights &accessRights,
                                unsigned int fileSize,
                                unsigned int maxNumberOfRecords,
                                unsigned short isoFID, bool multiAccessRights) override;

    void createTransactionMACFile(unsigned char fileno, EncryptionMode comSettings,
                                  const DESFireAccessRights &accessRights,
                                  std::shared_ptr<DESFireKey> tmkey) override;

    ByteVector getKeyVersion(uint8_t keysetno, uint8_t keyno) override;

    void setConfiguration(bool formatCardEnabled, bool randomIdEnabled,
                          bool PCMandatoryEnabled, bool AuthVCMandatoryEnabled) override;

    void setConfiguration(uint8_t sak1, uint8_t sak2) override;

    void setConfiguration(bool D40SecureMessagingEnabled, bool EV1SecureMessagingEnabled,
                          bool EV2ChainedWritingEnabled) override;

    void setConfigurationPDCap(uint8_t pdcap1_2, uint8_t pdcap2_5,
                               uint8_t pdcap2_6) override;

    void setConfiguration(ByteVector DAMMAC, ByteVector ISODFNameOrVCIID) override;

    void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
                            std::vector<DESFireAccessRights> accessRights) override;

    void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
                            std::vector<DESFireAccessRights> accessRights,
                            bool sdmAndMirroring,
                            unsigned int tmcLimit,
                            bool sdmVCUID,
                            bool sdmReadCtr,
                            bool sdmReadCtrLimit,
                            bool sdmEncFileData,
                            bool asciiEncoding,
                            DESFireAccessRights sdmAccessRights,
                            unsigned int vcuidOffset,
                            unsigned int sdmReadCtrOffset,
                            unsigned int piccDataOffset,
                            unsigned int sdmMacInputOffset,
                            unsigned int sdmEncOffset,
                            unsigned int sdmEncLength,
                            unsigned int sdmMacOffset,
                            unsigned int sdmReadCtrLimitValue) override;

    void proximityCheck(std::shared_ptr<DESFireKey> key, uint8_t chunk_size) override;

    ByteVector commitTransaction(bool return_tmac) override;

    ByteVector commitReaderID(ByteVector readerid) override;

    void restoreTransfer(unsigned char target_fileno, unsigned char source_fileno) override;

    ByteVector readSignature(unsigned char address = 0x00) override;

    bool performECCOriginalityCheck() override;

    ByteVector getFileCounters(unsigned char fileno) override;
};

}

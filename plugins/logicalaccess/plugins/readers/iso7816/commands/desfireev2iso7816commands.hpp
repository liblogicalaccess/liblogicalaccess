#pragma once

#include <logicalaccess/plugins/readers/iso7816/commands/desfireev1iso7816commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/iso7816iso7816commands.hpp>

namespace logicalaccess
{
#define CMD_DESFIREEV2ISO7816 "DESFireEV2ISO7816"

class LLA_READERS_ISO7816_API DESFireEV2ISO7816Commands
    : public DESFireEV1ISO7816Commands
#ifndef SWIG
      ,
      public DESFireEV2Commands
#endif
{
  public:
    DESFireEV2ISO7816Commands();

    explicit DESFireEV2ISO7816Commands(const std::string &cmd_type);


    void changeKeyEV2(uint8_t keyset, uint8_t keyno,
                      std::shared_ptr<DESFireKey> key) override;

    void authenticateEV2First(uint8_t keyno, std::shared_ptr<DESFireKey> key) override;

    void sam_authenticateEV2First(uint8_t keyno,
                                  std::shared_ptr<DESFireKey> key) override;

    /**
     * \brief Authenticate, given a key number.
     * \param keyno The key number.
     * \param key The key.
     */
    void authenticate(unsigned char keyno, std::shared_ptr<DESFireKey> key) override;

    void changeKey(unsigned char keyno, std::shared_ptr<DESFireKey> newkey) override;

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
        DESFireKeyType cryptoMethod, FidSupport fidSupported = FIDS_NO_ISO_FID,
        unsigned short isoFID = 0x00, ByteVector isoDFName = ByteVector(),
        unsigned char numberKeySets = 0, unsigned char maxKeySize = 0,
        unsigned char actualkeySetVersion = 0, unsigned char rollkeyno = 0,
        bool specificCapabilityData = false, bool specificVCKeys = false) override;

    std::pair<ByteVector, ByteVector> createDAMChallenge(
        std::shared_ptr<DESFireKey> DAMMACKey, std::shared_ptr<DESFireKey> DAMENCKey,
        std::shared_ptr<DESFireKey> DAMDefaultKey, unsigned int aid,
        unsigned short DAMSlotNo, unsigned char DAMSlotVersion,
        unsigned short quotatLimit, DESFireKeySettings settings, unsigned char maxNbKeys,
        DESFireKeyType cryptoMethod, FidSupport fidSupported = FIDS_NO_ISO_FID,
        unsigned short isoFID = 0x00, ByteVector isoDFName = ByteVector(),
        unsigned char numberKeySets = 0, unsigned char maxKeySize = 0,
        unsigned char actualkeySetVersion = 0, unsigned char rollkeyno = 0,
        bool specificCapabilityData = false, bool specificVCKeys = false) override;

    void initliazeKeySet(uint8_t keySetNo, DESFireKeyType keySetType) override;

    void rollKeySet(uint8_t keySetNo) override;

    void finalizeKeySet(uint8_t keySetNo, uint8_t keySetVersion) override;

    /**
     * \brief Change file settings of a specific file in the current application.
     * \param fileno The file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param plain Communication is currently in plain data.
     */
    void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
                            std::vector<DESFireAccessRights> accessRights,
                            bool plain) override;

    /**
     * \brief Create a new data file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size (in bytes).
     */
    virtual void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                                   const DESFireAccessRights &accessRights,
                                   unsigned int fileSize, unsigned short isoFID,
                                   bool multiAccessRights);

    /**
     * \brief Create a new backup file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size (in bytes).
     */
    virtual void createBackupFile(unsigned char fileno, EncryptionMode comSettings,
                                  const DESFireAccessRights &accessRights,
                                  unsigned int fileSize, unsigned short isoFID,
                                  bool multiAccessRights);

    /**
     * \brief Create a new linear record file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size (in bytes).
     * \param maxNumberOfRecords Max number of records in the file.
     */
    virtual void createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                        const DESFireAccessRights &accessRights,
                                        unsigned int fileSize,
                                        unsigned int maxNumberOfRecords,
                                        unsigned short isoFID, bool multiAccessRights);

    /**
     * \brief Create a new cyclic record file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size (in bytes).
     * \param maxNumberOfRecords Max number of records in the file.
     */
    virtual void createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                        const DESFireAccessRights &accessRights,
                                        unsigned int fileSize,
                                        unsigned int maxNumberOfRecords,
                                        unsigned short isoFID, bool multiAccessRights);

    void createTransactionMACFile(unsigned char fileno, EncryptionMode comSettings,
                                  const DESFireAccessRights &accessRights,
                                  std::shared_ptr<DESFireKey> tmkey,
                                  unsigned char tmkversion) override;

    ByteVector getKeyVersion(uint8_t keysetno, uint8_t keyno) override;

    void setConfiguration(bool formatCardEnabled, bool randomIdEnabled,
                          bool PCMandatoryEnabled, bool AuthVCMandatoryEnabled) override;

    void setConfiguration(uint8_t sak1, uint8_t sak2) override;

    void setConfiguration(bool D40SecureMessagingEnabled, bool EV1SecureMessagingEnabled,
                          bool EV2ChainedWritingEnabled) override;

    void setConfigurationPDCap(uint8_t pdcap1_2, uint8_t pdcap2_5, uint8_t pdcap2_6) override;

    void setConfiguration(ByteVector DAMMAC, ByteVector ISODFNameOrVCIID) override;

    std::shared_ptr<Chip> getChip() const override
    {
        return DESFireEV1ISO7816Commands::getChip();
    }

    std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const override
    {
        return DESFireEV1ISO7816Commands::getReaderCardAdapter();
    }

    void proximityCheck(std::shared_ptr<DESFireKey> key, uint8_t chunk_size) override;

  private:
    /**
     * \brief Transmit a command.
     * \param cmd The command code.
     * \param buf The command buffer.
     * \param buflen The command buffer length.
     * \param forceLc Undocumented.
     * \return The response.
     */
    ISO7816Response transmit(unsigned char cmd, const ByteVector &buf = ByteVector(),
                        unsigned char lc = 0, bool forceLc = false) override;

    /**
     * \brief Transmit a command.
     * \param cmd The command code.
     * \param buf The command buffer.
     * \param buflen The command buffer length.
     * \param forceLc Undocumented.
     * \return The response.
     */
    ISO7816Response transmit_nomacv(unsigned char cmd,
                                    const ByteVector &buf = ByteVector(),
                               unsigned char lc = 0, bool forceLc = false) override;

    /**
     * \brief Generic method to create read file cmd.
     * \param cmd The command to send
     * \param mode The communication mode
     * \return The data buffer.
     */
    ISO7816Response handleReadCmd(unsigned char cmd, const ByteVector &data,
                             EncryptionMode mode) override;

    /**
     * \brief Generic method to read data from a file.
     * \param err The last error code
     * \param firstMsg The first message to send
     * \param length The length to read
     * \param mode The communication mode
     * \return The data buffer.
     */
    ByteVector handleReadData(unsigned char err, const ByteVector &firstMsg,
                              unsigned int length, EncryptionMode mode) override;

    /**
     * \brief Generic method to write data into a file.
     * \param cmd The command to send
     * \param parameters The command parameters
     * \param data The data buffer
     * \param mode The communication mode
     */
    void handleWriteData(unsigned char cmd, const ByteVector &parameters,
                         const ByteVector &data, EncryptionMode mode) override;

    static ByteVector createDelegatedApplicationParam(
        unsigned int aid, unsigned short DAMSlotNo, unsigned char DAMSlotVersion,
        unsigned short quotatLimit, DESFireKeySettings settings, unsigned char maxNbKeys,
        DESFireKeyType cryptoMethod, FidSupport fidSupported, unsigned short isoFID,
        ByteVector isoDFName, unsigned char numberKeySets, unsigned char maxKeySize,
        unsigned char actualkeySetVersion, unsigned char rollkeyno,
        bool specificCapabilityData, bool specificVCKeys);
};
}

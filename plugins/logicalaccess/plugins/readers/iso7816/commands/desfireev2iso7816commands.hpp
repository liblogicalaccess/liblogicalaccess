#pragma once

#include <logicalaccess/plugins/readers/iso7816/commands/desfireev1iso7816commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/iso7816iso7816commands.hpp>

#include <logicalaccess/plugins/cards/desfire/transactionmacinput.hpp>
#include <logicalaccess/plugins/cards/desfire/transactionmaccrypto.hpp>

#include <cstdint>

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

    void sam_authenticate_p2(unsigned char keyno, ByteVector rndap, bool authev2) const;

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

    void initializeKeySet(uint8_t keySetNo, DESFireKeyType keySetType) override;

    void rollKeySet(uint8_t keySetNo) override;

    void finalizeKeySet(uint8_t keySetNo, uint8_t keySetVersion) override;

    /**
     * \brief Change file settings of a specific file in the current application.
     * \param fileno The file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     */
    void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
                            std::vector<DESFireAccessRights> accessRights) override;

    void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                                   const DESFireAccessRights &accessRights,
                                   unsigned int fileSize, unsigned short isoFID,
                                   bool multiAccessRights) override;

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

    CommitTransactionResult commitTransaction(bool return_tmac) override;

    TransactionMACFileData readTransactionMacFile(unsigned char fileno) override;

    ByteVector commitReaderID(ByteVector readerid) override;

    void restoreTransfer(unsigned char target_fileno, unsigned char source_fileno) override;

    ByteVector readSignature(unsigned char address = 0x00) override;

    bool performECCOriginalityCheck() override;

    // Explicitly rejects NoCredential and Signature results. It doesn't interpret the signature structure or TMV
    [[nodiscard]] std::uint32_t decodeTransactionMACCounter(const CommitTransactionResult &result) override;

    [[nodiscard]] std::uint32_t decodeTransactionCounter(const TransactionManagement::TransactionCounter &tmc) override;

    /**
     * \brief Configures host side Transaction MAC verification.
     *
     * Validates and stores the application Transaction MAC key, card UID and Transaction MAC file number.
     * Initializes transaction MAC input tracking and resets the previously committed TMI so that subsequent
     * transactions can be verified using the configured parameters.
     *
     * \param applicationTMACKey Application Transaction MAC key used for session key
     *                           derivation and Transaction MAC verification.
     * \param uid Card UID used during Transaction MAC session-key derivation.
     * \param transactionMACFileNo File number of the application's Transaction MAC file.
     */
    void configureTransactionMAC(const ByteVector &applicationTMACKey, const ByteVector &uid,
                                 unsigned char transactionMACFileNo) override;

    void resetTransactionMACInput() override;

    ByteVector getTransactionMACInput() const override;

    [[nodiscard]] bool verifyTransactionMAC(const TransactionMACFileData &committed) const override;

    void abortTransaction() override;

    ByteVector readData(unsigned char fileno, unsigned int offset, unsigned int length, EncryptionMode mode) override;

    void writeData(unsigned char fileno, unsigned int offset, const ByteVector &data, EncryptionMode mode) override;

    int32_t getValue(unsigned char fileno, EncryptionMode mode) override;

    void credit(unsigned char fileno, unsigned int value, EncryptionMode mode) override;

    void debit(unsigned char fileno, unsigned int value, EncryptionMode mode) override;

    void limitedCredit(unsigned char fileno, int32_t value, EncryptionMode mode) override;

    ByteVector readRecords(unsigned char fileno, unsigned int offset, unsigned int length, EncryptionMode mode) override;

    void writeRecord(unsigned char fileno, unsigned int offset, const ByteVector &data, EncryptionMode mode) override;

    void clearRecordFile(unsigned char fileno) override;

  protected:
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

    virtual void validateTransactionMACConfiguration(const ByteVector &applicationTMACKey, const ByteVector &uid) const;
    
    /**
     * \brief Initializes host side Transaction MAC Input (TMI) tracking.
     *
     * Clears the current TMI and invalidates any previously committed transaction input.
     * Enables tracking of transaction-related commands so that the accumulated TMI can be
     * retained when a transaction is successfully committed.
     *
     * This method initializes transaction input tracking independently of the cryptographic mechanism
     * used to authenticate the transaction and therefore does not configure or derive any transaction MAC keys.
     */
    void initializeTransactionMACInputTracking(unsigned char fileNo);

    void onTransactionCommitted();
    void onTransactionAborted();

    // Shared Transaction Management configuration and lifecycle
    unsigned char transactionManagementFileNo_ = 0xFF;
    TransactionMACState transactionMACState_   = TransactionMACState::Disabled;

    // Transaction MAC Input (TMI) shared tracking
    DESFireTransactionMACInput transactionMACInput_;
    ByteVector lastCommittedTransactionMACInput_;
    bool hasCommittedTransactionMACInput_ = false;

    // Mechanism specific TMAC/Transaction MAC Value (TMAC/TMV) configuration
    ByteVector transactionMACKey_;
    ByteVector transactionMACUID_;
    bool transactionMACConfigured_ = false;

    // ReaderID
    bool readerIDCommitted_ = false;

  private:

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

    bool shouldTrackTransactionInput(unsigned char fileno) const;
    bool shouldTrackTransactionMAC(unsigned char fileno) const;


};

} // namespace logicalaccess
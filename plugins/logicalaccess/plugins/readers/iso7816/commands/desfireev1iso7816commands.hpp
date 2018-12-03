/**
 * \file desfireev1iso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 commands.
 */

#ifndef LOGICALACCESS_DESFIREEV1ISO7816COMMANDS_HPP
#define LOGICALACCESS_DESFIREEV1ISO7816COMMANDS_HPP

#include <logicalaccess/plugins/readers/iso7816/commands/desfireiso7816commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/iso7816iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/lla_readers_iso7816_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define DESFIREEV1_CLEAR_DATA_LENGTH_CHUNK 60
#define CMD_DESFIREEV1ISO7816 "DESFireEV1ISO7816"

/**
 * \brief The DESFire EV1 base commands class.
 */
class LLA_READERS_ISO7816_API DESFireEV1ISO7816Commands : public DESFireISO7816Commands
#ifndef SWIG
                                                          ,
                                                          public DESFireEV1Commands
#endif
{
  public:
    /**
     * \brief Constructor.
     */
    DESFireEV1ISO7816Commands();

    explicit DESFireEV1ISO7816Commands(std::string ct);

    /**
     * \brief Destructor.
     */
    virtual ~DESFireEV1ISO7816Commands();

    /**
     * \brief Get the value of available bytes.
     * \return The available memory in bytes.
     */
    unsigned int getFreeMem() override;

    /**
     * \brief Get the ISO DF-Names of all active application.
     * \return The DF-Names list.
     */
    std::vector<DFName> getDFNames() override;

    /**
     * \brief Get the ISO FID of all active files within the currently selected
     * application.
     * \return The ISO FID list.
     */
    std::vector<unsigned short> getISOFileIDs() override;
#ifndef SWIG
    using DESFireISO7816Commands::createApplication;
#endif

    /**
     * \brief Create a new application.
     * \param aid The Application ID to create
     * \param settings Key settings
     * \param maxNbKeys Maximum number of keys
     */
    void createApplication(unsigned int aid, DESFireKeySettings settings,
                           unsigned char maxNbKeys, DESFireKeyType cryptoMethod,
                           FidSupport fidSupported = FIDS_NO_ISO_FID,
                           unsigned short isoFID   = 0x00,
                           ByteVector isoDFName    = ByteVector()) override;
#ifndef SWIG
    using DESFireISO7816Commands::getKeySettings;
#endif

    /**
     * \brief Get key settings on the current application.
     * \param settings Key settings
     * \param maxNbKeys Maximum number of keys
     * \param keyType The key type
     */
    void getKeySettings(DESFireKeySettings &settings, unsigned char &maxNbKeys,
                        DESFireKeyType &keyType) override;

    /**
     * \brief Get a random card UID.
     * \return The card UID.
     */
    ByteVector getCardUID() override;
#ifndef SWIG
    using DESFireISO7816Commands::createStdDataFile;
#endif

    /**
     * \brief Create a new data file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size (in bytes).
     */
    void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                           const DESFireAccessRights &accessRights, unsigned int fileSize,
                           unsigned short isoFID) override;
#ifndef SWIG
    using DESFireISO7816Commands::createBackupFile;
#endif

    /**
     * \brief Create a new backup file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size (in bytes).
     */
    void createBackupFile(unsigned char fileno, EncryptionMode comSettings,
                          const DESFireAccessRights &accessRights, unsigned int fileSize,
                          unsigned short isoFID) override;
#ifndef SWIG
    using DESFireISO7816Commands::createLinearRecordFile;
#endif

    /**
     * \brief Create a new linear record file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size (in bytes).
     * \param maxNumberOfRecords Max number of records in the file.
     */
    void createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                const DESFireAccessRights &accessRights,
                                unsigned int fileSize, unsigned int maxNumberOfRecords,
                                unsigned short isoFID) override;
#ifndef SWIG
    using DESFireISO7816Commands::createCyclicRecordFile;
#endif

    /**
     * \brief Create a new cyclic record file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size (in bytes).
     * \param maxNumberOfRecords Max number of records in the file.
     */
    void createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                const DESFireAccessRights &accessRights,
                                unsigned int fileSize, unsigned int maxNumberOfRecords,
                                unsigned short isoFID) override;

    /**
     * \brief Select file under current DF.
     * \param fid The FID
     */
    void iso_selectFile(unsigned short fid = 0) override;

    /**
     * \brief Read records.
     * \param fid The FID
     * \param start_record The start record (0 = read last written record)
     * \param record_number The number of records to read
     * \return The record(s) data
     */
    ByteVector
    iso_readRecords(unsigned short fid = 0, unsigned char start_record = 0,
                    DESFireRecords record_number = DF_RECORD_ONERECORD) override;

    /**
     * \brief Append a record to a file.
     * \param data The record data
     * \param fid The FID
     */
    void iso_appendrecord(const ByteVector &data = ByteVector(),
                          unsigned short fid     = 0) override;

    /**
     * \brief Get the ISO challenge for authentication.
     * \param length The challenge length (8 = 2K3DES, 16 = 3K3DES and AES)
     * \return The ISO challenge.
     */
    ByteVector iso_getChallenge(unsigned int length = 8) override;

    /**
     * \brief ISO external authenticate.
     * \param algorithm The ISO algorithm to use for authentication.
     * \param isMasterCardKey True if the key to authenticate is the master card key,
     * false otherwise.
     * \param keyno The key number.
     * \param data The data.
     */
    void iso_externalAuthenticate(DESFireISOAlgorithm algorithm = DF_ALG_BY_CONTEXT,
                                  bool isMasterCardKey = true, unsigned char keyno = 0x00,
                                  const ByteVector &data = ByteVector()) override;

    /**
     * \brief ISO internal authenticate.
     * \param algorithm The ISO algorithm to use for authentication.
     * \param isMasterCardKey True if the key to authenticate is the master card key,
     * false otherwise.
     * \param keyno The key number.
     * \param RPCD2 The RPCD2.
     * \param length The length.
     * \return The cryptogram.
     */
    ByteVector iso_internalAuthenticate(DESFireISOAlgorithm algorithm = DF_ALG_BY_CONTEXT,
                                        bool isMasterCardKey          = true,
                                        unsigned char keyno           = 0x00,
                                        const ByteVector &RPCD2       = ByteVector(),
                                        unsigned int length           = 16) override;

    /**
     * \brief ISO authenticate.
     * \param key The key.
     * \param algorithm The ISO algorithm to use for authentication.
     * \param isMasterCardKey True if the key to authenticate is the master card key,
     * false otherwise.
     * \param keyno The key number.
     */
    virtual void iso_authenticate(std::shared_ptr<DESFireKey> key,
                                  DESFireISOAlgorithm algorithm = DF_ALG_2K3DES,
                                  bool isMasterCardKey          = true,
                                  unsigned char keyno           = 0x00);

    // Only for AES PKCS key.
    void pkcs_iso_authenticate(std::shared_ptr<DESFireKey> key,
                               bool isMasterCardKey = true, unsigned char keyno = 0x00);

    /**
     * \brief Authenticate, given a key number.
     * \param keyno The key number.
     * \param key The key.
     */
    void authenticate(unsigned char keyno, std::shared_ptr<DESFireKey> key) override;

    /**
     * \brief AuthenticateISO command.
     * \param keyno The key number.
     * \param algorithm The ISO algorithm to use for authentication.
     */
    void authenticateISO(unsigned char keyno,
                         DESFireISOAlgorithm algorithm = DF_ALG_2K3DES) override;

    /**
     * \brief AuthenticateAES command.
     * \param keyno The key number.
     */
    void authenticateAES(unsigned char keyno) override;
    void iks_authenticateAES(std::shared_ptr<DESFireKey> key, uint8_t keyno);

    /**
     * \brief Read data from a specific file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The data length to read
     * \param mode The communication mode
     * \return The bytes readed.
     */
    ByteVector readData(unsigned char fileno, unsigned int offset, unsigned int length,
                        EncryptionMode mode) override;

    /**
     * \brief Read record from a specific record file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The length to read
     * \param data The data buffer
     * \param mode The communication mode
     * \return The number of bytes read.
     */
    ByteVector readRecords(unsigned char fileno, unsigned int offset, unsigned int length,
                           EncryptionMode mode) override;

    /**
     * \brief Change file settings of a specific file in the current application.
     * \param fileno The file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param plain Communication is currently in plain data.
     */
    void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
                            const DESFireAccessRights &accessRights, bool plain) override;

    /**
     * \brief Change key settings for the current application.
     * \param settings The new key settings
     */
    void changeKeySettings(DESFireKeySettings settings) override;

    /**
     * \brief Change a key in the current application.
     * \param keyno The key number to change
     * \param key The new key
     */
    void changeKey(unsigned char keyno, std::shared_ptr<DESFireKey> key) override;

    /**
     * \brief Get the card version information.
     * \param dataVersion The card version information structure that will be filled
     */
    DESFireCardVersion getVersion() override;

    /**
     * \brief Get the current application list.
     * \return The application list.
     */
    std::vector<unsigned int> getApplicationIDs() override;

    /**
     * \brief Get the File IDentifiers of all active files within the currently selected
     * application
     * \return The file ID list.
     */
    ByteVector getFileIDs() override;

    /**
     * \brief Get value from a specific value file.
     * \param fileno The file number
     * \param mode The communication mode
     * \param value The value stored in the card
     */
    void getValue(unsigned char fileno, EncryptionMode mode,
                  unsigned int &value) override;

    /**
     * \brief ISO select application command.
     * \param isoaid The iso AID
     */
    void iso_selectApplication(
        ByteVector isoaid = ByteVector(DFEV1_DESFIRE_AID,
                                       DFEV1_DESFIRE_AID +
                                           sizeof(DFEV1_DESFIRE_AID))) override;

    /**
     * \brief Set the card configuration.
     * \param formatCardEnabled If true, the format card command is enabled.
     * \param randomIdEnabled If true, the CSN is random.
     */
    void setConfiguration(bool formatCardEnabled, bool randomIdEnabled) override;

    /**
     * \brief Set the card configuration default key.
     * param defaultKey The new default key.
     */
    void setConfiguration(std::shared_ptr<DESFireKey> defaultKey) override;

    /**
     * \brief Set a custom card ATS.
     * \param ats The new card ATS.
     */
    void setConfiguration(const ByteVector &ats) override;

    virtual void sam_iso_authenticate(std::shared_ptr<DESFireKey> key,
                                      DESFireISOAlgorithm algorithm, bool isMasterCardKey,
                                      unsigned char keyno);

    /**
     * Perform the authentication relying on Islog Key Server to perform
     * cryptographic operation.
     * This routine is suitable only for AES key: this is enforced by assertion.
     */
    virtual void iks_iso_authenticate(std::shared_ptr<DESFireKey> key,
                                      bool isMasterCardKey, uint8_t keyno);

    MyDivInfo extract_iks_div_info(std::shared_ptr<Key> key, uint8_t keyno);

    void selectApplication(unsigned int aid) override;

    /**
    * \brief Get the chip.
    * \return The chip.
    */
    std::shared_ptr<Chip> getChip() const override
    {
        return DESFireISO7816Commands::getChip();
    }

    /**
    * \brief Get the reader/card adapter.
    * \return The reader/card adapter.
    */
    std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const override
    {
        return DESFireISO7816Commands::getReaderCardAdapter();
    }

    std::shared_ptr<ISO7816Commands> getISO7816Commands() const override
    {
        auto command = std::make_shared<ISO7816ISO7816Commands>();
        command->setChip(getChip());
        command->setReaderCardAdapter(getReaderCardAdapter());
        return command;
    }

    /**
     * Retrieve the IKS signature corresponding the last PARTIAL read.
     * todo: Should retrieve something for the whole readData() block.
     *
     * But currently this work using the underlying DESFireCrypto and
     * therefore signature can only be retrieve for by block passed
     * to handleReadData().
     */
    SignatureResult IKS_getLastReadSignature() const override;

  protected:
    /**
     * \brief Generic method to send read file cmd.
     * \param cmd The command to send
     * \param data The command parameters
     * \param mode The communication mode
     * \return The data buffer.
     */
    virtual ByteVector handleReadCmd(unsigned char cmd, const ByteVector &data,
                                     EncryptionMode mode);

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

    /**
     * \brief Transmit a command.
     * \param cmd The command code.
     * \param buf The command buffer.
     * \param buflen The command buffer length.
     * \param forceLc Undocumented.
     * \return The response.
     */
    ByteVector transmit(unsigned char cmd, const ByteVector &buf = ByteVector(),
                        unsigned char lc = 0, bool forceLc = false) override;

    /**
     * \brief Transmit a command.
     * \param cmd The command code.
     * \param lc Undocumented.
     * \return The response.
     */
    virtual ByteVector transmit_plain(unsigned char cmd, unsigned char lc);

    /**
     * \brief Transmit a command.
     * \param cmd The command code.
     * \param buf The command buffer.
     * \param buflen The command buffer length.
     * \param forceLc Undocumented.
     * \return The response.
     */
    virtual ByteVector transmit_plain(unsigned char cmd,
                                      const ByteVector &buf = ByteVector(),
                                      unsigned char lc = 0, bool forceLc = false);

    /**
     * \brief Transmit a command.
     * \param cmd The command code.
     * \param lc Undocumented.
     * \return The response.
     */
    virtual ByteVector transmit_nomacv(unsigned char cmd, unsigned char lc);

    /**
     * \brief Transmit a command.
     * \param cmd The command code.
     * \param buf The command buffer.
     * \param buflen The command buffer length.
     * \param forceLc Undocumented.
     * \return The response.
     */
    virtual ByteVector transmit_nomacv(unsigned char cmd,
                                       const ByteVector &buf = ByteVector(),
                                       unsigned char lc = 0, bool forceLc = false);

    /**
     * Called after authentication was performed
     */
    void onAuthenticated();

  protected:
    SignatureResult handle_read_data_last_sig_;
};
}

#endif /* LOGICALACCESS_DESFIREEV1ISO7816COMMANDS_HPP */

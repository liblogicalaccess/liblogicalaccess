
/**
 * \file desfireev1stidstrcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 STidSTR commands.
 */

#ifndef LOGICALACCESS_DESFIREEV1STIDSTRCOMMANDS_HPP
#define LOGICALACCESS_DESFIREEV1STIDSTRCOMMANDS_HPP

#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1commands.hpp>
#include <logicalaccess/plugins/readers/stidstr/readercardadapters/stidstrreadercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_DESFIREEV1STIDSTR "DESFireEV1STidSTR"

/**
 * \brief The STid DESFire baudrates.
 */
typedef enum {
    STID_DF_BR_106KBPS = 0x00, /**< 106 kbps */
    STID_DF_BR_212KBPS = 0x01, /**< 212 kbps */
    STID_DF_BR_424KBPS = 0x02  /**< 424 kbps */
} STidDESFireBaudrate;

/**
 * \brief The STid DESFire baudrates.
 */
typedef enum {
    STID_DF_KEYLOC_RAM     = 0x00, /**< Key located in RAM memory */
    STID_DF_KEYLOC_EEPROM  = 0x01, /**< Key located in EEPROM memory */
    STID_DF_KEYLOC_INDEXED = 0x02  /**< Key indexed */
} STidKeyLocationType;

/**
 * \brief The DESFire EV1 base commands class for STidSTR reader.
 */
class LIBLOGICALACCESS_API DESFireEV1STidSTRCommands : public Commands
#ifndef SWIG
                                                       ,
                                                       public DESFireEV1Commands
#endif
{
  public:
    /**
     * \brief Constructor.
     */
    DESFireEV1STidSTRCommands();

    explicit DESFireEV1STidSTRCommands(std::string);


    /**
     * \brief Destructor.
     */
    virtual ~DESFireEV1STidSTRCommands();

    /**
     * \brief Scan the RFID field for DESFire tag.
     * \return The tag uid if present.
     */
    ByteVector scanDESFire() const;

    /**
     * \brief Release the RFID field.
     */
    void releaseRFIDField() const;

    /**
     * \brief Change the communication speed between the reader and the chip.
     * \param readerToChipKbps The reader to chip kbps to use.
     * \param chipToReaderKbps The chip to reader kbps to use.
     */
    void changePPS(STidDESFireBaudrate readerToChipKbps,
                   STidDESFireBaudrate chipToReaderKbps) const;

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

    /**
     * \brief Erase the card.
     */
    virtual void erase();
#ifndef SWIG
    using DESFireEV1Commands::createApplication;
#endif

    /**
     * \brief Create a new application.
     * \param aid The Application ID to create
     * \param settings Key settings
     * \param maxNbKeys Maximum number of keys
     */
    virtual void createApplication(unsigned int aid, DESFireKeySettings settings,
                                   unsigned char maxNbKeys);

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

    /**
     * \brief Delete an application.
     * \param aid The Application ID to delete
     */
    virtual void deleteApplication(unsigned int aid);

    /**
     * \brief Select an application.
     * \param aid The Application ID
     */
    void selectApplication(unsigned int aid) override;

    /**
     * \brief Get key settings on the current application.
     * \param settings Key settings
     * \param maxNbKeys Maximum number of keys
     */
    virtual void getKeySettings(DESFireKeySettings &settings, unsigned char &maxNbKeys);

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
    using DESFireEV1Commands::createStdDataFile;
#endif

    /**
     * \brief Create a new data file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size.
     */
    virtual void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                                   const DESFireAccessRights &accessRights,
                                   unsigned int fileSize);

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

    /**
     * \brief Commit the transaction.
     */
    virtual void commitTransaction();

    /**
     * \brief Abort the transaction.
     */
    virtual void abortTransaction();

    /**
     * \brief Create a new backup file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size.
     */
    virtual void createBackupFile(unsigned char fileno, EncryptionMode comSettings,
                                  const DESFireAccessRights &accessRights,
                                  unsigned int fileSize);

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

    /**
     * \brief Create a new value file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param lowerLimit The file size (in bytes)
     * \param upperLimit The file size (in bytes)
     * \param value The default value
     * \param limitedCreditEnabled Set if the limited credit is enabled
     */
    virtual void createValueFile(unsigned char fileno, EncryptionMode comSettings,
                                 const DESFireAccessRights &accessRights,
                                 unsigned int lowerLimit, unsigned int upperLimit,
                                 unsigned int value, bool limitedCreditEnabled);

    /**
     * \brief Create a new linear record file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size.
     * \param maxNumberOfRecords Max number of records in the file.
     */
    virtual void createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                        const DESFireAccessRights &accessRights,
                                        unsigned int fileSize,
                                        unsigned int maxNumberOfRecords);

    /**
     * \brief Create a new linear record file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size (in bytes).
     * \param maxNumberOfRecords Max number of records in the file.
     * \return True on success, false otherwise.
     */
    void createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                const DESFireAccessRights &accessRights,
                                unsigned int fileSize, unsigned int maxNumberOfRecords,
                                unsigned short isoFID) override;

    /**
     * \brief Create a new cyclic record file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size.
     * \param maxNumberOfRecords Max number of records in the file.
     */
    virtual void createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                        const DESFireAccessRights &accessRights,
                                        unsigned int fileSize,
                                        unsigned int maxNumberOfRecords);

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
     * \brief Load a key in the reader memory.
     * \param key The key object to load.
     */
    virtual void loadKey(std::shared_ptr<DESFireKey> key);

    /**
     * \brief Load a key in the reader memory.
     * \param key The key buffer to load.
     * \param diversify True to diversify the key, false otherwise.
     * \param isVolatile True to store it in RAM, false to store it in EEPROM.
     */
    virtual void loadKey(ByteVector key, bool diversify, bool isVolatile);

    /**
     * \brief Authenticate DESFire according to the key location.
     * \param keylocation The key location on reader.
     * \param keyno The key number, previously loaded.
     * \param cryptoMethod The cryptographic method.
     * \param keyindex The key index (if key location is indexed).
     */
    virtual void authenticate(STidKeyLocationType keylocation, unsigned char keyno,
                              DESFireKeyType cryptoMethod, unsigned char keyindex);

    /**
     * \brief Authenticate, given a key number.
     * \param keyno The key number.
     */
    virtual void authenticate(unsigned char keyno);

    /**
     * \brief Authenticate a block, given a key number.
     * \param keyno The key number.
     * \param key The key.
     */
    virtual void authenticate(unsigned char keyno, std::shared_ptr<DESFireKey> key);

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

    /**
     * \brief Read data from a specific file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The data length to read
     * \param mode The communication mode
     * \return The bytes readed.
     */
    virtual ByteVector readData(unsigned char fileno, unsigned int offset,
                                unsigned int length, EncryptionMode mode);

    /**
     * \brief Write data into a specific file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param data The data buffer
     * \param mode The communication mode
     */
    virtual void writeData(unsigned char fileno, unsigned int offset,
                           const ByteVector &data, EncryptionMode mode);

    /**
     * \brief Credit a specific value file.
     * \param fileno The file number
     * \param value The value to credit
     * \param mode The communication mode
     */
    virtual void credit(unsigned char fileno, unsigned int value, EncryptionMode mode);

    /**
     * \brief Debit a specific value file.
     * \param fileno The file number
     * \param value The value to debit
     * \param mode The communication mode
     */
    virtual void debit(unsigned char fileno, unsigned int value, EncryptionMode mode);

    /**
     * \brief Limit credit on a specific value file.
     * \param handle The SCardHandle
     * \param fileno The file number
     * \param value The value to limit credit
     * \param mode The communication mode
     * \return True on success, false otherwise.
     */
    virtual void limitedCredit(unsigned char fileno, unsigned int value,
                               EncryptionMode mode);

    /**
     * \brief Write record into a specific record file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The length to write
     * \param data The data buffer
     * \param mode The communication mode
     */
    virtual void writeRecord(unsigned char fileno, unsigned int offset,
                             const ByteVector &data, EncryptionMode mode);

    /**
     * \brief Read record from a specific record file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The length to read
     * \param data The data buffer
     * \param mode The communication mode
     * \return The number of bytes read.
     */
    virtual ByteVector readRecords(unsigned char fileno, unsigned int offset,
                                   unsigned int length, EncryptionMode mode);

    /**
     * \brief Clear a specific record file.
     * \param fileno The file number.
     */
    virtual void clearRecordFile(unsigned char fileno);

    /**
     * \brief Change file settings of a specific file in the current application.
     * \param fileno The file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param plain Communication is currently in plain data.
     */
    virtual void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
                                    const DESFireAccessRights &accessRights, bool plain);

    /**
     * \brief Delete a file in the current application.
     * \param fileno The file number to delete
     */
    virtual void deleteFile(unsigned char fileno);

    /**
     * \brief Change key settings for the current application.
     * \param settings The new key settings
     */
    virtual void changeKeySettings(DESFireKeySettings settings);

    /**
     * \brief Change a key in the current application.
     * \param keyno The key number to change
     * \param key The new key
     */
    virtual void changeKey(unsigned char keyno, std::shared_ptr<DESFireKey> key);

    /**
     * \brief Change the key indexed keyno by a new index or crypto method.
     * \param keyno The key number into the application.
     * \param cryptoMethod The cryptographic method.
     * \param keyversion The key version (AES only).
     * \param newkeyindex The new key index.
     * \param oldkeyindex The old key index.
     */
    void changeKeyIndex(unsigned char keyno, DESFireKeyType cryptoMethod,
                        unsigned char keyversion, unsigned char newkeyindex,
                        unsigned char oldkeyindex) const;

    /**
     * \brief Get the key version.
     * \param keyno The key number on the application.
     * \return The key version.
     */
    unsigned char getKeyVersion(unsigned char keyno) const;

    /**
     * \brief Get the card version information.
     * \param dataVersion The card version information structure that will be filled
     */
    virtual void getVersion(DESFireCommands::DESFireCardVersion &dataVersion);

    /**
     * \brief Get the current application list.
     * \return The application list.
     */
    virtual std::vector<unsigned int> getApplicationIDs();

    /**
     * \brief Get the File IDentifiers of all active files within the currently selected
     * application
     * \return The file ID list.
     */
    virtual ByteVector getFileIDs();

    /**
     * \brief Get settings of a specific file in the current application.
     * \param fileno The file number
     * \param fileSetting The file setting
     */
    virtual void getFileSettings(unsigned char fileno,
                                 DESFireCommands::FileSetting &fileSetting);

    /**
     * \brief Get value from a specific value file.
     * \param fileno The file number
     * \param mode The communication mode
     * \param value The value stored in the card
     */
    virtual void getValue(unsigned char fileno, EncryptionMode mode, unsigned int &value);

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

    /**
    * \brief Get the chip.
    * \return The chip.
    */
    std::shared_ptr<Chip> getChip() const override
    {
        return Commands::getChip();
    }

    /**
    * \brief Get the reader/card adapter.
    * \return The reader/card adapter.
    */
    std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const override
    {
        return Commands::getReaderCardAdapter();
    }


    /**
     * \brief Get the STidSTR reader/card adapter.
     * \return The STidSTR reader/card adapter.
     */
    std::shared_ptr<STidSTRReaderCardAdapter> getSTidSTRReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<STidSTRReaderCardAdapter>(
            getReaderCardAdapter());
    }

    /**
     * \brief Get the STid Key Location Type in string format.
     * \return The key location type in string.
     */
    static std::string STidKeyLocationTypeStr(STidKeyLocationType t);

  protected:
    unsigned int d_currentAid;
};
}

#endif /* LOGICALACCESS_DESFIREEV1STIDSTRCOMMANDS_H */
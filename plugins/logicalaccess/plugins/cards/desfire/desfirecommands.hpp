/**
 * \file desfirecommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire commands.
 */

#ifndef LOGICALACCESS_DESFIRECOMMANDS_HPP
#define LOGICALACCESS_DESFIRECOMMANDS_HPP

#include <logicalaccess/plugins/cards/desfire/desfireaccessinfo.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirelocation.hpp>
#include <logicalaccess/cards/commands.hpp>
#include <vector>
#include <logicalaccess/iks/RemoteCrypto.hpp>

namespace logicalaccess
{
#define EXCEPTION_MSG_SELECTAPPLICATION                                                  \
    "Select application failed." /**< \brief The select application exception message */

#ifndef DF_INS
#define DF_INS

#define DF_INS_AUTHENTICATE 0x0A
#define DF_INS_CHANGE_KEY_SETTINGS 0x54
#define DF_INS_GET_KEY_SETTINGS 0x45
#define DF_INS_CHANGE_KEY 0xC4
#define DF_INS_GET_KEY_VERSION 0x64

#define DF_INS_CREATE_APPLICATION 0xCA
#define DF_INS_DELETE_APPLICATION 0xDA
#define DF_INS_GET_APPLICATION_IDS 0x6A
#define DF_INS_SELECT_APPLICATION 0x5A
#define DF_INS_FORMAT_PICC 0xFC
#define DF_INS_GET_VERSION 0x60

#define DF_INS_GET_FILE_IDS 0x6F
#define DF_INS_GET_FILE_SETTINGS 0xF5
#define DF_INS_CHANGE_FILE_SETTINGS 0x5F
#define DF_INS_CREATE_STD_DATA_FILE 0xCD
#define DF_INS_CREATE_BACKUP_DATA_FILE 0xCB
#define DF_INS_CREATE_VALUE_FILE 0xCC
#define DF_INS_CREATE_LINEAR_RECORD_FILE 0xC1
#define DF_INS_CREATE_CYCLIC_RECORD_FILE 0xC0
#define DF_INS_DELETE_FILE 0xDF

#define DF_INS_READ_DATA 0xBD
#define DF_INS_WRITE_DATA 0x3D
#define DF_INS_GET_VALUE 0x6C
#define DF_INS_CREDIT 0x0C
#define DF_INS_DEBIT 0xDC
#define DF_INS_LIMITED_CREDIT 0x1C
#define DF_INS_WRITE_RECORD 0x3B
#define DF_INS_READ_RECORDS 0xBB
#define DF_INS_CLEAR_RECORD_FILE 0xEB
#define DF_COMMIT_TRANSACTION 0xC7
#define DF_INS_ABORT_TRANSACTION 0xA7

#define DF_INS_ADDITIONAL_FRAME 0xAF

#define DF_CLA_ISO_WRAP 0x90
#endif

#define CMD_DESFIRE "DESFire"

/**
 * \brief The key settings
 */
typedef enum {
    KS_ALLOW_CHANGE_MK                = 0x01,
    KS_FREE_LISTING_WITHOUT_MK        = 0x02,
    KS_FREE_CREATE_DELETE_WITHOUT_MK  = 0x04,
    KS_CONFIGURATION_CHANGEABLE       = 0x08,
    KS_CHANGE_KEY_WITH_MK             = 0x00,
    KS_CHANGE_KEY_WITH_TARGETED_KEYNO = 0xE0,
    KS_CHANGE_KEY_FROZEN              = 0xF0,
    KS_DEFAULT                        = 0x0B
} DESFireKeySettings;

/**
 * \brief The access key flag
 */
typedef enum {
    AR_KEY0  = 0x00,
    AR_KEY1  = 0x01,
    AR_KEY2  = 0x02,
    AR_KEY3  = 0x03,
    AR_KEY4  = 0x04,
    AR_KEY5  = 0x05,
    AR_KEY6  = 0x06,
    AR_KEY7  = 0x07,
    AR_KEY8  = 0x08,
    AR_KEY9  = 0x09,
    AR_KEY10 = 0x0A,
    AR_KEY11 = 0x0B,
    AR_KEY12 = 0x0C,
    AR_KEY13 = 0x0D,
    AR_FREE  = 0x0E,
    AR_NEVER = 0x0F
} TaskAccessRights;

/**
 * \brief Access rights on DESFire file.
 */
struct DESFireAccessRights
{
    TaskAccessRights readAccess;         /**< \brief Read access */
    TaskAccessRights writeAccess;        /**< \brief Write access */
    TaskAccessRights readAndWriteAccess; /**< \brief Read and Write access */
    TaskAccessRights changeAccess;       /**< \brief Change access */
};

/**
 * \brief Convert file access rights into flags.
 * \param rights The file access rights
 * \return The access rights flag.
 */
inline short AccessRightsInMemory(const DESFireAccessRights &rights)
{
    return static_cast<short>((rights.readAccess << 12) | (rights.writeAccess << 8) |
                              (rights.readAndWriteAccess << 4) | rights.changeAccess);
}

class DESFireChip;

/**
 * \brief The DESFire commands class.
 */
class LIBLOGICALACCESS_API DESFireCommands : public Commands
{
  public:
    /**
    * \brief Constructor.
    */
    DESFireCommands()
        : Commands(CMD_DESFIRE)
    {
    }

    /**
    * \brief Constructor.
    * \param ct The cmd type.
    */
    explicit DESFireCommands(std::string cmdtype)
        : Commands(cmdtype)
    {
    }

    virtual ~DESFireCommands() = default;

    /**
     * \brief Erase the card.
     */
    virtual void erase() = 0;

    /**
     * \brief Describe setting for Data File type.
     */
    struct LIBLOGICALACCESS_API DataFileSetting
    {
        unsigned char fileSize[3]; /**< \brief The file size */
    };

    /**
     * \brief Describe setting for Value File type.
     */
    struct LIBLOGICALACCESS_API ValueFileSetting
    {
        uint32_t lowerLimit;         /**< \brief The lower value limit */
        uint32_t upperLimit;         /**< \brief The upper value limit */
        uint32_t limitedCreditValue; /**< \brief The limited credit value */
        bool limitedCreditEnabled;   /**< \brief True if limited credit is enabled, false
                                        otherwise */
    };

    /**
     * \brief Describe setting for Record File type.
     */
    struct LIBLOGICALACCESS_API RecordFileSetting
    {
        unsigned char recordSize[3];       /**< \brief The record size */
        unsigned char maxNumberRecords[3]; /**< \brief The maximum number of records */
        unsigned char
            currentNumberRecords[3]; /**< \brief The current number of records */
    };

    /**
     * \brief Describe file setting.
     */
    struct LIBLOGICALACCESS_API FileSetting
    {
        unsigned char fileType;        /**< \brief The file type */
        unsigned char comSett;         /**< \brief The communication setting */
        unsigned char accessRights[2]; /**< \brief The file access rights */
        union {
            DataFileSetting dataFile;
            ValueFileSetting valueFile;
            RecordFileSetting recordFile;
        } type; /**< \brief The file type specific information */
        DataFileSetting getDataFile() const;
        void setDataFile(const DataFileSetting &);
        ValueFileSetting getValueFile() const;
        void setValueFile(const ValueFileSetting &);
        RecordFileSetting getRecordFile() const;
        void setRecordFile(const RecordFileSetting &);
    };

    /**
     * \brief Card information about software and hardware version.
     */
    struct LIBLOGICALACCESS_API DESFireCardVersion
    {
        unsigned char hardwareVendor;      /**< \brief The hardware vendor */
        unsigned char hardwareType;        /**< \brief The hardware type */
        unsigned char hardwareSubType;     /**< \brief The hardware subtype */
        unsigned char hardwareMjVersion;   /**< \brief The hardware major version */
        unsigned char hardwareMnVersion;   /**< \brief The hardware minor version */
        unsigned char hardwareStorageSize; /**< \brief The hardware storage size */
        unsigned char hardwareProtocol;    /**< \brief The hardware protocol */

        unsigned char softwareVendor;      /**< \brief The software vendor */
        unsigned char softwareType;        /**< \brief The software type */
        unsigned char softwareSubType;     /**< \brief The software subtype */
        unsigned char softwareMjVersion;   /**< \brief The software major version */
        unsigned char softwareMnVersion;   /**< \brief The software minor version */
        unsigned char softwareStorageSize; /**< \brief The software storage size */
        unsigned char softwareProtocol;    /**< \brief The software protocol */

        unsigned char uid[7];     /**< \brief The serial card number */
        unsigned char batchNo[5]; /**< \brief The batch number */
        unsigned char cwProd;     /**< \brief The production id */
        unsigned char yearProd;   /**< \brief The production year */
    };

    /**
     * \brief Select an application.
     * \param location The DESFire location
     */
    virtual void selectApplication(std::shared_ptr<DESFireLocation> location);

    /**
     * \brief Create a new application.
     * \param location The DESFire location
     * \param settings Key settings
     * \param maxNbKeys Maximum number of keys
     */
    virtual void createApplication(std::shared_ptr<DESFireLocation> location,
                                   DESFireKeySettings settings, unsigned char maxNbKeys);

    /**
     * \brief Create a new data file in the current application.
     * \param location The DESFire location
     * \param accessRights The file access rights
     * \param fileSize The file size.
     */
    virtual void createStdDataFile(std::shared_ptr<DESFireLocation> location,
                                   const DESFireAccessRights &accessRights,
                                   unsigned int fileSize);

    /**
     * \brief Get the communication mode for a file.
     * \param fileno The file number
     * \param isReadMode Is read or write mode
     * \param needLoadKey Set if it's necessary to be authenticate for the access.
     * \return The communication mode.
     */
    virtual EncryptionMode getEncryptionMode(unsigned char fileno, bool isReadMode,
                                             bool *needLoadKey = nullptr);

    virtual EncryptionMode getEncryptionMode(const FileSetting &fileSetting,
                                             bool isReadMode,
                                             bool *needLoadKey = nullptr);

    /**
     * \brief Get the length of a file.
     * \param fileno The file number
     * \return The length.
     */
    virtual unsigned int getFileLength(unsigned char fileno);

    /**
     * \brief Select an application.
     * \param aid The Application ID
     */
    virtual void selectApplication(unsigned int aid) = 0;

    /**
     * \brief Create a new application.
     * \param aid The Application ID to create
     * \param settings Key settings
     * \param maxNbKeys Maximum number of keys
     * \return True on success, false otherwise.
     */
    virtual void createApplication(unsigned int aid, DESFireKeySettings settings,
                                   unsigned char maxNbKeys) = 0;

    /**
     * \brief Delete an application.
     * \param aid The Application ID to delete
     */
    virtual void deleteApplication(unsigned int aid) = 0;

    /**
     * \brief Get the current application list.
     * \return The application list.
     */
    virtual std::vector<unsigned int> getApplicationIDs() = 0;

    /**
     * \brief Change a key in the current application.
     * \param keyno The key number to change
     * \param key The new key
     */
    virtual void changeKey(unsigned char keyno, std::shared_ptr<DESFireKey> key) = 0;

    /**
     * \brief Get key settings on the current application.
     * \param settings Key settings
     * \param maxNbKeys Maximum number of keys
     */
    virtual void getKeySettings(DESFireKeySettings &settings,
                                unsigned char &maxNbKeys) = 0;

    /**
     * \brief Change key settings for the current application.
     * \param settings The new key settings
     * \return
     */
    virtual void changeKeySettings(DESFireKeySettings settings) = 0;

    /**
     * \brief Get the File IDentifiers of all active files within the currently selected
     * application
     * \return The file ID list.
     */
    virtual ByteVector getFileIDs() = 0;

    /**
     * \brief Get settings of a specific file in the current application.
     * \param fileno The file number
     * \param fileSetting The file setting
     */
    virtual FileSetting getFileSettings(unsigned char fileno) = 0;

    /**
     * \brief Change file settings of a specific file in the current application.
     * \param fileno The file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param plain Communication is currently in plain data.
     */
    virtual void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
                                    const DESFireAccessRights &accessRights,
                                    bool plain) = 0;

    /**
     * \brief Create a new data file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size.
     */
    virtual void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                                   const DESFireAccessRights &accessRights,
                                   unsigned int fileSize) = 0;

    /**
     * \brief Create a new backup file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size.
     */
    virtual void createBackupFile(unsigned char fileno, EncryptionMode comSettings,
                                  const DESFireAccessRights &accessRights,
                                  unsigned int fileSize) = 0;

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
                                 unsigned int value, bool limitedCreditEnabled) = 0;

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
                                        unsigned int maxNumberOfRecords) = 0;

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
                                        unsigned int maxNumberOfRecords) = 0;

    /**
     * \brief Delete a file in the current application.
     * \param fileno The file number to delete
     */
    virtual void deleteFile(unsigned char fileno) = 0;

    /**
     * \brief Read data from a specific file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The data length to read
     * \param mode The communication mode
     * \return The bytes readed.
     */
    virtual ByteVector readData(unsigned char fileno, unsigned int offset,
                                unsigned int length, EncryptionMode mode) = 0;

    /**
     * \brief Write data into a specific file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param data The data buffer
     * \param mode The communication mode
     */
    virtual void writeData(unsigned char fileno, unsigned int offset,
                           const ByteVector &data, EncryptionMode mode) = 0;

    /**
     * \brief Get value from a specific value file.
     * \param fileno The file number
     * \param mode The communicatio mode
     * \param value The value stored in the card
     */
    virtual void getValue(unsigned char fileno, EncryptionMode mode,
                          unsigned int &value) = 0;

    /**
     * \brief Credit a specific value file.
     * \param fileno The file number
     * \param value The value to credit
     * \param mode The communication mode
     */
    virtual void credit(unsigned char fileno, unsigned int value,
                        EncryptionMode mode) = 0;

    /**
     * \brief Debit a specific value file.
     * \param fileno The file number
     * \param value The value to debit
     * \param mode The communication mode
     */
    virtual void debit(unsigned char fileno, unsigned int value, EncryptionMode mode) = 0;

    /**
     * \brief Limit credit on a specific value file.
     * \param fileno The file number
     * \param value The value to limit credit
     * \param mode The communication mode
     */
    virtual void limitedCredit(unsigned char fileno, unsigned int value,
                               EncryptionMode mode) = 0;

    /**
     * \brief Write record into a specific record file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param data The data buffer
     * \param mode The communication mode
     */
    virtual void writeRecord(unsigned char fileno, unsigned int offset,
                             const ByteVector &data, EncryptionMode mode) = 0;

    /**
     * \brief Read record from a specific record file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The length to read
     * \param mode The communication mode
     * \return The number of bytes read.
     */
    virtual ByteVector readRecords(unsigned char fileno, unsigned int offset,
                                   unsigned int length, EncryptionMode mode) = 0;

    /**
     * \brief Clear a specific record file.
     * \param fileno The file number.
     */
    virtual void clearRecordFile(unsigned char fileno) = 0;

    /**
     * \brief Commit the transaction.
     */
    virtual void commitTransaction() = 0;

    /**
     * \brief Abort the transaction.
     */
    virtual void abortTransaction() = 0;

    /**
     * \brief Authenticate a block, given a key number.
     * \param keyno The key number, previously loaded on the profile.
     */
    virtual void authenticate(unsigned char keyno = 0) = 0;

    /**
     * \brief Authenticate a block, given a key number.
     * \param keyno The key number.
     * \param key The key.
     */
    virtual void authenticate(unsigned char keyno, std::shared_ptr<DESFireKey> key) = 0;

    /**
     * \brief Get the card version information.
     * \param dataVersion The card version information structure that will be filled
     */
    virtual DESFireCardVersion getVersion() = 0;

    /**
     * Get the IKS signature corresponding to the last readData() call.
     *
     * This will return the empty SignatureResult and will only work if both:
     *    1) DESFireEV1ISO7816 is used.
     *    2) An IKS backend is used.
     */
    virtual SignatureResult IKS_getLastReadSignature() const
    {
        return SignatureResult{};
    }

  protected:
    std::shared_ptr<DESFireChip> getDESFireChip() const;
};
}

#endif
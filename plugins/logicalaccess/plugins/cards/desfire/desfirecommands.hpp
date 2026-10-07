/**
 * \file desfirecommands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire commands.
 */

#ifndef LOGICALACCESS_DESFIRECOMMANDS_HPP
#define LOGICALACCESS_DESFIRECOMMANDS_HPP

#include <logicalaccess/plugins/cards/desfire/desfireaccessinfo.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirelocation.hpp>
#include <logicalaccess/cards/commands.hpp>

#include <cstdint>
#include <string>
#include <vector>

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

#endif // DF_INS

#define CMD_DESFIRE "DESFire"

/**
 * \brief The key settings
 */
typedef enum {
    KS_ALLOW_CHANGE_MK                = 0x01,
    KS_FREE_LISTING_WITHOUT_MK        = 0x02,
    KS_FREE_CREATE_DELETE_WITHOUT_MK  = 0x04,
    KS_CONFIGURATION_CHANGEABLE       = 0x08,
    KS_ENABLE_NXP_DAM_KEYS            = 0x10,
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
 * 
 * Each access right occupies four bits in the two-byte DESFire access-rights field.
 */
struct DESFireAccessRights
{
    TaskAccessRights readAccess;         /**< \brief Read access. */
    TaskAccessRights writeAccess;        /**< \brief Write access. */
    TaskAccessRights readAndWriteAccess; /**< \brief Read and Write access. */
    TaskAccessRights changeAccess;       /**< \brief Change access. */
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
class LLA_CARDS_DESFIRE_API DESFireCommands : public Commands
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

    virtual ~DESFireCommands();

    /**
     * \brief Erase the card.
     */
    virtual void erase() = 0;

    /**
     * \brief Maximum number of additional access-right sets supported by the GetFileSettings response.
     */
    static const unsigned char MaxAdditionalAccessRights = 7U;

    /**
     * \brief Describe the additional access-right sets of a file.
     *
     * The DESFire protocol limits the number of additional access-right sets to seven.
     * A fixed-size array is therefore intentionally used instead of dynamic allocation.
     */
    struct LLA_CARDS_DESFIRE_API AdditionalAccessRights
    {
        unsigned char nrAddARs;
        DESFireAccessRights accessRights[MaxAdditionalAccessRights];
    };

    /**
     * \brief Describe Secure Dynamic Messaging settings.
     * 
     * A has* flag is intentionally kept separately from the associated
     * three-byte value because an absent optional field and a present field
     * whose value is zero are distinct protocol states.
     */
    struct LLA_CARDS_DESFIRE_API SDMSettings
    {
        unsigned char options;                 /**< \brief SDM options. */
        unsigned char accessRights[2];         /**< \brief SDM access rights. */

        unsigned char vcuidOffset[3];          /**< \brief VCUID mirror offset. */
        unsigned char sdmReadCounterOffset[3]; /**< \brief SDM read-counter mirror offset. */
        unsigned char piccDataOffset[3];       /**< \brief Encrypted PICCData mirror offset. */
        unsigned char gpioStatusOffset[3];     /**< \brief GPIOStatus mirror offset. */
        unsigned char sdmMacInputOffset[3];    /**< \brief SDM MAC input offset. */
        unsigned char sdmEncOffset[3];         /**< \brief SDM encrypted-data offset. */
        unsigned char sdmEncLength[3];         /**< \brief SDM encrypted-data length. */
        unsigned char sdmMacOffset[3];         /**< \brief SDM MAC mirror offset. */
        unsigned char sdmReadCounterLimit[3];  /**< \brief SDM read-counter limit. */

        bool hasVcuidOffset;          /**< \brief Indicates whether vcuidOffset is present. */
        bool hasSdmReadCounterOffset; /**< \brief Indicates whether sdmReadCounterOffset is present. */
        bool hasPiccDataOffset;       /**< \brief Indicates whether piccDataOffset is present. */
        bool hasGpioStatusOffset;     /**< \brief Indicates whether gpioStatusOffset is present. */
        bool hasSdmMacInputOffset;    /**< \brief Indicates whether sdmMacInputOffset is present. */
        bool hasSdmEncOffset;         /**< \brief Indicates whether sdmEncOffset is present. */
        bool hasSdmEncLength;         /**< \brief Indicates whether sdmEncLength is present. */
        bool hasSdmMacOffset;         /**< \brief Indicates whether sdmMacOffset is present. */
        bool hasSdmReadCounterLimit;  /**< \brief Indicates whether sdmReadCounterLimit is present. */
    };

    /**
     * \brief Describe DUOX CRL settings.
     *
     * These settings are only applicable to a DUOX CRL file.
     */
    struct LLA_CARDS_DESFIRE_API CRLSettings
    {
        unsigned char options;
        unsigned char csnSize;
        unsigned char signatureKey;

        bool hasSignatureKey; /**< \brief Indicates whether the CRL signature-key field was present. */
    };

    /**
     * \brief Describe settings for a Standard Data or Backup Data File type.
     * 
     * The structure contains the common data-file properties as well as
     * optional Secure Dynamic Messaging and DUOX CRL information.
     */
    struct LLA_CARDS_DESFIRE_API DataFileSetting
    {
        unsigned char fileSize[3]; /**< \brief The file size */

        unsigned char nrAddARs; /**< \brief Number of additional access-right sets. */
        DESFireAccessRights additionalAccessRights[MaxAdditionalAccessRights];  /**< \brief Additional access-right sets. */

        bool hasSDM; /**< \brief Indicates whether Secure Dynamic Messaging settings are present. */
        SDMSettings sdm; /**< \brief Secure Dynamic Messaging settings. */

        bool hasCRL; /**< Indicates whether DUOX CRL settings are present. */
        CRLSettings crl; /**< \brief DUOX CRL settings. */
    };

    /**
     * \brief Describe setting for Value File type.
     */
    struct LLA_CARDS_DESFIRE_API ValueFileSetting
    {
        std::int32_t lowerLimit;    /**< \brief Lower value limit. */
        std::int32_t upperLimit;    /**< \brief Upper value limit. */
        std::int32_t limitedCreditValue; /**< \brief Limited credit value. */

        bool limitedCreditEnabled;  /**< \brief Indicates whether Limited Credit is enabled. */
        bool freeGetValue; /**< \brief Indicates whether GetValue is available without access-right restrictions. */

        unsigned char nrAddARs; /**< \brief Number of additional access-right sets. */
        DESFireAccessRights additionalAccessRights[MaxAdditionalAccessRights]; /**< \brief Additional access-right sets. */
    };

    /**
     * \brief Describe settings for a Linear Record or Cyclic Record file.
     */
    struct LLA_CARDS_DESFIRE_API RecordFileSetting
    {
        unsigned char recordSize[3];           /**< \brief Record size. */
        unsigned char maxNumberRecords[3];     /**< \brief Maximum number of records. */
        unsigned char currentNumberRecords[3]; /**< \brief Current number of records. */
        
        unsigned char nrAddARs; /**< \brief Number of additional access-right sets. */
        DESFireAccessRights additionalAccessRights[MaxAdditionalAccessRights]; /**< \brief Additional access-right sets. */
    };

    /**
     * \brief Describe setting for Transaction MAC File type.
     *
     * This structure describes the Transaction MAC file configuration returned
     * by GetFileSettings for a Transaction MAC file.
     * It does not represent the data stored in the Transaction MAC file itself (TMC/TMV/TSV).
     */
    struct LLA_CARDS_DESFIRE_API TransactionMacFileSetting
    {
        /** \brief Options associated with AppTransactionMACKey. */
        unsigned char tmKeyOption;
       /** \brief Version of AppTransactionMACKey.
        * 
        * This field is optional for DUOX and is present when an AppTransactionMACKey is configured.
        * The presence flag is required because a present key version of zero is distinct from an absent field.
        */
        unsigned char tmKeyVersion;

         /** \brief Indicates whether tmKeyVersion was present in the response. */
        bool hasTmKeyVersion;

        /** \brief Indicates whether Transaction Signature is configured. */
        bool transactionSignature;
        /** \brief Key number of KeyID.AppTransactionSIGKey.
         * This field is present when Transaction Signature is configured.
         */
        unsigned char transactionSignatureKeyNo;
        /** \brief Indicates whether transactionSignatureKeyNo was present in the response. */
        bool hasTransactionSignatureKeyNo;

        /**
         * \brief DUOX Transaction MAC Information exclusion file map.
         *
         * Valid when the corresponding DUOX option indicates that the exclusion file map is configured.
         */
        std::uint32_t tmiExclusionFileMap;
        /** \brief Indicates whether tmiExclusionFileMap was present in the response. */
        bool hasTmiExclusionFileMap;

        /**
         * \brief DUOX Transaction MAC limit.
         *
         * Valid when the corresponding DUOX option indicates that Transaction MAC limiting is configured.
         */
        std::uint32_t tmcLimit;
        /** \brief Indicates whether tmcLimit was present in the response. */
        bool hasTmcLimit;
    };

    /**
     * \brief Describe file setting.
     *
     * The first fields contain information common to every DESFire file.
     * The type union contains the settings specific to the reported file type.
     *
     * The union is intentionally retained for compatibility with the
     * existing LLA DESFire API and to avoid unnecessary dynamic allocation.
     */
    struct LLA_CARDS_DESFIRE_API FileSetting
    {
        unsigned char fileType;        /**< \brief File type. */
        unsigned char comSett;         /**< \brief Communication setting. */
        unsigned char accessRights[2]; /**< \brief File access rights. */
        union {
            DataFileSetting dataFile;
            ValueFileSetting valueFile;
            RecordFileSetting recordFile;
            TransactionMacFileSetting transactionMacFile;
        } type; /**< \brief The file type specific information. */
        DataFileSetting getDataFile() const;
        void setDataFile(const DataFileSetting &);
        ValueFileSetting getValueFile() const;
        void setValueFile(const ValueFileSetting &);
        RecordFileSetting getRecordFile() const;
        void setRecordFile(const RecordFileSetting &);
        TransactionMacFileSetting getTransactionMacFile() const;
        void setTransactionMacFile(const TransactionMacFileSetting &);
    };

    /**
     * \brief GetVersion option used by cards supporting extended manufacturing data.
     */
    enum class DESFireGetVersionOption : unsigned char
    {
        None  = 0x00,
        FabID = 0x01
    };

    /**
     * \brief Card information about a DESFire hardware or software version.
     */
    struct LLA_CARDS_DESFIRE_API DESFireVersionInfo
    {
        unsigned char vendor;       /**< \brief The vendor identifier */
        unsigned char type;         /**< \brief The product type */
        unsigned char subtype;      /**< \brief The product subtype */
        unsigned char majorVersion; /**< \brief The major version */
        unsigned char minorVersion; /**< \brief The minor version */
        unsigned char storageSize;  /**< \brief The storage size information */
        unsigned char protocol;     /**< \brief The protocol information */
    };

    /**
     * \brief Card manufacturing information returned by GetVersion.
     *
     * The variable-size fields are represented by ByteVector because the
     * GetVersion response may contain different UID sizes and manufacturing
     * data layouts depending on the card variant.
     */
    struct LLA_CARDS_DESFIRE_API DESFireManufacturingInfo
    {
        ByteVector uid;         /**< \brief The serial card number */
        ByteVector batchNo;     /**< \brief The batch number */
        ByteVector typeId;      /**< \brief The card type identifier */
        unsigned char cwProd;   /**< \brief The production card/vendor information */
        unsigned char yearProd; /**< \brief The production year */
    };

    /**
     * \brief Card information about software, hardware and manufacturing data.
     *
     * This structure is the common GetVersion representation for DESFire card variants.
     * It intentionally uses dynamically sized manufacturing fields
     * because newer card variants may return UID and manufacturing data with different sizes.
     *
     * The hasFabID flag indicates whether fabID is present in the response.
     * fabID is meaningful only when hasFabID is true.
     */
    struct LLA_CARDS_DESFIRE_API DESFireCardVersion
    {
        DESFireVersionInfo hardware;            /**< \brief The hardware version information */
        DESFireVersionInfo software;            /**< \brief The software version information */
        DESFireManufacturingInfo manufacturing; /**< \brief The manufacturing information */

        bool hasFabID;       /**< \brief Indicates whether a FabID was returned */
        unsigned char fabID; /**< \brief The factory identifier */
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
     * \return The file setting
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
                                 int32_t lowerLimit, int32_t upperLimit,
                                 int32_t value, bool limitedCreditEnabled) = 0;

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
     * \return The value stored in the card
     */
    virtual int32_t getValue(unsigned char fileno, EncryptionMode mode) = 0;

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
    virtual void limitedCredit(unsigned char fileno, int32_t value,
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
     * \return The card version information structure that will be filled
     */
    virtual DESFireCardVersion getVersion() = 0;

  protected:
    std::shared_ptr<DESFireChip> getDESFireChip() const;
};
}

#endif
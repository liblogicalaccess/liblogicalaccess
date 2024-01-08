/**
 * \file desfireev1commands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire EV1 commands.
 */

#ifndef LOGICALACCESS_DESFIREEV1COMMANDS_HPP
#define LOGICALACCESS_DESFIREEV1COMMANDS_HPP

#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1location.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816commands.hpp>

namespace logicalaccess
{
#ifndef DFEV1_INS
#define DFEV1_INS

#define DFEV1_DESFIRE_AID "\xd2\x76\x00\x00\x85\x01\x00"

#define DFEV1_INS_FREE_MEM 0x6E
#define DFEV1_INS_GET_DF_NAMES 0x6D
#define DFEV1_INS_GET_CARD_UID 0x51
#define DFEV1_INS_GET_ISO_FILE_IDS 0x61
#define DFEV1_INS_SET_CONFIGURATION 0x5C

#define DFEV1_INS_AUTHENTICATE_ISO 0x1A
#define DFEV1_INS_AUTHENTICATE_AES 0xAA

#define SELECT_FILE_BY_AID 0x04
#define SELECT_FILE_BY_FID 0x00
#endif

#define CMD_DESFIRE_EV1 "DESFireEV1"

/**
 * \brief The ISO DFName and FID
 */
struct DFName
{
    unsigned int AID;
    unsigned short FID;
    char DF_Name[64];
};

/**
 * \brief The FID Support for ISO 7816.
 */
typedef enum { FIDS_NO_ISO_FID = 0x00, FIDS_ISO_FID = 0x20 } FidSupport;

/**
 * \brief The DESFire EV1 transmission mode.
 */
typedef enum {
    DFEV1_CMD_PLAIN  = 0x00,
    DFEV1_CMD_NOMACV = 0x01,
    DFEV1_CMD_MACV   = 0x02
} DESFireEV1TransmissionMode;

/**
 * \brief The DESFire EV1 algorithm.
 */
typedef enum {
    DF_ALG_BY_CONTEXT = 0x00,
    DF_ALG_2K3DES     = 0x02,
    DF_ALG_3K3DES     = 0x04,
    DF_ALG_AES        = 0x09
} DESFireISOAlgorithm;

class DESFireEV1Chip;

/**
 * \brief The DESFire EV1 base commands class.
 */
class LLA_CARDS_DESFIRE_API DESFireEV1Commands : public ICommands
{
  public:
    DESFireEV1Commands() = default;

    /**
     * \brief Select an application.
     * \param location The DESFire location
     */
    virtual void selectApplication(std::shared_ptr<DESFireEV1Location> location);

    /**
     * \brief Create a new application.
     * \param location The DESFire location
     * \param settings Key settings
     * \param maxNbKeys Maximum number of keys
     */
    virtual void createApplication(std::shared_ptr<DESFireEV1Location> location,
                                   DESFireKeySettings settings, unsigned char maxNbKeys);

    /**
     * \brief Create a new data file in the current application.
     * \param location The DESFire location
     * \param accessRights The file access rights
     * \param fileSize The file size.
     */
    virtual void createStdDataFile(std::shared_ptr<DESFireEV1Location> location,
                                   const DESFireAccessRights &accessRights,
                                   unsigned int fileSize);

    /**
     * \brief Get the value of available bytes.
     * \return The available memory in bytes.
     */
    virtual unsigned int getFreeMem() = 0;

    /**
     * \brief Get the ISO DF-Names of all active application.
     * \return The DF-Names list.
     */
    virtual std::vector<DFName> getDFNames() = 0;

    /**
     * \brief Get the ISO FID of all active files within the currently selected
     * application.
     * \return The ISO FID list.
     */
    virtual std::vector<unsigned short> getISOFileIDs() = 0;

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
     */
    virtual void createApplication(unsigned int aid, DESFireKeySettings settings,
                                   unsigned char maxNbKeys, DESFireKeyType cryptoMethod,
                                   FidSupport fidSupported = FIDS_NO_ISO_FID,
                                   unsigned short isoFID   = 0x00,
                                   ByteVector isoDFName = ByteVector()) = 0;

    /**
     * \brief Get key settings on the current application.
     * \param settings Key settings
     * \param maxNbKeys Maximum number of keys
     * \param keyType The key type
     */
    virtual void getKeySettings(DESFireKeySettings &settings, unsigned char &maxNbKeys,
                                DESFireKeyType &keyType) = 0;

    /**
     * \brief Get a random card UID.
     * \return The card UID.
     */
    virtual ByteVector getCardUID() = 0;

    /**
     * \brief Create a new data file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size (in bytes).
     */
    virtual void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                                   const DESFireAccessRights &accessRights,
                                   unsigned int fileSize,
                                   unsigned short isoFID = 0x00) = 0;

    /**
     * \brief Create a new backup file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size (in bytes).
     */
    virtual void createBackupFile(unsigned char fileno, EncryptionMode comSettings,
                                  const DESFireAccessRights &accessRights,
                                  unsigned int fileSize,
                                  unsigned short isoFID = 0x00) = 0;

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
                                        unsigned short isoFID = 0x00) = 0;

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
                                        unsigned short isoFID = 0x00) = 0;

    /**
     * \brief AuthenticateISO command.
     * \param keyno The key number.
     * \param algorithm The ISO algorithm to use for authentication.
     */
    virtual void authenticateISO(unsigned char keyno,
                                 DESFireISOAlgorithm algorithm = DF_ALG_2K3DES) = 0;

    /**
     * \brief AuthenticateAES command.
     * \param keyno The key number.
     */
    virtual void authenticateAES(unsigned char keyno) = 0;

    /**
     * \brief Read data from a specific file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The data length to read
     * \param data The data buffer
     * \param mode The communication mode
     * \return The number of bytes read.
     */
    // virtual size_t readData(int fileno, size_t offset, size_t length, void* data,
    // EncryptionMode mode) = 0;

    /**
     * \brief Read record from a specific record file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The length to read
     * \param data The data buffer
     * \param mode The communication mode
     * \return The number of bytes read.
     */
    // virtual size_t readRecords(int fileno, size_t offset, size_t length, void* data,
    // EncryptionMode mode) = 0;

    /**
     * \brief Change file settings of a specific file in the current application.
     * \param fileno The file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param plain Communication is currently in plain data.
     * \return True on success, false otherwise.
     */
    // virtual bool changeFileSettings(int fileno, EncryptionMode comSettings,
    // DESFireAccessRights accessRights, bool plain) = 0;

    /**
     * \brief Change key settings for the current application.
     * \param settings The new key settings
     * \return True on success, false otherwise.
     */
    // virtual bool changeKeySettings(DESFireKeySettings settings) = 0;

    /**
     * \brief Change a key in the current application.
     * \param keyno The key number to change
     * \param key The new key
     * \return True on success, false otherwise.
     */
    // virtual bool changeKey(unsigned char keyno, std::shared_ptr<DESFireKey> key) = 0;

    /**
     * \brief Get the card version information.
     * \param dataVersion The card version information structure that will be filled
     * \return True on success, false otherwise.
     */
    // virtual bool getVersion(DESFireCommands::DESFireCardVersion& dataVersion) = 0;

    /**
     * \brief Get the current application list.
     * \return The application list.
     */
    // virtual std::vector<int> getApplicationIDs() = 0;

    /**
     * \brief Get the File IDentifiers of all active files within the currently selected
     * application
     * \return The file ID list.
     */
    // virtual std::vector<int> getFileIDs() = 0;

    /**
     * \brief Get value from a specific value file.
     * \param fileno The file number
     * \param mode The communication mode
     * \param value The value stored in the card
     * \return True on success, false otherwise.
     */
    // virtual bool getValue(int fileno, EncryptionMode mode, int& value) = 0;

    /**
     * \brief Set the card configuration.
     * \param formatCardEnabled If true, the format card command is enabled.
     * \param randomIdEnabled If true, the CSN is random (can't be changed once
     * activated).
     */
    virtual void setConfiguration(bool formatCardEnabled, bool randomIdEnabled) = 0;

    /**
     * \brief Set the card configuration default key.
     * param defaultKey The new default key.
     */
    virtual void setConfiguration(std::shared_ptr<DESFireKey> defaultKey) = 0;

    /**
     * \brief Set a custom card ATS.
     * \param ats The new card ATS.
     */
    virtual void setConfiguration(const ByteVector &ats) = 0;

    virtual std::shared_ptr<ISO7816Commands> getISO7816Commands() const
    {
        return std::shared_ptr<ISO7816Commands>();
    }

  protected:
    std::shared_ptr<DESFireEV1Chip> getDESFireEV1Chip() const;
};
}

#endif /* LOGICALACCESS_DESFIREEV1CARDPROVIDER_HPP */
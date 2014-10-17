/**
 * \file desfireev1iso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 commands.
 */

#ifndef LOGICALACCESS_DESFIREEV1ISO7816COMMANDS_HPP
#define LOGICALACCESS_DESFIREEV1ISO7816COMMANDS_HPP

#include "desfireiso7816commands.hpp"
#include "desfireev1commands.hpp"
#include "iso7816iso7816commands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The DESFire EV1 base commands class.
     */
    class LIBLOGICALACCESS_API DESFireEV1ISO7816Commands : public DESFireISO7816Commands, public DESFireEV1Commands, public ISO7816ISO7816Commands
    {
    public:

        /**
         * \brief Constructor.
         */
        DESFireEV1ISO7816Commands();

        /**
         * \brief Destructor.
         */
        virtual ~DESFireEV1ISO7816Commands();

        /**
         * \brief Get the value of available bytes.
         * \return The available memory in bytes.
         */
        virtual unsigned int getFreeMem();

        /**
         * \brief Get the ISO DF-Names of all active application.
         * \return The DF-Names list.
         */
        virtual vector<DFName> getDFNames();

        /**
         * \brief Get the ISO FID of all active files within the currently selected application.
         * \return The ISO FID list.
         */
        virtual vector<unsigned short> getISOFileIDs();

        /**
         * \brief Create a new application.
         * \param aid The Application ID to create
         * \param settings Key settings
         * \param maxNbKeys Maximum number of keys
         */
        virtual void createApplication(unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys, DESFireKeyType cryptoMethod, FidSupport fidSupported = FIDS_NO_ISO_FID, unsigned short isoFID = 0x00, std::vector<unsigned char> isoDFName = std::vector<unsigned char>());

        /**
         * \brief Get key settings on the current application.
         * \param settings Key settings
         * \param maxNbKeys Maximum number of keys
         * \param keyType The key type
         */
        virtual void getKeySettings(DESFireKeySettings& settings, unsigned char& maxNbKeys, DESFireKeyType& keyType);

        /**
         * \brief Get a random card UID.
         * \return The card UID.
         */
        virtual std::vector<unsigned char> getCardUID();

        /**
         * \brief Create a new data file in the current application.
         * \param fileno The new file number
         * \param comSettings The file communication setting
         * \param accessRights The file access rights
         * \param fileSize The file size (in bytes).
         */
        virtual void createStdDataFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned short isoFID);

        /**
         * \brief Create a new backup file in the current application.
         * \param fileno The new file number
         * \param comSettings The file communication setting
         * \param accessRights The file access rights
         * \param fileSize The file size (in bytes).
         */
        virtual void createBackupFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned short isoFID);

        /**
         * \brief Create a new linear record file in the current application.
         * \param fileno The new file number
         * \param comSettings The file communication setting
         * \param accessRights The file access rights
         * \param fileSize The file size (in bytes).
         * \param maxNumberOfRecords Max number of records in the file.
         */
        virtual void createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords, unsigned short isoFID);

        /**
         * \brief Create a new cyclic record file in the current application.
         * \param fileno The new file number
         * \param comSettings The file communication setting
         * \param accessRights The file access rights
         * \param fileSize The file size (in bytes).
         * \param maxNumberOfRecords Max number of records in the file.
         */
        virtual void createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords, unsigned short isoFID);

        /**
         * \brief Select file under current DF.
         * \param fid The FID
         */
        virtual void iso_selectFile(unsigned short fid = 0);

        /**
         * \brief Read records.
         * \param fid The FID
         * \param start_record The start record (0 = read last written record)
         * \param record_number The number of records to read
         * \return The record(s) data
         */
        virtual std::vector<unsigned char> iso_readRecords(unsigned short fid = 0, unsigned char start_record = 0, DESFireRecords record_number = DF_RECORD_ONERECORD);

        /**
         * \brief Append a record to a file.
         * \param data The record data
         * \param fid The FID
         */
        virtual void iso_appendrecord(const std::vector<unsigned char>& data = std::vector<unsigned char>(), unsigned short fid = 0);

        /**
         * \brief Get the ISO challenge for authentication.
         * \param length The challenge length (8 = 2K3DES, 16 = 3K3DES and AES)
         * \return The ISO challenge.
         */
        virtual std::vector<unsigned char> iso_getChallenge(unsigned int length = 8);

        /**
         * \brief ISO external authenticate.
         * \param algorithm The ISO algorithm to use for authentication.
         * \param isMasterCardKey True if the key to authenticate is the master card key, false otherwise.
         * \param keyno The key number.
         * \param data The data.
         */
        virtual void iso_externalAuthenticate(DESFireISOAlgorithm algorithm = DF_ALG_BY_CONTEXT, bool isMasterCardKey = true, unsigned char keyno = 0x00, const std::vector<unsigned char>& data = std::vector<unsigned char>());

        /**
         * \brief ISO internal authenticate.
         * \param algorithm The ISO algorithm to use for authentication.
         * \param isMasterCardKey True if the key to authenticate is the master card key, false otherwise.
         * \param keyno The key number.
         * \param RPCD2 The RPCD2.
         * \param length The length.
         * \return The cryptogram.
         */
        virtual std::vector<unsigned char> iso_internalAuthenticate(DESFireISOAlgorithm algorithm = DF_ALG_BY_CONTEXT, bool isMasterCardKey = true, unsigned char keyno = 0x00, const std::vector<unsigned char>& RPCD2 = std::vector<unsigned char>(), unsigned int length = 16);

        /**
         * \brief ISO authenticate.
         * \param key The key.
         * \param algorithm The ISO algorithm to use for authentication.
         * \param isMasterCardKey True if the key to authenticate is the master card key, false otherwise.
         * \param keyno The key number.
         */
        virtual void iso_authenticate(std::shared_ptr<DESFireKey> key, DESFireISOAlgorithm algorithm = DF_ALG_2K3DES, bool isMasterCardKey = true, unsigned char keyno = 0x00);

        /**
         * \brief Authenticate, given a key number.
         * \param keyno The key number.
         * \param key The key.
         */
        virtual void authenticate(unsigned char keyno, std::shared_ptr<DESFireKey> key);

        /**
         * \brief AuthenticateISO command.
         * \param keyno The key number.
         * \param algorithm The ISO algorithm to use for authentication.
         */
        virtual void authenticateISO(unsigned char keyno, DESFireISOAlgorithm algorithm = DF_ALG_2K3DES);

        /**
         * \brief AuthenticateAES command.
         * \param keyno The key number.
         */
        virtual void authenticateAES(unsigned char keyno);

        /**
         * \brief Read data from a specific file.
         * \param fileno The file number
         * \param offset The byte offset
         * \param length The data length to read
         * \param data The data buffer
         * \param mode The communication mode
         * \return The number of bytes read.
         */
        virtual unsigned int readData(unsigned char fileno, unsigned int offset, unsigned int length, void* data, EncryptionMode mode);

        /**
         * \brief Read record from a specific record file.
         * \param fileno The file number
         * \param offset The byte offset
         * \param length The length to read
         * \param data The data buffer
         * \param mode The communication mode
         * \return The number of bytes read.
         */
        virtual unsigned int readRecords(unsigned char fileno, unsigned int offset, unsigned int length, void* data, EncryptionMode mode);

        /**
         * \brief Change file settings of a specific file in the current application.
         * \param fileno The file number
         * \param comSettings The file communication setting
         * \param accessRights The file access rights
         * \param plain Communication is currently in plain data.
         */
        virtual void changeFileSettings(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain);

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
         * \brief Get the card version information.
         * \param dataVersion The card version information structure that will be filled
         */
        virtual void getVersion(DESFireCommands::DESFireCardVersion& dataVersion);

        /**
         * \brief Get the current application list.
         * \return The application list.
         */
        virtual std::vector<unsigned int> getApplicationIDs();

        /**
         * \brief Get the File IDentifiers of all active files within the currently selected application
         * \return The file ID list.
         */
        virtual std::vector<unsigned char> getFileIDs();

        /**
         * \brief Get value from a specific value file.
         * \param fileno The file number
         * \param mode The communication mode
         * \param value The value stored in the card
         */
        virtual void getValue(unsigned char fileno, EncryptionMode mode, unsigned int& value);

        /**
         * \brief ISO select application command.
         * \param isoaid The iso AID
         */
        virtual void iso_selectApplication(std::vector<unsigned char> isoaid = std::vector<unsigned char>(DFEV1_DESFIRE_AID, DFEV1_DESFIRE_AID + sizeof(DFEV1_DESFIRE_AID)));

        /**
         * \brief Set the card configuration.
         * \param formatCardEnabled If true, the format card command is enabled.
         * \param randomIdEnabled If true, the CSN is random.
         */
        virtual void setConfiguration(bool formatCardEnabled, bool randomIdEnabled);

        /**
         * \brief Set the card configuration default key.
         * param defaultKey The new default key.
         */
        virtual void setConfiguration(std::shared_ptr<DESFireKey> defaultKey);

        /**
         * \brief Set a custom card ATS.
         * \param ats The new card ATS.
         */
        virtual void setConfiguration(const std::vector<unsigned char>& ats);

        virtual void sam_iso_authenticate(std::shared_ptr<DESFireKey> key, DESFireISOAlgorithm algorithm, bool isMasterCardKey, unsigned char keyno);

    protected:

        /**
         * \brief Generic method to read data from a file.
         * \param err The last error code
         * \param firstMsg The first message to send
         * \param length The length to read
         * \param mode The communication mode
         * \return The data buffer.
         */
        virtual std::vector<unsigned char> handleReadData(unsigned char err, const std::vector<unsigned char>& firstMsg, unsigned int length, EncryptionMode mode);

        /**
         * \brief Generic method to write data into a file.
         * \param cmd The command to send
         * \param parameters The command parameters
         * \param paramLength The parameters length
         * \param data The data buffer
         * \param mode The communication mode
         */
        virtual void handleWriteData(unsigned char cmd, unsigned char* parameters, unsigned int paramLength, const std::vector<unsigned char>& data, EncryptionMode mode);

        /**
         * \brief Transmit a command.
         * \param cmd The command code.
         * \param buf The command buffer.
         * \param buflen The command buffer length.
         * \param forceLc Undocumented.
         * \return The response.
         */
        virtual std::vector<unsigned char> transmit(unsigned char cmd, const std::vector<unsigned char>& buf = std::vector<unsigned char>(), unsigned char lc = 0, bool forceLc = false);

        /**
         * \brief Transmit a command.
         * \param cmd The command code.
         * \param lc Undocumented.
         * \return The response.
         */
        virtual std::vector<unsigned char> transmit_plain(unsigned char cmd, unsigned char lc);

        /**
         * \brief Transmit a command.
         * \param cmd The command code.
         * \param buf The command buffer.
         * \param buflen The command buffer length.
         * \param forceLc Undocumented.
         * \return The response.
         */
        virtual std::vector<unsigned char> transmit_plain(unsigned char cmd, const std::vector<unsigned char>& buf = std::vector<unsigned char>(), unsigned char lc = 0, bool forceLc = false);

        /**
         * \brief Transmit a command.
         * \param cmd The command code.
         * \param lc Undocumented.
         * \return The response.
         */
        virtual std::vector<unsigned char> transmit_nomacv(unsigned char cmd, unsigned char lc);

        /**
         * \brief Transmit a command.
         * \param cmd The command code.
         * \param buf The command buffer.
         * \param buflen The command buffer length.
         * \param forceLc Undocumented.
         * \return The response.
         */
        virtual std::vector<unsigned char> transmit_nomacv(unsigned char cmd, const std::vector<unsigned char>& buf = std::vector<unsigned char>(), unsigned char lc = 0, bool forceLc = false);

    protected:
    };
}

#endif /* LOGICALACCESS_DESFIREEV1ISO7816COMMANDS_HPP */
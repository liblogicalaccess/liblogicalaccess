/**
 * \file desfirerwk400commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Desfire Rwk400 Commands
 */

#ifndef LOGICALACCESS_DESFIRERWK400COMMANDS_HPP
#define LOGICALACCESS_DESFIRERWK400COMMANDS_HPP

#include "desfirecommands.hpp"
#include "desfirecrypto.hpp"
#include "../readercardadapters/rwk400readercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	/**
	 * \brief A DESFire Rwk400 commands base class.
	 */
	class LIBLOGICALACCESS_API DesfireRwk400Commands : public DESFireCommands
	{
	public:
			/**
			 * \brief Constructor.
			 */
			DesfireRwk400Commands();

			/**
			 * \brief Destructor.
			 */
			virtual ~DesfireRwk400Commands();

			/**
			 * \brief Get the internal DESFire crypto.
			 * \return The internal DESFire crypto.
			 */
			DESFireCrypto& getCrypto();

			/**
			 * \brief Erase the card.
			 * \return true if the card was erased, false otherwise. If false, the card may be partially erased.
			 */
			virtual bool erase(bool resetKey = true);

			/**
			 * \brief Select an application.
			 * \param aid The Application ID
			 * \return true on success, false otherwise.
			 */
			virtual bool selectApplication(int aid);

			/**
			 * \brief Create a new application.
			 * \param aid The Application ID to create
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 * \return True on success, false otherwise.
			 */
			virtual bool createApplication(int aid, DESFireKeySettings settings, int maxNbKeys);

			/**
			 * \brief Delete an application.
			 * \param aid The Application ID to delete
			 * \return True on success, false otherwise.
			 */
			virtual bool deleteApplication(int aid);

			/**
			 * \brief Get the current application list.
			 * \return The application list.
			 */
			virtual std::vector<int> getApplicationIDs();

			/**
			 * \brief Change a key in the current application.
			 * \param keyno The key number to change
			 * \param key The new key
			 * \return True on success, false otherwise.
			 */
			virtual bool changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key);

			/**
			 * \brief Get key settings on the current application.
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 */
			virtual void getKeySettings(DESFireKeySettings& settings, unsigned int& maxNbKeys);

			/**
			 * \brief Change key settings for the current application.
			 * \param settings The new key settings
			 * \return 
			 */
			virtual bool changeKeySettings(DESFireKeySettings settings);

			/**
			 * \brief Get the File IDentifiers of all active files within the currently selected application
			 * \return The file ID list.
			 */
			virtual std::vector<int> getFileIDs();

			/**
			 * \brief Get settings of a specific file in the current application.
			 * \param fileno The file number
			 * \param fileSetting The file setting
			 */
			virtual bool getFileSettings(int fileno, FileSetting& fileSetting);

			/**
			 * \brief Change file settings of a specific file in the current application.
			 * \param fileno The file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param plain Communication is currently in plain data.
			 * \return True on success, false otherwise.
			 */
			virtual bool changeFileSettings(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain);

			/**
			 * \brief Create a new data file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 * \return True on success, false otherwise.
			 */
			virtual bool createStdDataFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize);

			/**
			 * \brief Create a new backup file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 * \return True on success, false otherwise.
			 */
			virtual bool createBackupFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize);

			/**
			 * \brief Create a new value file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param lowerLimit The file size (in bytes)
			 * \param upperLimit The file size (in bytes)
			 * \param value The default value
			 * \param limitedCreditEnabled Set if the limited credit is enabled
			 * \return True on success, false otherwise.
			 */
			virtual bool createValueFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int lowerLimit, unsigned int upperLimit, int value, bool limitedCreditEnabled);

			/**
			 * \brief Create a new linear record file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 * \param maxNumberOfRecords Max number of records in the file.
			 * \return True on success, false otherwise.
			 */
			virtual bool createLinearRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords);

			/**
			 * \brief Create a new cyclic record file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 * \param maxNumberOfRecords Max number of records in the file.
			 * \return True on success, false otherwise.
			 */
			virtual bool createCyclicRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords);

			/**
			 * \brief Delete a file in the current application.
			 * \param fileno The file number to delete
			 * \return True on success, false otherwise.
			 */
			virtual bool deleteFile(int fileno);

			/**
			 * \brief Read data from a specific file.
			 * \param fileno The file number
			 * \param offset The byte offset
			 * \param length The data length to read
			 * \param data The data buffer
			 * \param mode The communication mode
			 * \return The number of bytes read.
			 */
			virtual size_t readData(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode);

			/**
			 * \brief Write data into a specific file.
			 * \param fileno The file number
			 * \param offset The byte offset
			 * \param length The data length to read
			 * \param data The data buffer
			 * \param mode The communication mode
			 * \return True on success, false otherwise.
			 */
			virtual bool writeData(int fileno, size_t offset, size_t length, const void* data, EncryptionMode mode);

			/**
			 * \brief Get value from a specific value file.
			 * \param handle The SCardHandle
			 * \param fileno The file number
			 * \param mode The communicatio mode
			 * \param value The value stored in the card
			 * \return True on success, false otherwise.
			 */
			virtual bool getValue(int fileno, EncryptionMode mode, int& value);

			/**
			 * \brief Credit a specific value file.
			 * \param fileno The file number
			 * \param value The value to credit
			 * \param mode The communication mode
			 * \return True on success, false otherwise.
			 */
			virtual bool credit(int fileno, int value, EncryptionMode mode);

			/**
			 * \brief Debit a specific value file.
			 * \param fileno The file number
			 * \param value The value to debit
			 * \param mode The communication mode
			 * \return True on success, false otherwise.
			 */
			virtual bool debit(int fileno, int value, EncryptionMode mode);

			/**
			 * \brief Limit credit on a specific value file.
			 * \param handle The SCardHandle
			 * \param fileno The file number
			 * \param value The value to limit credit
			 * \param mode The communication mode
			 * \return True on success, false otherwise.
			 */
			virtual bool limitedCredit(int fileno, int value, EncryptionMode mode);

			/**
			 * \brief Write record into a specific record file.
			 * \param fileno The file number
			 * \param offset The byte offset
			 * \param length The length to write
			 * \param data The data buffer
			 * \param mode The communication mode
			 * \return True on success, false otherwise.
			 */
			virtual bool writeRecord(int fileno, size_t offset, size_t length, const void* data, EncryptionMode mode);

			/**
			 * \brief Read record from a specific record file.
			 * \param fileno The file number
			 * \param offset The byte offset
			 * \param length The length to read
			 * \param data The data buffer
			 * \param mode The communication mode
			 * \return The number of bytes read.
			 */
			virtual size_t readRecords(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode);

			/**
			 * \brief Clear a specific record file.
			 * \param fileno The file number.
			 * \return True on success, false otherwise.
			 */
			virtual bool clearRecordFile(int fileno);

			/**
			 * \brief Commit the transaction.
			 * \return True on success, false otherwise.
			 */
			virtual bool commitTransaction();

			/**
			 * \brief Abort the transaction.
			 * \return True on success, false otherwise.
			 */
			virtual bool abortTransaction();

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param keyno The key number, previously loaded with DESFire::loadKey().
			 * \return true if authenticated, false otherwise.
			 */
			virtual bool authenticate(unsigned char keyno = 0);

			/**
			 * \brief Get the card version information.
			 * \param dataVersion The card version information structure that will be filled
			 * \return True on success, false otherwise.
			 */
			virtual bool getVersion(DESFireCardVersion& dataVersion);

		protected:

			/**
			 * \brief Crypto instance for security manipulation.
			 */
			DESFireCrypto d_crypto;
	};	
}

#endif /* LOGICALACCESS_DESFIRERWK400COMMANDS_HPP */


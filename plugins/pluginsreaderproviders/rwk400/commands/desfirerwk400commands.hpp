/**
 * \file desfirerwk400commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Desfire Rwk400 Commands
 */

#ifndef LOGICALACCESS_DESFIRERWK400COMMANDS_HPP
#define LOGICALACCESS_DESFIRERWK400COMMANDS_HPP

#include "desfirecommands.hpp"
#include "desfirecrypto.hpp"
#include "desfirechip.hpp"
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
			 */
			virtual void erase();

			/**
			 * \brief Select an application.
			 * \param aid The Application ID
			 * \return true on success, false otherwise.
			 */
			virtual void selectApplication(unsigned int aid);

			/**
			 * \brief Create a new application.
			 * \param aid The Application ID to create
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 */
			virtual void createApplication(unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys);

			/**
			 * \brief Delete an application.
			 * \param aid The Application ID to delete
			 */
			virtual void deleteApplication(unsigned int aid);

			/**
			 * \brief Get the current application list.
			 * \return The application list.
			 */
			virtual std::vector<unsigned int> getApplicationIDs();

			/**
			 * \brief Change a key in the current application.
			 * \param keyno The key number to change
			 * \param key The new key
			 */
			virtual void changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key);

			/**
			 * \brief Get key settings on the current application.
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 */
			virtual void getKeySettings(DESFireKeySettings& settings, unsigned char& maxNbKeys);

			/**
			 * \brief Change key settings for the current application.
			 * \param settings The new key settings
			 * \return 
			 */
			virtual void changeKeySettings(DESFireKeySettings settings);

			/**
			 * \brief Get the File IDentifiers of all active files within the currently selected application
			 * \return The file ID list.
			 */
			virtual std::vector<unsigned char> getFileIDs();

			/**
			 * \brief Get settings of a specific file in the current application.
			 * \param fileno The file number
			 * \param fileSetting The file setting
			 */
			virtual void getFileSettings(unsigned char fileno, FileSetting& fileSetting);

			/**
			 * \brief Change file settings of a specific file in the current application.
			 * \param fileno The file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param plain Communication is currently in plain data.
			 */
			virtual void changeFileSettings(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain);

			/**
			 * \brief Create a new data file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 */
			virtual void createStdDataFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize);

			/**
			 * \brief Create a new backup file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 */
			virtual void createBackupFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize);

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
			virtual void createValueFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int lowerLimit, unsigned int upperLimit, unsigned int value, bool limitedCreditEnabled);

			/**
			 * \brief Create a new linear record file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 * \param maxNumberOfRecords Max number of records in the file.
			 */
			virtual void createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords);

			/**
			 * \brief Create a new cyclic record file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 * \param maxNumberOfRecords Max number of records in the file.
			 */
			virtual void createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords);

			/**
			 * \brief Delete a file in the current application.
			 * \param fileno The file number to delete
			 */
			virtual void deleteFile(unsigned char fileno);

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
			 * \brief Write data into a specific file.
			 * \param fileno The file number
			 * \param offset The byte offset
			 * \param length The data length to read
			 * \param data The data buffer
			 * \param mode The communication mode
			 */
			virtual void writeData(unsigned char fileno, unsigned int offset, unsigned int length, const void* data, EncryptionMode mode);

			/**
			 * \brief Get value from a specific value file.
			 * \param handle The SCardHandle
			 * \param fileno The file number
			 * \param mode The communicatio mode
			 * \param value The value stored in the card
			 */
			virtual void getValue(unsigned char fileno, EncryptionMode mode, unsigned int& value);

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
			 */
			virtual void limitedCredit(unsigned char fileno, unsigned int value, EncryptionMode mode);

			/**
			 * \brief Write record into a specific record file.
			 * \param fileno The file number
			 * \param offset The byte offset
			 * \param length The length to write
			 * \param data The data buffer
			 * \param mode The communication mode
			 */
			virtual void writeRecord(unsigned char fileno, unsigned int offset, unsigned int length, const void* data, EncryptionMode mode);

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
			 * \brief Clear a specific record file.
			 * \param fileno The file number.
			 */
			virtual void clearRecordFile(unsigned char fileno);

			/**
			 * \brief Commit the transaction.
			 */
			virtual void commitTransaction();

			/**
			 * \brief Abort the transaction.
			 */
			virtual void abortTransaction();

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param keyno The key number, previously loaded with DESFire::loadKey().
			 */
			virtual void authenticate(unsigned char keyno = 0);

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param keyno The key number.
			 * \param key The key.
			 */
			virtual void authenticate(unsigned char keyno, boost::shared_ptr<DESFireKey> key);

			/**
			 * \brief Get the card version information.
			 * \param dataVersion The card version information structure that will be filled
			 */
			virtual void getVersion(DESFireCardVersion& dataVersion);

			std::vector<unsigned char> premutationKey(const unsigned char * key, size_t keylength);

			std::vector<unsigned char> computeChecksum (std::vector<unsigned char> command, std::vector<unsigned char> permutedKey);

			void setChip(boost::shared_ptr<DESFireChip> chip);


		protected:

			/**
			 * \brief Crypto instance for security manipulation.
			 */
			DESFireCrypto d_crypto;

			boost::shared_ptr<DESFireChip> d_chip;
	};	
}

#endif /* LOGICALACCESS_DESFIRERWK400COMMANDS_HPP */


/**
 * \file DESFireCommands.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire commands.
 */

#ifndef LOGICALACCESS_DESFIRECOMMANDS_H
#define LOGICALACCESS_DESFIRECOMMANDS_H

#include "DESFireAccessInfo.h"
#include "DESFireLocation.h"
#include "logicalaccess/Cards/Commands.h"


namespace logicalaccess
{

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

	/**
	 * \brief The key settings
	 */
	typedef enum 
	{
		KS_ALLOW_CHANGE_MK = 0x01,
		KS_FREE_LISTING_WITHOUT_MK = 0x02,
		KS_FREE_CREATE_DELETE_WITHOUT_MK = 0x04,
		KS_CONFIGURATION_CHANGEABLE = 0x08,
		KS_CHANGE_KEY_WITH_MK = 0x00,
		KS_CHANGE_KEY_WITH_TARGETED_KEYNO = 0xE0,
		KS_CHANGE_KEY_FROZEN = 0xF0,
		KS_DEFAULT = 0x0B
	}DESFireKeySettings;

	/**
	 * \brief The access key flag
	 */
	typedef enum
	{
		AR_KEY0 = 0x00,
		AR_KEY1 = 0x01,
		AR_KEY2 = 0x02,
		AR_KEY3 = 0x03,
		AR_KEY4 = 0x04,
		AR_KEY5 = 0x05,
		AR_KEY6 = 0x06,
		AR_KEY7 = 0x07,
		AR_KEY8 = 0x08,
		AR_KEY9 = 0x09,
		AR_KEY10 = 0x0A,
		AR_KEY11 = 0x0B,
		AR_KEY12 = 0x0C,
		AR_KEY13 = 0x0D,
		AR_FREE = 0x0E,
		AR_NEVER = 0x0F
	}TaskAccessRights;

	/**
	 * \brief Access rights on DESFire file.
	 */
	struct DESFireAccessRights
	{
		TaskAccessRights  readAccess; /**< \brief Read access */
		TaskAccessRights  writeAccess; /**< \brief Write access */
		TaskAccessRights  readAndWriteAccess; /**< \brief Read and Write access */
		TaskAccessRights  changeAccess; /**< \brief Change access */
	};

	/**
	 * \brief Convert file access rights into flags.
	 * \param rights The file access rights
	 * \return The access rights flag.
	 */
	inline short AccessRightsInMemory(DESFireAccessRights rights)
	{
		return static_cast<short>(
			(rights.readAccess << 12) |
			(rights.writeAccess << 8) |
			(rights.readAndWriteAccess << 4) |
			rights.changeAccess
		);
	}

	/**
	 * \brief The DESFire commands class.
	 */
	class LIBLOGICALACCESS_API DESFireCommands : public virtual Commands
	{
		public:

			/**
			 * \brief Erase the card.
			 * \return true if the card was erased, false otherwise. If false, the card may be partially erased.
			 */
			virtual bool erase(bool resetKey = true) = 0;

			/**
			 * \brief Describe setting for Data File type.
			 */
			struct DataFileSetting
			{
				unsigned char		fileSize[3]; /**< \brief The file size */
			};

			/**
			 * \brief Describe setting for Value File type.
			 */
			struct ValueFileSetting
			{
				uint32_t	lowerLimit; /**< \brief The lower value limit */
				uint32_t	upperLimit; /**< \brief The upper value limit */
				uint32_t	limitedCreditValue; /**< \brief The limited credit value */
				bool		limitedCreditEnabled; /**< \brief True if limited credit is enabled, false otherwise */
			};

			/**
			 * \brief Describe setting for Record File type.
			 */
			struct RecordFileSetting
			{
				unsigned char		recordSize[3]; /**< \brief The record size */
				unsigned char		maxNumberRecords[3]; /**< \brief The maximum number of records */
				unsigned char		currentNumberRecords[3]; /**< \brief The current number of records */
			};

			/**
			 * \brief Describe file setting.
			 */
			struct FileSetting
			{				
				unsigned char		fileType; /**< \brief The file type */
				unsigned char		comSett; /**< \brief The communication setting */
				unsigned char		accessRights[2]; /**< \brief The file access rights */
				union
				{
					DataFileSetting		dataFile;
					ValueFileSetting	valueFile;
					RecordFileSetting	recordFile;
				} type; /**< \brief The file type specific information */
			};

			/**
			 * \brief Card information about software and hardware version.
			 */
			struct DESFireCardVersion
			{
				unsigned char hardwareVendor; /**< \brief The hardware vendor */	
				unsigned char hardwareType; /**< \brief The hardware type */
				unsigned char hardwareSubType; /**< \brief The hardware subtype */
				unsigned char hardwareMjVersion; /**< \brief The hardware major version */
				unsigned char hardwareMnVersion;  /**< \brief The hardware minor version */
				unsigned char hardwareStorageSize;  /**< \brief The hardware storage size */
				unsigned char hardwareProtocol; /**< \brief The hardware protocol */

				unsigned char softwareVendor; /**< \brief The software vendor */
				unsigned char softwareType; /**< \brief The software type */
				unsigned char softwareSubType; /**< \brief The software subtype */
				unsigned char softwareMjVersion; /**< \brief The software major version */
				unsigned char softwareMnVersion; /**< \brief The software minor version */
				unsigned char softwareStorageSize; /**< \brief The software storage size */
				unsigned char softwareProtocol; /**< \brief The software protocol */

				unsigned char uid[7]; /**< \brief The serial card number */
				unsigned char batchNo[5]; /**< \brief The batch number */
				unsigned char cwProd; /**< \brief The production id */
				unsigned char yearProd; /**< \brief The production year */
			};

			/**
			 * \brief Select an application.
			 * \param aid The Application ID
			 * \return true on success, false otherwise.
			 */
			virtual bool selectApplication(int aid) = 0;

			/**
			 * \brief Create a new application.
			 * \param aid The Application ID to create
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 * \return True on success, false otherwise.
			 */
			virtual bool createApplication(int aid, DESFireKeySettings settings, int maxNbKeys) = 0;

			/**
			 * \brief Delete an application.
			 * \param aid The Application ID to delete
			 * \return True on success, false otherwise.
			 */
			virtual bool deleteApplication(int aid) = 0;

			/**
			 * \brief Get the current application list.
			 * \return The application list.
			 */
			virtual std::vector<int> getApplicationIDs() = 0;

			/**
			 * \brief Change a key in the current application.
			 * \param keyno The key number to change
			 * \param key The new key
			 * \return True on success, false otherwise.
			 */
			virtual bool changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key) = 0;

			/**
			 * \brief Get key settings on the current application.
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 */
			virtual void getKeySettings(DESFireKeySettings& settings, unsigned int& maxNbKeys) = 0;

			/**
			 * \brief Change key settings for the current application.
			 * \param settings The new key settings
			 * \return 
			 */
			virtual bool changeKeySettings(DESFireKeySettings settings) = 0;

			/**
			 * \brief Get the File IDentifiers of all active files within the currently selected application
			 * \return The file ID list.
			 */
			virtual std::vector<int> getFileIDs() = 0;

			/**
			 * \brief Get settings of a specific file in the current application.
			 * \param fileno The file number
			 * \param fileSetting The file setting
			 */
			virtual bool getFileSettings(int fileno, FileSetting& fileSetting) = 0;

			/**
			 * \brief Change file settings of a specific file in the current application.
			 * \param fileno The file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param plain Communication is currently in plain data.
			 * \return True on success, false otherwise.
			 */
			virtual bool changeFileSettings(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain) = 0;

			/**
			 * \brief Create a new data file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 * \return True on success, false otherwise.
			 */
			virtual bool createStdDataFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize) = 0;

			/**
			 * \brief Create a new backup file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 * \return True on success, false otherwise.
			 */
			virtual bool createBackupFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize) = 0;

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
			virtual bool createValueFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int lowerLimit, unsigned int upperLimit, int value, bool limitedCreditEnabled) = 0;

			/**
			 * \brief Create a new linear record file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 * \param maxNumberOfRecords Max number of records in the file.
			 * \return True on success, false otherwise.
			 */
			virtual bool createLinearRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords) = 0;

			/**
			 * \brief Create a new cyclic record file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 * \param maxNumberOfRecords Max number of records in the file.
			 * \return True on success, false otherwise.
			 */
			virtual bool createCyclicRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords) = 0;

			/**
			 * \brief Delete a file in the current application.
			 * \param fileno The file number to delete
			 * \return True on success, false otherwise.
			 */
			virtual bool deleteFile(int fileno) = 0;

			/**
			 * \brief Read data from a specific file.
			 * \param fileno The file number
			 * \param offset The byte offset
			 * \param length The data length to read
			 * \param data The data buffer
			 * \param mode The communication mode
			 * \return The number of bytes read.
			 */
			virtual size_t readData(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode) = 0;

			/**
			 * \brief Write data into a specific file.
			 * \param fileno The file number
			 * \param offset The byte offset
			 * \param length The data length to read
			 * \param data The data buffer
			 * \param mode The communication mode
			 * \return True on success, false otherwise.
			 */
			virtual bool writeData(int fileno, size_t offset, size_t length, const void* data, EncryptionMode mode) = 0;

			/**
			 * \brief Get value from a specific value file.
			 * \param handle The SCardHandle
			 * \param fileno The file number
			 * \param mode The communicatio mode
			 * \param value The value stored in the card
			 * \return True on success, false otherwise.
			 */
			virtual bool getValue(int fileno, EncryptionMode mode, int& value) = 0;

			/**
			 * \brief Credit a specific value file.
			 * \param fileno The file number
			 * \param value The value to credit
			 * \param mode The communication mode
			 * \return True on success, false otherwise.
			 */
			virtual bool credit(int fileno, int value, EncryptionMode mode) = 0;

			/**
			 * \brief Debit a specific value file.
			 * \param fileno The file number
			 * \param value The value to debit
			 * \param mode The communication mode
			 * \return True on success, false otherwise.
			 */
			virtual bool debit(int fileno, int value, EncryptionMode mode) = 0;

			/**
			 * \brief Limit credit on a specific value file.
			 * \param handle The SCardHandle
			 * \param fileno The file number
			 * \param value The value to limit credit
			 * \param mode The communication mode
			 * \return True on success, false otherwise.
			 */
			virtual bool limitedCredit(int fileno, int value, EncryptionMode mode) = 0;

			/**
			 * \brief Write record into a specific record file.
			 * \param fileno The file number
			 * \param offset The byte offset
			 * \param length The length to write
			 * \param data The data buffer
			 * \param mode The communication mode
			 * \return True on success, false otherwise.
			 */
			virtual bool writeRecord(int fileno, size_t offset, size_t length, const void* data, EncryptionMode mode) = 0;

			/**
			 * \brief Read record from a specific record file.
			 * \param fileno The file number
			 * \param offset The byte offset
			 * \param length The length to read
			 * \param data The data buffer
			 * \param mode The communication mode
			 * \return The number of bytes read.
			 */
			virtual size_t readRecords(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode) = 0;

			/**
			 * \brief Clear a specific record file.
			 * \param fileno The file number.
			 * \return True on success, false otherwise.
			 */
			virtual bool clearRecordFile(int fileno) = 0;

			/**
			 * \brief Commit the transaction.
			 * \return True on success, false otherwise.
			 */
			virtual bool commitTransaction() = 0;

			/**
			 * \brief Abort the transaction.
			 * \return True on success, false otherwise.
			 */
			virtual bool abortTransaction() = 0;

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param keyno The key number, previously loaded with DESFire::loadKey().
			 * \return true if authenticated, false otherwise.
			 */
			virtual bool authenticate(unsigned char keyno = 0) = 0;

			/**
			 * \brief Get the card version information.
			 * \param dataVersion The card version information structure that will be filled
			 * \return True on success, false otherwise.
			 */
			virtual bool getVersion(DESFireCardVersion& dataVersion) = 0;
	};
}

#endif

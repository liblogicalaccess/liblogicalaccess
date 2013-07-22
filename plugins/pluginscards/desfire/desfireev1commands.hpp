/**
 * \file desfireev1commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 commands.
 */

#ifndef LOGICALACCESS_DESFIREEV1COMMANDS_HPP
#define LOGICALACCESS_DESFIREEV1COMMANDS_HPP

#include "desfirecommands.hpp"


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

	#define ISO7816_INS_APPEND_RECORD 0xE2
	#define ISO7816_INS_GET_CHALLENGE 0x84
	#define ISO7816_INS_READ_RECORDS 0xB2
	#define ISO7816_INS_EXTERNAL_AUTHENTICATE 0x82
	#define ISO7816_INS_INTERNAL_AUTHENTICATE 0x88
	#define ISO7816_INS_SELECT_FILE 0xA4
	#define ISO7816_INS_READ_BINARY 0xB0
	#define ISO7816_INS_UPDATE_BINARY 0xD6

	#define DFEV1_CLA_ISO_COMPATIBLE 0x00

	#define SELECT_FILE_BY_AID  0x04
	#define SELECT_FILE_BY_FID 0x00
#endif

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
	typedef enum 
	{
		FIDS_NO_ISO_FID = 0x00,
		FIDS_ISO_FID = 0x20
	}FidSupport;	

	/**
	 * \brief The DESFire EV1 transmission mode.
	 */
	typedef enum 
	{
		DFEV1_CMD_PLAIN = 0x00,
		DFEV1_CMD_NOMACV = 0x01,
		DFEV1_CMD_MACV = 0x02
	}DESFireEV1TransmissionMode;

	/**
	 * \brief The DESFire EV1 transmission mode.
	 */
	typedef enum 
	{
		DF_RECORD_ONERECORD = 0x04,
		DF_RECORD_ALLRECORDS = 0x05
	}DESFireRecords;

	/**
	 * \brief The DESFire EV1 algorithm.
	 */
	typedef enum 
	{
		DF_ALG_BY_CONTEXT = 0x00,
		DF_ALG_2K3DES = 0x02,
		DF_ALG_3K3DES = 0x04,
		DF_ALG_AES = 0x09
	}DESFireISOAlgorithm;

	class DESFireEV1Chip;

	/**
	 * \brief The DESFire EV1 base commands class.
	 */
	class LIBLOGICALACCESS_API DESFireEV1Commands : public virtual DESFireCommands
	{
		public:		

			/**
			 * \brief Select an application.
			 * \param location The DESFire location
			 */
			virtual void selectApplication(boost::shared_ptr<DESFireLocation> location);

			/**
			 * \brief Create a new application.
			 * \param location The DESFire location
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 */
			virtual void createApplication(boost::shared_ptr<DESFireLocation> location, DESFireKeySettings settings, int maxNbKeys);

			/**
			 * \brief Create a new data file in the current application.
			 * \param location The DESFire location
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 */
			virtual void createStdDataFile(boost::shared_ptr<DESFireLocation> location, DESFireAccessRights accessRights, int fileSize);

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
			 * \brief Get the ISO FID of all active files within the currently selected application.
			 * \return The ISO FID list.
			 */
			virtual std::vector<unsigned short> getISOFileIDs() = 0;		

			/**
			 * \brief Create a new application.
			 * \param aid The Application ID to create
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 * \return True on success, false otherwise.
			 */
			virtual bool createApplication(int aid, DESFireKeySettings settings, size_t maxNbKeys, FidSupport fidSupported = FIDS_NO_ISO_FID, DESFireKeyType cryptoMethod = DF_KEY_DES, unsigned short isoFID = 0x00, const char* isoDFName = NULL) = 0;			

			/**
			 * \brief Get key settings on the current application.
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 * \param keyType The key type
			 */
			virtual void getKeySettings(DESFireKeySettings& settings, unsigned int& maxNbKeys, DESFireKeyType& keyType) = 0;

			/**
			 * \brief Get a random card UID.
			 * \return The card UID.
			 */
			virtual std::vector<unsigned char> getCardUID() = 0;

			/**
			 * \brief Create a new data file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size (in bytes).
			 * \return True on success, false otherwise.
			 */
			virtual bool createStdDataFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, unsigned short isoFID = 0x00) = 0;

			/**
			 * \brief Create a new backup file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size (in bytes).
			 * \return True on success, false otherwise.
			 */
			virtual bool createBackupFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, unsigned short isoFID = 0x00) = 0;			

			/**
			 * \brief Create a new linear record file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size (in bytes).
			 * \param maxNumberOfRecords Max number of records in the file.
			 * \return True on success, false otherwise.
			 */
			virtual bool createLinearRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords, unsigned short isoFID = 0x00) = 0;

			/**
			 * \brief Create a new cyclic record file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size (in bytes).
			 * \param maxNumberOfRecords Max number of records in the file.
			 * \return True on success, false otherwise.
			 */
			virtual bool createCyclicRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords, unsigned short isoFID = 0x00) = 0;			

			/**
			 * \brief Select file under current DF.
			 * \param fid The FID
			 */
			virtual void iso_selectFile(unsigned short fid = 0) = 0;

			/**
			 * \brief Read records.
			 * \param fid The FID
			 * \param start_record The start record (0 = read last written record)
			 * \param record_number The number of records to read
			 * \return The record(s) data
			 */
			virtual std::vector<unsigned char> iso_readRecords(unsigned short fid = 0, unsigned char start_record = 0, DESFireRecords record_number = DF_RECORD_ONERECORD) = 0;

			/**
			 * \brief Append a record to a file.
			 * \param data The record data
			 * \param fid The FID
			 */
			virtual void iso_appendrecord(const std::vector<unsigned char>& data = std::vector<unsigned char>(), unsigned short fid = 0) = 0;

			/**
			 * \brief Get the ISO challenge for authentication.
			 * \param length The challenge length (8 = 2K3DES, 16 = 3K3DES and AES)
			 * \return The ISO challenge.
			 */
			virtual std::vector<unsigned char> iso_getChallenge(unsigned int length = 8) = 0;

			/**
			 * \brief ISO external authenticate.
			 * \param algorithm The ISO algorithm to use for authentication.
			 * \param isMasterCardKey True if the key to authenticate is the master card key, false otherwise.
			 * \param keyno The key number.
			 * \param data The data.
			 */
			virtual void iso_externalAuthenticate(DESFireISOAlgorithm algorithm = DF_ALG_BY_CONTEXT, bool isMasterCardKey = true, unsigned char keyno = 0x00, const std::vector<unsigned char>& data = std::vector<unsigned char>()) = 0;

			/**
			 * \brief ISO internal authenticate.
			 * \param algorithm The ISO algorithm to use for authentication.
			 * \param isMasterCardKey True if the key to authenticate is the master card key, false otherwise.
			 * \param keyno The key number.
			 * \param RPCD2 The RPCD2.
			 * \param length The length.
			 * \return The cryptogram.
			 */
			virtual std::vector<unsigned char> iso_internalAuthenticate(DESFireISOAlgorithm algorithm = DF_ALG_BY_CONTEXT, bool isMasterCardKey = true, unsigned char keyno = 0x00, const std::vector<unsigned char>& RPCD2 = std::vector<unsigned char>(), unsigned int length = 16) = 0;

			/**
			 * \brief AuthenticateISO command.
			 * \param keyno The key number.
			 * \param algorithm The ISO algorithm to use for authentication.
			 */
			virtual void authenticateISO(unsigned char keyno, DESFireISOAlgorithm algorithm = DF_ALG_2K3DES) = 0;

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
			//virtual size_t readData(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode) = 0;						
			
			/**
			 * \brief Read record from a specific record file.
			 * \param fileno The file number
			 * \param offset The byte offset
			 * \param length The length to read
			 * \param data The data buffer
			 * \param mode The communication mode
			 * \return The number of bytes read.
			 */
			//virtual size_t readRecords(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode) = 0;			

			/**
			 * \brief Change file settings of a specific file in the current application.
			 * \param fileno The file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param plain Communication is currently in plain data.
			 * \return True on success, false otherwise.
			 */
			//virtual bool changeFileSettings(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain) = 0;

			/**
			 * \brief Change key settings for the current application.
			 * \param settings The new key settings
			 * \return True on success, false otherwise.
			 */
			//virtual bool changeKeySettings(DESFireKeySettings settings) = 0;

			/**
			 * \brief Change a key in the current application.
			 * \param keyno The key number to change
			 * \param key The new key
			 * \return True on success, false otherwise.
			 */
			//virtual bool changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key) = 0;

			/**
			 * \brief Get the card version information.
			 * \param dataVersion The card version information structure that will be filled
			 * \return True on success, false otherwise.
			 */
			//virtual bool getVersion(DESFireCommands::DESFireCardVersion& dataVersion) = 0;	

			/**
			 * \brief Get the current application list.
			 * \return The application list.
			 */
			//virtual std::vector<int> getApplicationIDs() = 0;

			/**
			 * \brief Get the File IDentifiers of all active files within the currently selected application
			 * \return The file ID list.
			 */
			//virtual std::vector<int> getFileIDs() = 0;

			/**
			 * \brief Get value from a specific value file.
			 * \param fileno The file number
			 * \param mode The communication mode
			 * \param value The value stored in the card
			 * \return True on success, false otherwise.
			 */
			//virtual bool getValue(int fileno, EncryptionMode mode, int& value) = 0;

			/**
			 * \brief ISO select application command.
			 * \param isoaid The iso AID
			 */
			virtual void iso_selectApplication(std::vector<unsigned char> isoaid = std::vector<unsigned char>(DFEV1_DESFIRE_AID, DFEV1_DESFIRE_AID + sizeof(DFEV1_DESFIRE_AID))) = 0;

			/**
			 * \brief Set the card configuration.
			 * \param formatCardEnabled If true, the format card command is enabled.
			 * \param randomIdEnabled If true, the CSN is random (can't be changed once activated).
			 */
			virtual void setConfiguration(bool formatCardEnabled, bool randomIdEnabled) = 0;

			/**
			 * \brief Set the card configuration default key.
			 * param defaultKey The new default key.
			 */
			virtual void setConfiguration(boost::shared_ptr<DESFireKey> defaultKey) = 0;

			/**
			 * \brief Set a custom card ATS.
			 * \param ats The new card ATS.
			 */
			virtual void setConfiguration(const std::vector<unsigned char>& ats) = 0;

		protected:
			
			boost::shared_ptr<DESFireEV1Chip> getDESFireEV1Chip() const;
	};
}

#endif /* LOGICALACCESS_DESFIREEV1CARDPROVIDER_HPP */


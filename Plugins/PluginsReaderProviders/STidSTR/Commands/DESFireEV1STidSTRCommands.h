/**
 * \file DESFireEV1STidSTRCommands.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief DESFire EV1 STidSTR commands.
 */

#ifndef LOGICALACCESS_DESFIREEV1STIDSTRCOMMANDS_H
#define LOGICALACCESS_DESFIREEV1STIDSTRCOMMANDS_H

#include "DESFireEV1Commands.h"
#include "../ReaderCardAdapters/STidSTRReaderCardAdapter.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	/**
	 * \brief The STid DESFire baudrates.
	 */
	typedef enum {
		STID_DF_BR_106KBPS = 0x00, /**< 106 kbps */
		STID_DF_BR_212KBPS = 0x01, /**< 212 kbps */
		STID_DF_BR_424KBPS = 0x02 /**< 424 kbps */
	} STidDESFireBaudrate;

	/**
	 * \brief The STid DESFire baudrates.
	 */
	typedef enum {
		STID_DF_KEYLOC_RAM = 0x00, /**< Key located in RAM memory */
		STID_DF_KEYLOC_EEPROM = 0x01, /**< Key located in EEPROM memory */
		STID_DF_KEYLOC_INDEXED = 0x02 /**< Key indexed */
	} STidKeyLocationType;

	class DESFireProfile;
	
	/**
	 * \brief The DESFire EV1 base commands class for STidSTR reader.
	 */
	class LIBLOGICALACCESS_API DESFireEV1STidSTRCommands : public DESFireCommands, public DESFireEV1Commands
	{
		public:

			/**
			 * \brief Constructor.
			 */
			DESFireEV1STidSTRCommands();

			/**
			 * \brief Destructor.
			 */
			virtual ~DESFireEV1STidSTRCommands();

			/**
			 * \brief Scan the RFID field for DESFire tag.
			 * \return The tag uid if present.
			 */
			std::vector<unsigned char> scanDESFire();

			/**
			 * \brief Release the RFID field.
			 */
			void releaseRFIDField();

			/**
			 * \brief Change the communication speed between the reader and the chip.
			 * \param readerToChipKbps The reader to chip kbps to use.
			 * \param chipToReaderKbps The chip to reader kbps to use.
			 */
			void changePPS(STidDESFireBaudrate readerToChipKbps, STidDESFireBaudrate chipToReaderKbps);
			
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
			 * \brief Erase the card.
			 * \return true if the card was erased, false otherwise. If false, the card may be partially erased.
			 */
			virtual bool erase(bool resetKey = true);

			/**
			 * \brief Create a new application.
			 * \param aid The Application ID to create
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 * \return True on success, false otherwise.
			 */
			virtual bool createApplication(int aid, DESFireKeySettings settings, int maxNbKeys);

			/**
			 * \brief Create a new application.
			 * \param aid The Application ID to create
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 * \return True on success, false otherwise.
			 */
			virtual bool createApplication(int aid, DESFireKeySettings settings, size_t maxNbKeys, FidSupport fidSupported = FIDS_NO_ISO_FID, DESFireKeyType cryptoMethod = DF_KEY_DES, unsigned short isoFID = 0x00, const char* isoDFName = NULL);

			/**
			 * \brief Delete an application.
			 * \param aid The Application ID to delete
			 * \return True on success, false otherwise.
			 */
			virtual bool deleteApplication(int aid);

			/**
			 * \brief Select an application.
			 * \param aid The Application ID
			 * \return true on success, false otherwise.
			 */
			virtual bool selectApplication(int aid);

			/**
			 * \brief Get key settings on the current application.
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 */
			virtual void getKeySettings(DESFireKeySettings& settings, unsigned int& maxNbKeys);

			/**
			 * \brief Get key settings on the current application.
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 * \param keyType The key type
			 */
			virtual void getKeySettings(DESFireKeySettings& settings, unsigned int& maxNbKeys, DESFireKeyType& keyType);

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
			 * \param fileSize The file size.
			 * \return True on success, false otherwise.
			 */
			virtual bool createStdDataFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize);

			/**
			 * \brief Create a new data file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size (in bytes).
			 * \return True on success, false otherwise.
			 */
			virtual bool createStdDataFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, unsigned short isoFID);

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
			 * \brief Create a new backup file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 * \return True on success, false otherwise.
			 */
			virtual bool createBackupFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize);

			/**
			 * \brief Create a new backup file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size (in bytes).
			 * \return True on success, false otherwise.
			 */
			virtual bool createBackupFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, unsigned short isoFID);			

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
			 * \brief Create a new linear record file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size (in bytes).
			 * \param maxNumberOfRecords Max number of records in the file.
			 * \return True on success, false otherwise.
			 */
			virtual bool createLinearRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords, unsigned short isoFID);

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
			 * \brief Create a new cyclic record file in the current application.
			 * \param fileno The new file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param fileSize The file size (in bytes).
			 * \param maxNumberOfRecords Max number of records in the file.
			 * \return True on success, false otherwise.
			 */
			virtual bool createCyclicRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords, unsigned short isoFID);			

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
			 * \brief Load a key in the reader memory.
			 * \param key The key object to load.
			 */
			virtual void loadKey(boost::shared_ptr<DESFireKey> key);

			/**
			 * \brief Load a key in the reader memory.
			 * \param key The key buffer to load.
			 * \param diversify True to diversify the key, false otherwise.
			 * \param isVolatile True to store it in RAM, false to store it in EEPROM.
			 */
			virtual void loadKey(std::vector<unsigned char> key, bool diversify, bool isVolatile);

			/**
			 * \brief Authenticate DESFire according to the key location.
			 * \param keylocation The key location on reader.
			 * \param keyno The key number, previously loaded.
			 * \param cryptoMethod The cryptographic method.
			 * \param keyindex The key index (if key location is indexed).
			 */
			virtual void authenticate(STidKeyLocationType keylocation, unsigned char keyno, DESFireKeyType cryptoMethod, unsigned char keyindex);

			/**
			 * \brief Authenticate, given a key number.
			 * \param keyno The key number.
			 * \return true if authenticated, false otherwise.
			 */
			virtual bool authenticate(unsigned char keyno);

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
			 * \brief Change file settings of a specific file in the current application.
			 * \param fileno The file number
			 * \param comSettings The file communication setting
			 * \param accessRights The file access rights
			 * \param plain Communication is currently in plain data.
			 * \return True on success, false otherwise.
			 */
			virtual bool changeFileSettings(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain);

			/**
			 * \brief Delete a file in the current application.
			 * \param fileno The file number to delete
			 * \return True on success, false otherwise.
			 */
			virtual bool deleteFile(int fileno);

			/**
			 * \brief Change key settings for the current application.
			 * \param settings The new key settings
			 * \return True on success, false otherwise.
			 */
			virtual bool changeKeySettings(DESFireKeySettings settings);

			/**
			 * \brief Change a key in the current application.
			 * \param keyno The key number to change
			 * \param key The new key
			 * \return True on success, false otherwise.
			 */
			virtual bool changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key);

			/**
			 * \brief Change the key indexed keyno by a new index or crypto method.
			 * \param keyno The key number into the application.
			 * \param cryptoMethod The cryptographic method.
			 * \param keyversion The key version (AES only).
			 * \param newkeyindex The new key index.
			 * \param oldkeyindex The old key index.
			 */
			void changeKeyIndex(unsigned char keyno, DESFireKeyType cryptoMethod, unsigned char keyversion, unsigned char newkeyindex, unsigned char oldkeyindex);

			/**
			 * \brief Get the key version.
			 * \param keyno The key number on the application.
			 * \return The key version.
			 */
			unsigned char getKeyVersion(unsigned char keyno);

			/**
			 * \brief Get the card version information.
			 * \param dataVersion The card version information structure that will be filled
			 * \return True on success, false otherwise.
			 */
			virtual bool getVersion(DESFireCommands::DESFireCardVersion& dataVersion);	

			/**
			 * \brief Get the current application list.
			 * \return The application list.
			 */
			virtual std::vector<int> getApplicationIDs();

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
			 * \brief Get value from a specific value file.
			 * \param fileno The file number
			 * \param mode The communication mode
			 * \param value The value stored in the card
			 * \return True on success, false otherwise.
			 */
			virtual bool getValue(int fileno, EncryptionMode mode, int& value);

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
			virtual void setConfiguration(boost::shared_ptr<DESFireKey> defaultKey);

			/**
			 * \brief Set a custom card ATS.
			 * \param ats The new card ATS.
			 */
			virtual void setConfiguration(const std::vector<unsigned char>& ats);

			/**
			 * \brief Get the STidSTR reader/card adapter.
			 * \return The STidSTR reader/card adapter.
			 */
			boost::shared_ptr<STidSTRReaderCardAdapter> getSTidSTRReaderCardAdapter() { return boost::dynamic_pointer_cast<STidSTRReaderCardAdapter>(getReaderCardAdapter()); };

			void setProfile(boost::shared_ptr<DESFireProfile> profile) { d_profile = profile; };

			/**
			 * \brief Get the STid Key Location Type in string format.
			 * \return The key location type in string.
			 */
			static std::string STidKeyLocationTypeStr(STidKeyLocationType t);

		protected:						

			int d_currentAid;

			boost::shared_ptr<DESFireProfile> d_profile;
	};
}

#endif /* LOGICALACCESS_DESFIREEV1STIDSTRCOMMANDS_H */


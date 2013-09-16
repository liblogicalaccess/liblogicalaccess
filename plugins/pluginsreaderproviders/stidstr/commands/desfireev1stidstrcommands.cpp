/**
 * \file desfireev1stidstrcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireEV1 STidSTR commands.
 */

#include "desfireev1stidstrcommands.hpp"
#include "desfirechip.hpp"
#include <openssl/rand.h>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"
#include "desfireprofile.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"

namespace logicalaccess
{
	DESFireEV1STidSTRCommands::DESFireEV1STidSTRCommands()
	{
		d_currentAid = 0;
	}

	DESFireEV1STidSTRCommands::~DESFireEV1STidSTRCommands()
	{
		
	}

	std::vector<unsigned char> DESFireEV1STidSTRCommands::scanDESFire()
	{
		INFO_SIMPLE_("Scanning DESFire card...");
		std::vector<unsigned char> uid;
		std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x0001, std::vector<unsigned char>());

		bool hasCard = (r[0] == 0x01);
		if (hasCard)
		{
			INFO_SIMPLE_("Card detected !");
			unsigned char uidLength = r[1];
			uid = std::vector<unsigned char>(r.begin() + 2, r.begin() + 2 + uidLength);

			INFO_("Card uid %s-{%s}", BufferHelper::getHex(uid).c_str(), BufferHelper::getStdString(uid).c_str());
		}
		else
		{
			INFO_SIMPLE_("No card detected !");
		}

		return uid;
	}

	void DESFireEV1STidSTRCommands::releaseRFIDField()
	{
		INFO_SIMPLE_("Releasing RFID field...");
		getSTidSTRReaderCardAdapter()->sendCommand(0x0002, std::vector<unsigned char>());
	}

	void DESFireEV1STidSTRCommands::changePPS(STidDESFireBaudrate readerToChipKbps, STidDESFireBaudrate chipToReaderKbps)
	{
		INFO_SIMPLE_("Changing PPS (communication speed between the chip and reader)...");
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(readerToChipKbps));
		command.push_back(static_cast<unsigned char>(chipToReaderKbps));
		getSTidSTRReaderCardAdapter()->sendCommand(0x0003, command);
	}

	unsigned int DESFireEV1STidSTRCommands::getFreeMem()
	{
		INFO_SIMPLE_("Getting free memory of card (number of bytes free available)...");
		unsigned int freemem = 0;
		std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x006E, std::vector<unsigned char>());

		if (r.size() == 3)
		{
			freemem = (r[0] << 16) | (r[1] << 8) | r[2];
		}
		else
		{
			ERROR_SIMPLE_("The response buffer size should be 3 !");
		}

		INFO_("Free memory {%u}", freemem);
		return freemem;
	}

	vector<DFName> DESFireEV1STidSTRCommands::getDFNames()
	{
		vector<DFName> dfnames;		
		
		ERROR_SIMPLE_("Function not available with this reader.");
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");

		return dfnames;
	}

	vector<unsigned short> DESFireEV1STidSTRCommands::getISOFileIDs()
	{
		vector<unsigned short> fileids;		

		ERROR_SIMPLE_("Function not available with this reader.");
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");

		return fileids;
	}

	void DESFireEV1STidSTRCommands::createApplication(unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys)
	{
		createApplication(aid, settings, maxNbKeys, DF_KEY_DES, FIDS_NO_ISO_FID, 0, NULL);
	}

	void DESFireEV1STidSTRCommands::createApplication(unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys, DESFireKeyType cryptoMethod, FidSupport /*fidSupported*/, unsigned short isoFID, const char* isoDFName)
	{
		INFO_("Creating application aid {0x%x(%d)} settings {0x%x(%d)} max nb Keys {%d} key type {0x%x(%d)}", aid, aid, settings, settings, maxNbKeys, cryptoMethod, cryptoMethod);

		unsigned char tmpcmd[3];
		std::vector<unsigned char> command;

		DESFireLocation::convertUIntToAid(aid, tmpcmd);
		command.insert(command.end(), tmpcmd, tmpcmd + 3);
		command.push_back(static_cast<unsigned char>(settings));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned char>(maxNbKeys) & 0x0f));
		command.push_back((cryptoMethod == DF_KEY_AES) ? 0x02 : 0x00);

		if (isoFID != 0x00)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "isoFID parameter not available with this reader.");
		}
		if (isoDFName != NULL)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "isoDFName parameter not available with this reader.");
		}

		getSTidSTRReaderCardAdapter()->sendCommand(0x00CA, command);

		for (unsigned char i = 0; i < maxNbKeys; ++i)
		{
			d_profile->setKey(aid, i, d_profile->getDefaultKey(cryptoMethod));
		}
	}

	void DESFireEV1STidSTRCommands::deleteApplication(unsigned int aid)
	{
		INFO_("Deleting application aid {0x%x(%d)}...", aid, aid);
		unsigned char command[3];

		DESFireLocation::convertUIntToAid(aid, command);

		getSTidSTRReaderCardAdapter()->sendCommand(0x00DA, std::vector<unsigned char>(command, command + sizeof(command)));
	}

	void DESFireEV1STidSTRCommands::selectApplication(unsigned int aid)
	{
		INFO_("Selecting application aid {0x%x(%d)}", aid, aid);
		unsigned char command[3];
		DESFireLocation::convertUIntToAid(aid, command);

		getSTidSTRReaderCardAdapter()->sendCommand(0x005A, std::vector<unsigned char>(command, command + sizeof(command)));

		d_currentAid = aid;
	}

	void DESFireEV1STidSTRCommands::getKeySettings(DESFireKeySettings& settings, unsigned char& maxNbKeys)
	{
		DESFireKeyType keyType;
		getKeySettings(settings, maxNbKeys, keyType);
	}

	void DESFireEV1STidSTRCommands::getKeySettings(DESFireKeySettings& settings, unsigned char& maxNbKeys, DESFireKeyType& keyType)
	{
		INFO_SIMPLE_("Retrieving key settings...");
		std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x0045, std::vector<unsigned char>());

		EXCEPTION_ASSERT_WITH_LOG(r.size() >= 3, LibLogicalAccessException, "The response length should be at least 3-byte long");

		settings = static_cast<DESFireKeySettings>(r[0]);
		maxNbKeys = r[1] & 0x0F;
		keyType = (r[2] == 0x02) ? DF_KEY_AES : DF_KEY_DES;

		INFO_("Returns settings {0x%x(%d)} max nb Keys {%u} key type {0x%x(%d)}", settings, settings, maxNbKeys, keyType, keyType);
	}

	unsigned char DESFireEV1STidSTRCommands::getKeyVersion(unsigned char keyno)
	{
		INFO_("Retrieving key version for key number {%u}...", keyno);
		std::vector<unsigned char> command;
		command.push_back(keyno);
		std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x0064, command);

		EXCEPTION_ASSERT_WITH_LOG(r.size() >= 1, LibLogicalAccessException, "The response length should be at least 1-byte long");

		return r[0];
	}

	std::vector<unsigned char> DESFireEV1STidSTRCommands::getCardUID()
	{
		INFO_SIMPLE_("Retrieving card uid...");
		std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x0051, std::vector<unsigned char>());

		return r;
	}

	void DESFireEV1STidSTRCommands::createStdDataFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize)
	{
		createStdDataFile(fileno, comSettings, accessRights, fileSize, 0x00);
	}

	void DESFireEV1STidSTRCommands::createStdDataFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned short isoFID)
	{
		INFO_("Creating standard data file - file number {0x%x(%d)} encryption mode {0x%x(%d)} access right {0x%x(%d)} file size {%d}...",
			fileno, fileno, comSettings, comSettings, accessRights, accessRights, fileSize);

		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(static_cast<unsigned char>(fileno));
		if (isoFID != 0)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "isoFID parameter not available with this reader.");		
		}
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		command.push_back(static_cast<unsigned char>(fileSize & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

		getSTidSTRReaderCardAdapter()->sendCommand(0x00CD, command);
	}

	void DESFireEV1STidSTRCommands::commitTransaction()
	{
		INFO_SIMPLE_("Committing transaction...");

		getSTidSTRReaderCardAdapter()->sendCommand(0x00C7, std::vector<unsigned char>());
	}

	void DESFireEV1STidSTRCommands::abortTransaction()
	{
		INFO_SIMPLE_("Aborting transaction...");

		getSTidSTRReaderCardAdapter()->sendCommand(0x00A7, std::vector<unsigned char>());
	}

	void DESFireEV1STidSTRCommands::createBackupFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize)
	{
		createBackupFile(fileno, comSettings, accessRights, fileSize, 0x00);
	}

	void DESFireEV1STidSTRCommands::createBackupFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned short isoFID)
	{
		INFO_("Creating backup file - file number {0x%x(%d)} encryption mode {0x%x(%d)} access right {0x%x(%d)} file size {%d}...",
			fileno, fileno, comSettings, comSettings, accessRights, accessRights, fileSize);

		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(static_cast<unsigned char>(fileno));
		if (isoFID != 0)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "isoFID parameter not available with this reader.");		
		}
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		command.push_back(static_cast<unsigned char>(fileSize & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

		getSTidSTRReaderCardAdapter()->sendCommand(0x00CB, command);
	}

	void DESFireEV1STidSTRCommands::createValueFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int lowerLimit, unsigned int upperLimit, unsigned int value, bool limitedCreditEnabled)
	{
		INFO_("Creating value file - file number {0x%x(%d)} encryption mode {0x%x(%d)} access right {0x%x(%d)} lower limit {%d} upper limit {%d} value {0x%x(%d)}, limited credit enabled {%d}...",
			fileno, fileno, comSettings, comSettings, accessRights, accessRights, lowerLimit, upperLimit, value, value, limitedCreditEnabled);

		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(fileno);
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		BufferHelper::setInt32(command, lowerLimit);
		BufferHelper::setInt32(command, upperLimit);
		BufferHelper::setInt32(command, value);
		command.push_back(limitedCreditEnabled ? 0x01 : 0x00);

		getSTidSTRReaderCardAdapter()->sendCommand(0x00CC, command);
	}

	void DESFireEV1STidSTRCommands::createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords)
	{
		createLinearRecordFile(fileno, comSettings, accessRights, fileSize, maxNumberOfRecords, 0x00);
	}

	void DESFireEV1STidSTRCommands::createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords, unsigned short isoFID)
	{
		INFO_("Creating linear record file - file number {0x%x(%d)} encryption mode {0x%x(%d)} access right {0x%x(%d)} file size {%d} max number records {%d}...",
			fileno, fileno, comSettings, comSettings, accessRights, accessRights, fileSize, maxNumberOfRecords);

		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(fileno);
		if (isoFID != 0)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "isoFID parameter not available with this reader.");			
		}
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		command.push_back(static_cast<unsigned char>(fileSize & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));
		command.push_back(static_cast<unsigned char>(maxNumberOfRecords & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(maxNumberOfRecords & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(maxNumberOfRecords & 0xff0000) >> 16));

		getSTidSTRReaderCardAdapter()->sendCommand(0x00C1, command);
	}

	void DESFireEV1STidSTRCommands::createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords)
	{
		createCyclicRecordFile(fileno, comSettings, accessRights, fileSize, maxNumberOfRecords, 0x00);
	}

	void DESFireEV1STidSTRCommands::createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords, unsigned short isoFID)
	{
		INFO_("Creating cyclic record file - file number {0x%x(%d)} encryption mode {0x%x(%d)} access right {0x%x(%d)} file size {%d} max number records {%d}...",
			fileno, fileno, comSettings, comSettings, accessRights, accessRights, fileSize, maxNumberOfRecords);

		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(fileno);
		if (isoFID != 0)
		{
			command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(isoFID & 0xff00) >> 8));
			command.push_back(static_cast<unsigned char>(isoFID & 0xff));			
		}
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		command.push_back(static_cast<unsigned char>(fileSize & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));
		command.push_back(static_cast<unsigned char>(maxNumberOfRecords & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(maxNumberOfRecords & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(maxNumberOfRecords & 0xff0000) >> 16));

		getSTidSTRReaderCardAdapter()->sendCommand(0x00C0, command);
	}

	void DESFireEV1STidSTRCommands::iso_selectFile(unsigned short /*fid*/)
	{
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
	}

	std::vector<unsigned char> DESFireEV1STidSTRCommands::iso_readRecords(unsigned short /*fid*/, unsigned char /*start_record*/, DESFireRecords /*record_number*/)
	{
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
		return std::vector<unsigned char>();
	}

	void DESFireEV1STidSTRCommands::iso_appendrecord(const std::vector<unsigned char>& /*data*/, unsigned short /*fid*/)
	{
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
	}

	std::vector<unsigned char> DESFireEV1STidSTRCommands::iso_getChallenge(unsigned int /*length*/)
	{
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
		return std::vector<unsigned char>();
	}

	void DESFireEV1STidSTRCommands::iso_externalAuthenticate(DESFireISOAlgorithm /*algorithm*/, bool /*isMasterCardKey*/, unsigned char /*keyno*/, const std::vector<unsigned char>& /*data*/)
	{
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
	}

	std::vector<unsigned char> DESFireEV1STidSTRCommands::iso_internalAuthenticate(DESFireISOAlgorithm /*algorithm*/, bool /*isMasterCardKey*/, unsigned char /*keyno*/, const std::vector<unsigned char>& /*RPCD2*/, unsigned int /*length*/)
	{
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
		return std::vector<unsigned char>();
	}

	void DESFireEV1STidSTRCommands::loadKey(boost::shared_ptr<DESFireKey> key)
	{
		INFO_("Loading key from storage {%s}...", boost::dynamic_pointer_cast<XmlSerializable>(key)->serialize().c_str());
		boost::shared_ptr<KeyStorage> key_storage = key->getKeyStorage();

		if (boost::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
		{
			INFO_SIMPLE_("Using computer memory key storage...");
			unsigned char* keydata = key->getData();
			loadKey(std::vector<unsigned char>(keydata, keydata + key->getLength()), key->getDiversify(), true);
		}
		else if (boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
		{
			INFO_SIMPLE_("Using reader memory key storage... -> Doing nothing !");
		}
		else
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The key storage type is not supported for this card/reader.");
		}
	}

	void DESFireEV1STidSTRCommands::loadKey(std::vector<unsigned char> key, bool /*diversify*/, bool /*isVolatile*/)
	{
		INFO_("Loading key in reader memory... key size {%d}", key.size());
		EXCEPTION_ASSERT_WITH_LOG(key.size() == 16, LibLogicalAccessException, "The key length must be 16-byte long");

		std::vector<unsigned char> command;
		command.push_back(0x00); //(isVolatile ? 0x00 : 0x01);  Always use the Reader RAM Memory (0x00) (if want to write in EEPROM, use 0x01)
		command.insert(command.end(), key.begin(), key.end());
		command.push_back(0x00);	// Diversify, not supported yet.

		getSTidSTRReaderCardAdapter()->sendCommand(0x00D0, command);
	}

	void DESFireEV1STidSTRCommands::authenticate(STidKeyLocationType keylocation, unsigned char keyno, DESFireKeyType cryptoMethod, unsigned char keyindex)
	{
		INFO_("Authenticating from location... key location {%s} key number {0x%x(%u)}, key type {%s}, key index {0x%x(%u)}",
			DESFireEV1STidSTRCommands::STidKeyLocationTypeStr(keylocation).c_str(), keyno, keyno,
			DESFireKey::DESFireKeyTypeStr(cryptoMethod).c_str(), keyindex, keyindex);

		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(keylocation));
		command.push_back(keyno);
		command.push_back((cryptoMethod == DF_KEY_AES) ? 0x02 : 0x00);
		command.push_back(keyindex);

		getSTidSTRReaderCardAdapter()->sendCommand(0x000B, command);
	}

	void DESFireEV1STidSTRCommands::authenticate(unsigned char keyno)
	{
		boost::shared_ptr<DESFireKey> key = d_profile->getKey(d_currentAid, keyno);
		authenticate(keyno, key);
	}

	void DESFireEV1STidSTRCommands::authenticate(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
	{
		INFO_("Authenticating... key number {0x%x(%u)}",  keyno, keyno);

		boost::shared_ptr<KeyStorage> key_storage = key->getKeyStorage();
		STidKeyLocationType keylocation = STID_DF_KEYLOC_RAM;
		unsigned char keyindex = 0x00;

		loadKey(key);	// Load key in the reader memory if needed (only when using Computer Memory Storage)

		if (boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
		{
			INFO_SIMPLE_("Retreving key index from reader memory key storage...");
			boost::shared_ptr<ReaderMemoryKeyStorage> rmKs = boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
			keylocation = STID_DF_KEYLOC_INDEXED;
			keyindex = rmKs->getKeySlot();
		}
		
		authenticate(keylocation, keyno, key->getKeyType(), keyindex);
	}	

	void DESFireEV1STidSTRCommands::authenticateISO(unsigned char /*keyno*/, DESFireISOAlgorithm /*algorithm*/)
	{
		ERROR_SIMPLE_("Function not available with this reader.");
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
	}

	void DESFireEV1STidSTRCommands::authenticateAES(unsigned char /*keyno*/)
	{
		ERROR_SIMPLE_("Function not available with this reader.");
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
	}

	unsigned int DESFireEV1STidSTRCommands::readData(unsigned char fileno, unsigned int offset, unsigned int length, void* data, EncryptionMode mode)
	{
		INFO_("Reading data... file number {0x%x(%d)} offset {%d} length {%d} encrypt mode {0x%x(%d)}",  fileno, fileno, offset, length, mode, mode);
		unsigned int ret = 0;
		std::vector<unsigned char> command;
		
		size_t stidMaxDataSize = 1024; // New to version 1.2 of API (before was limited to 200)
		// STid STR reader doesn't support data larger than <stidMaxDataSize> bytes, we should split the command.
		for (size_t i = 0; i < length; i += stidMaxDataSize)
		{
			INFO_("Reading data from index {%d} to {%d}...", i, i+stidMaxDataSize-1);
			command.clear();
			command.push_back(static_cast<unsigned char>(mode));
			command.push_back(fileno);
			size_t trunloffset = offset + i;
			size_t trunklength = ((length - i) > stidMaxDataSize) ? stidMaxDataSize : (length - i);			
			command.insert(command.end(), (unsigned char*)&trunloffset, (unsigned char*)&trunloffset + 3);
			command.insert(command.end(), (unsigned char*)&trunklength, (unsigned char*)&trunklength + 3);

			std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x00BD, command);

			memcpy(reinterpret_cast<unsigned char*>(data) + i, &result[0], result.size());
			ret += static_cast<unsigned int>(result.size());
		}
		INFO_("Returns data size {%d}", ret);

		return ret;
	}

	void DESFireEV1STidSTRCommands::writeData(unsigned char fileno, unsigned int offset, unsigned int length, const void* data, EncryptionMode mode)
	{
		INFO_("Writing data... file number {0x%x(%d)} offset {%d} length {%d} encrypt mode {0x%x(%d)}", fileno, fileno, offset, length, mode, mode);
		std::vector<unsigned char> command;
		
		size_t stidMaxDataSize = 1024; // New to version 1.2 of API (before was limited to 200)
		// STid STR reader doesn't support data larger than <stidMaxDataSize> bytes, we should split the command.
		for (size_t i = 0; i < length; i += stidMaxDataSize)
		{
			INFO_("Writing data from index {%d} to {%d}...", i, i+stidMaxDataSize-1);
			command.clear();
			command.push_back(static_cast<unsigned char>(mode));
			command.push_back(fileno);
			size_t trunloffset = offset + i;
			size_t trunklength = ((length - i) > stidMaxDataSize) ? stidMaxDataSize : (length - i);			
			command.insert(command.end(), (unsigned char*)&trunloffset, (unsigned char*)&trunloffset + 3);
			command.insert(command.end(), (unsigned char*)&trunklength, (unsigned char*)&trunklength + 3);
			command.insert(command.end(), reinterpret_cast<const unsigned char*>(data) + i, reinterpret_cast<const unsigned char*>(data) + i + trunklength);

			getSTidSTRReaderCardAdapter()->sendCommand(0x003D, command);
		}
	}

	void DESFireEV1STidSTRCommands::credit(unsigned char fileno, unsigned int value, EncryptionMode mode)
	{
		INFO_("Increasing value file... file number {0x%x(%d)} value {0x%x(%d)} encrypt mode {0x%x(%d)}", fileno, fileno, value, value, mode, mode);
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(mode));
		command.push_back(fileno);
		BufferHelper::setUInt32(command, value);

		getSTidSTRReaderCardAdapter()->sendCommand(0x000C, command);
	}

	void DESFireEV1STidSTRCommands::debit(unsigned char fileno, unsigned int value, EncryptionMode mode)
	{
		INFO_("Decreasing value file... file number {0x%x(%d)} value {0x%x(%d)} encrypt mode {0x%x(%d)}", fileno, fileno, value, value, mode, mode);
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(mode));
		command.push_back(fileno);
		BufferHelper::setUInt32(command, value);

		getSTidSTRReaderCardAdapter()->sendCommand(0x00DC, command);
	}

	void DESFireEV1STidSTRCommands::limitedCredit(unsigned char fileno, unsigned int value, EncryptionMode mode)
	{
		INFO_("Increasing value file with limited amount... file number {0x%x(%d)} value {0x%x(%d)} encrypt mode {0x%x(%d)}", fileno, fileno, value, value, mode, mode);
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(mode));
		command.push_back(fileno);
		BufferHelper::setUInt32(command, value);

		getSTidSTRReaderCardAdapter()->sendCommand(0x001C, command);
	}

	void DESFireEV1STidSTRCommands::writeRecord(unsigned char fileno, unsigned int offset, unsigned int length, const void* data, EncryptionMode mode)
	{
		INFO_("Writing record... file number {0x%x(%d)} offset {%d} length {%d} encrypt mode {0x%x(%d)}", fileno, fileno, offset, length, mode, mode);
		std::vector<unsigned char> command;

		size_t stidMaxDataSize = 1024; // New to version 1.2 of API (before was limited to 200)
		// STid STR reader doesn't support data larger than <stidMaxDataSize> bytes, we should split the command.
		for (size_t i = 0; i < length; i += stidMaxDataSize)
		{
			INFO_("Writing data from index {%d} to {%d}...", i, i+stidMaxDataSize-1);
			command.clear();
			command.push_back(static_cast<unsigned char>(mode));
			command.push_back(fileno);
			size_t trunloffset = offset + i;
			size_t trunklength = ((length - i) > stidMaxDataSize) ? stidMaxDataSize : (length - i);			
			command.insert(command.end(), (unsigned char*)&trunloffset, (unsigned char*)&trunloffset + 3);
			command.insert(command.end(), (unsigned char*)&trunklength, (unsigned char*)&trunklength + 3);
			command.insert(command.end(), reinterpret_cast<const unsigned char*>(data) + i, reinterpret_cast<const unsigned char*>(data) + i + trunklength);

			getSTidSTRReaderCardAdapter()->sendCommand(0x003B, command);
		}
	}

	unsigned int DESFireEV1STidSTRCommands::readRecords(unsigned char fileno, unsigned int offset, unsigned int nbrecords, void* data, EncryptionMode mode)
	{
		INFO_("Reading record... file number {0x%x(%d)} offset {%d} nb records {%d} encrypt mode {0x%x(%d)}", fileno, fileno, offset, nbrecords, mode, mode);

		//size_t stidMaxDataSize = 1024; // New to version 1.2 of API (before was limited to 200)
		// STid STR reader doesn't support data larger than <stidMaxDataSize> bytes, a record cannot exceed this size.

		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(mode));
		command.push_back(fileno);	
		command.insert(command.end(), (unsigned char*)&offset, (unsigned char*)&offset + 3);
		command.insert(command.end(), (unsigned char*)&nbrecords, (unsigned char*)&nbrecords + 3);

		std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x00BB, command);

		memcpy(reinterpret_cast<unsigned char*>(data), &result[0], result.size());

		INFO_("Returns size {%d}", result.size());
		return static_cast<unsigned int>(result.size());
	}

	void DESFireEV1STidSTRCommands::clearRecordFile(unsigned char fileno)
	{
		INFO_("Clearing record... file number {0x%x(%d)}", fileno, fileno);

		std::vector<unsigned char> command;
		command.push_back(fileno);

		getSTidSTRReaderCardAdapter()->sendCommand(0x00EB, command);
	}

	void DESFireEV1STidSTRCommands::changeFileSettings(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain)
	{
		INFO_("Changing file settings... file number {0x%x(%d)} com settings {0x%x(%d)} access rights {0x%x(%d)} plain {%d}",
				fileno, fileno, comSettings, comSettings, accessRights, accessRights, plain, plain);

		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);
		
		command.push_back(fileno);
		command.push_back(plain ? 0x00 : 0x01);
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);

		getSTidSTRReaderCardAdapter()->sendCommand(0x005F, command);
	}

	void DESFireEV1STidSTRCommands::deleteFile(unsigned char fileno)
	{
		INFO_("Deleting file... file number {0x%x(%d)}", fileno, fileno);

		std::vector<unsigned char> command;
		command.push_back(fileno);

		getSTidSTRReaderCardAdapter()->sendCommand(0x00DF, command);
	}

	void DESFireEV1STidSTRCommands::changeKeySettings(DESFireKeySettings settings)
	{
		INFO_("Changing key settings... settings {0x%x(%d)}", settings, settings);
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(settings));

		getSTidSTRReaderCardAdapter()->sendCommand(0x0054, command);
	}

	void DESFireEV1STidSTRCommands::changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
	{
		INFO_("Changing key... key number {0x%x(%u)} new key {%s}", keyno, keyno, boost::dynamic_pointer_cast<XmlSerializable>(key)->serialize().c_str());
		// Only change the key if new key and old key are not the same.
		boost::shared_ptr<DESFireKey> oldKey = d_profile->getKey(d_currentAid, keyno);

		INFO_("Old key {%s}", boost::dynamic_pointer_cast<XmlSerializable>(oldKey)->serialize().c_str());

		if (key != oldKey)
		{
			// Old key and new key use reader memory. Use change keyindex instead.
			if (key->getKeyStorage()->getType() == KST_READER_MEMORY && oldKey->getKeyStorage()->getType() == KST_READER_MEMORY)
			{
				INFO_SIMPLE_("Changing key by using reader EEPROM INDEXED memory key storage...");
				changeKeyIndex(keyno, key->getKeyType(), key->getKeyVersion(), boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key->getKeyStorage())->getKeySlot(), boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(oldKey->getKeyStorage())->getKeySlot());
			}
			else
			{
				INFO_SIMPLE_("Changing key directly...");
				std::vector<unsigned char> command;
				command.push_back(keyno);
				command.push_back((key->getKeyType() == DF_KEY_AES) ? 0x02 : 0x00);
				if (key->getKeyType() == DF_KEY_AES)
				{
					command.push_back(key->getKeyVersion());
				}

				unsigned char* keydata = key->getData();
				command.insert(command.end(), keydata, keydata + key->getLength());		
				keydata = oldKey->getData();
				command.insert(command.end(), keydata, keydata + oldKey->getLength());	

				getSTidSTRReaderCardAdapter()->sendCommand(0x00C4, command);
			}
		}
		else
		{
			INFO_SIMPLE_("Key was already loaded. Doing nothing.");
		}
	}

	void DESFireEV1STidSTRCommands::changeKeyIndex(unsigned char keyno, DESFireKeyType cryptoMethod, unsigned char keyversion, unsigned char newkeyindex, unsigned char oldkeyindex)
	{
		INFO_("Changing key index... key number {0x%x(%u)} crypto method {%s} key version {0x%x(%u)} new key index {0x%x(%u)} old key index {0x%x(%u)}",
			keyno, keyno, DESFireKey::DESFireKeyTypeStr(cryptoMethod).c_str(), keyversion, keyversion, newkeyindex, newkeyindex, oldkeyindex, oldkeyindex);

		std::vector<unsigned char> command;
		command.push_back(keyno);
		command.push_back((cryptoMethod == DF_KEY_AES) ? 0x02 : 0x00);
		if (cryptoMethod == DF_KEY_AES)
		{
			command.push_back(keyversion);
		}
		command.push_back(newkeyindex);
		command.push_back(oldkeyindex);
		command.push_back(0x00); // RFU

		getSTidSTRReaderCardAdapter()->sendCommand(0x00C5, command);
	}

	void DESFireEV1STidSTRCommands::getVersion(DESFireCommands::DESFireCardVersion& dataVersion)
	{
		INFO_SIMPLE_("Retrieving version...");
		std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x0060, std::vector<unsigned char>());

		EXCEPTION_ASSERT_WITH_LOG(result.size() >= 28, LibLogicalAccessException, "The response length should be at least 28-byte long");

		memcpy(reinterpret_cast<char*>(&dataVersion), &result[0], 28);
	}

	std::vector<unsigned int> DESFireEV1STidSTRCommands::getApplicationIDs()
	{
		INFO_SIMPLE_("Retrieving all application ids...");
		std::vector<unsigned int> aids;

		std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x006A, std::vector<unsigned char>());

		unsigned nbaids = result[0];
		for (size_t i = 0; i < nbaids; ++i)
		{
			aids.push_back(DESFireLocation::convertAidToUInt(&result[1 + (i*3)]));
		}

		for (std::vector<unsigned int>::iterator it = aids.begin(); it != aids.end(); ++it)
		{
			INFO_("Processing application id {0x%x(%d)}...", *it, *it);
		}

		return aids;
	}

	void DESFireEV1STidSTRCommands::erase()
	{
		INFO_SIMPLE_("Erasing card...");
		getSTidSTRReaderCardAdapter()->sendCommand(0x00FC, std::vector<unsigned char>());
	}

	std::vector<unsigned char> DESFireEV1STidSTRCommands::getFileIDs()
	{
		INFO_SIMPLE_("Retrieving all files ids...");
		std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x006F, std::vector<unsigned char>());

		unsigned char nbfiles = result[0];

		std::vector<unsigned char> files;
		for (size_t i = 1; i <= nbfiles; ++i)
		{
			files.push_back(result[i]);
		}

		for (std::vector<unsigned char>::iterator it = files.begin(); it != files.end(); ++it)
		{
			INFO_("Processing file id {0x%x(%d)}...", *it, *it);
		}

		return files;
	}

	void DESFireEV1STidSTRCommands::getFileSettings(unsigned char fileno, FileSetting& fileSetting)
	{
		INFO_("Retrieving file settings for file number {0x%x(%d)}", fileno, fileno);

		std::vector<unsigned char> command;
		command.push_back(fileno);

		std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x00F5, command);
		memcpy(&fileSetting, &result[0], result.size());
	}

	void DESFireEV1STidSTRCommands::getValue(unsigned char fileno, EncryptionMode mode, unsigned int& value)
	{
		INFO_("Retrieving value for file number {0x%x(%d)} encrypt mode {0x%x(%d)}", fileno, fileno, mode, mode);

		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(mode));
		command.push_back(fileno);

		std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x00F5, command);

		if (result.size() >= 4)
		{
			size_t offset = 0;
			value = BufferHelper::getUInt32(result, offset);
			INFO_("Result value {0x%x(%d)}", value, value);
		}
	}

	void DESFireEV1STidSTRCommands::iso_selectApplication(std::vector<unsigned char> /*isoaid*/)
	{
		ERROR_SIMPLE_("Function not available with this reader.");
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
	}

	void DESFireEV1STidSTRCommands::setConfiguration(bool formatCardEnabled, bool randomIdEnabled)
	{
		std::vector<unsigned char> command;
		command.push_back(0x00);
		command.push_back(0x01);
		command.push_back(0x00 | ((formatCardEnabled) ? 0x00 : 0x01) | ((randomIdEnabled) ? 0x02 : 0x00));

		std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x005C, command);
	}

	void DESFireEV1STidSTRCommands::setConfiguration(boost::shared_ptr<DESFireKey> defaultKey)
	{
		if (defaultKey->getLength() < 24)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The default key length must be 24-byte long.");
		}

		std::vector<unsigned char> command;
		command.push_back(0x01);
		command.push_back(0x19);
		unsigned char* keydata = defaultKey->getData();
		command.insert(command.end(), keydata, keydata + 24);
		command.push_back(defaultKey->getKeyVersion());

		std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x005C, command);
	}

	void DESFireEV1STidSTRCommands::setConfiguration(const std::vector<unsigned char>& ats)
	{
		std::vector<unsigned char> command;
		command.push_back(0x02);
		command.push_back(static_cast<unsigned char>(ats.size()));
		command.insert(command.end(), ats.begin(), ats.end());

		std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x005C, command);
	}

	std::string DESFireEV1STidSTRCommands::STidKeyLocationTypeStr(STidKeyLocationType t)
	{
		switch (t)
		{
			case STID_DF_KEYLOC_RAM: return "STID_DF_KEYLOC_RAM";
			case STID_DF_KEYLOC_EEPROM: return "STID_DF_KEYLOC_EEPROM";
			case STID_DF_KEYLOC_INDEXED: return "STID_DF_KEYLOC_INDEXED";
			default: return "Unknown";
		}
	}
}

/**
 * \file MifareSTidSTRCommands.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Mifare STidSTR commands.
 */

#include "MifareSTidSTRCommands.h"
#include "../STidSTRReaderProvider.h"
#include "MifareChip.h"
#include "logicalaccess/Cards/ComputerMemoryKeyStorage.h"
#include "logicalaccess/Cards/ReaderMemoryKeyStorage.h"
#include "logicalaccess/Cards/SAMKeyStorage.h"

namespace LOGICALACCESS
{	
	MifareSTidSTRCommands::MifareSTidSTRCommands()
		: MifareCommands()
	{
		//INFO_SIMPLE_("Constructor");
		d_useSKB = false;
		d_skbIndex = 0;
		d_lastKeyType = KT_KEY_A;
	}

	MifareSTidSTRCommands::~MifareSTidSTRCommands()
	{
		//INFO_SIMPLE_("Destructor");
	}

	std::vector<unsigned char> MifareSTidSTRCommands::scanMifare()
	{
		INFO_SIMPLE_("Scanning mifare card...");
		std::vector<unsigned char> uid;
		unsigned char statusCode;
		std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x00A0, std::vector<unsigned char>(), statusCode);

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

	void MifareSTidSTRCommands::releaseRFIDField()
	{
		INFO_SIMPLE_("Releasing RFID field...");
		unsigned char statusCode;
		getSTidSTRReaderCardAdapter()->sendCommand(0x00A1, std::vector<unsigned char>(), statusCode);
	}

	bool MifareSTidSTRCommands::loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol)
	{
		INFO_("Loading key... key number {0x%x(%u)} key type {0x%x(%d)} key len {%d} volatile ? {%d}", keyno, keyno, keytype, keytype, keylen, vol);
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(keytype));
		command.push_back(vol ? 0x00 : 0x01);
		command.push_back(keyno);
		command.insert(command.end(), (unsigned char*)key, (unsigned char*)key + keylen);
		command.push_back(0x00);	// Diversify ?

		unsigned char statusCode;
		getSTidSTRReaderCardAdapter()->sendCommand(0x00D0, command, statusCode);

		return true;
	}

	void MifareSTidSTRCommands::loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifareKeyType keytype)
	{
		INFO_("Loading key... location {%s} key type {0x%x(%d)}", location->serialize().c_str(), keytype, keytype);

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(key, std::invalid_argument, "key cannot be null.");

		boost::shared_ptr<MifareLocation> mLocation = boost::dynamic_pointer_cast<MifareLocation>(location);
		boost::shared_ptr<MifareKey> mKey = boost::dynamic_pointer_cast<MifareKey>(key);

		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
		EXCEPTION_ASSERT_WITH_LOG(mKey, std::invalid_argument, "key must be a MifareKey.");

		boost::shared_ptr<KeyStorage> key_storage = key->getKeyStorage();

		if (boost::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
		{
			INFO_SIMPLE_("Using computer memory key storage !");
			loadKey(static_cast<unsigned char>(mLocation->sector), keytype, key->getData(), key->getLength(), true);
		}
		else if (boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
		{
			INFO_SIMPLE_("Using reader memory key storage !");
			// Don't load the key when reader memory, except if specified
			if (!key->isEmpty())
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "It's not possible to change a key at specific index through host/reader connection. Please use SKB configuration card instead.");
			}
		}
		else
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "The key storage type is not supported for this card/reader.");
		}
	}

	bool MifareSTidSTRCommands::authenticate(unsigned char /*blockno*/, unsigned char /*keyno*/, MifareKeyType /*keytype*/)
	{
		// STid STR doesn't separate authentication and read/write operation.
		WARNING_SIMPLE_("STid STR doesn't separate authentication and read/write operation.");
		return true;
	}

	void MifareSTidSTRCommands::authenticate(unsigned char /*blockno*/, boost::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype)
	{		
		INFO_("Setting the authenticate type... key storage {%s} key type {0x%x(%d)}", key_storage->serialize().c_str(), keytype, keytype);
		if (boost::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
		{
			INFO_SIMPLE_("Setting computer memory key storage !");
			d_useSKB = false;
			d_skbIndex = 0;
		}
		else if (boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
		{
			d_useSKB = true;
			boost::shared_ptr<ReaderMemoryKeyStorage> rmks = boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
			d_skbIndex = rmks->getKeySlot();
			INFO_("Setting reader memory key storage ! SKB index {%d}", d_skbIndex);
		}
		else
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "The key storage type is not supported for this card/reader.");
		}
		d_lastKeyType = keytype;
	}

	size_t MifareSTidSTRCommands::readBinary(unsigned char blockno, size_t len, void* buf, size_t buflen)
	{
		INFO_("Read binary block {0x%x(%u)} len {%d} [out] buffer len {%d}", blockno, blockno, len, buflen);
		
		if ((len >= 256) || (len > buflen) || (!buf))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
		}

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		if (d_useSKB)
		{
			INFO_SIMPLE_("Need to use reader memory key storage (SKB) !");
			return readBinaryIndex(d_skbIndex, blockno, len, buf, buflen);
		}

		INFO_SIMPLE_(" => Rescanning card to avoid bad authentication");
		scanMifare();
		INFO_SIMPLE_("Scan done ! Continue to read binary block.");

		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(d_lastKeyType));
		command.push_back(blockno);

		unsigned char statusCode;
		std::vector<unsigned char> sbuf = getSTidSTRReaderCardAdapter()->sendCommand(0x00B2, command, statusCode);

		INFO_("Read binary buffer returned %s len {%d}", BufferHelper::getHex(sbuf).c_str(), sbuf.size());
		EXCEPTION_ASSERT_WITH_LOG(sbuf.size() == 16, LibLOGICALACCESSException, "The read value should always be 16 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(sbuf.size() <= buflen, LibLOGICALACCESSException, "Buffer is too small to get all response value");

		memcpy(buf, &sbuf[0], sbuf.size());

		return sbuf.size();
	}

	size_t MifareSTidSTRCommands::readBinaryIndex(unsigned char keyindex, unsigned char blockno, size_t /*len*/, void* buf, size_t buflen)
	{
		INFO_("Read binary index key index {0x%x(%u)} block {0x%x(%u)} [out] buffer len {%d}",
				keyindex, keyindex, blockno, blockno, buflen);

		INFO_SIMPLE_(" => Rescanning card to avoid bad authentication");
		scanMifare();
		INFO_SIMPLE_("Scan done ! Continue to read binary index.");

		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(d_lastKeyType));
		command.push_back(keyindex);
		command.push_back(blockno);

		unsigned char statusCode;
		std::vector<unsigned char> sbuf = getSTidSTRReaderCardAdapter()->sendCommand(0x00B1, command, statusCode);

		EXCEPTION_ASSERT_WITH_LOG(sbuf.size() == 16, LibLOGICALACCESSException, "The read value should always be 16 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(buflen >= sbuf.size(), LibLOGICALACCESSException, "The buffer is too short to store the result.");

		memcpy(buf, &sbuf[0], sbuf.size());

		return sbuf.size();
	}

	size_t MifareSTidSTRCommands::updateBinary(unsigned char blockno, const void* buf, size_t buflen)
	{
		INFO_("Update binary block {0x%x(%u)} [in] buffer len {%d}", blockno, blockno, buflen);

		if ((buflen >= 256) || (!buf))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
		}

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasWriteDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		if (d_useSKB)
		{
			INFO_SIMPLE_("Need to use reader memory key storage (SKB) !");
			return updateBinaryIndex(d_skbIndex, blockno, buf, buflen);
		}

		INFO_SIMPLE_(" => Rescanning card to avoid bad authentication");
		scanMifare();
		INFO_SIMPLE_("Scan done ! Continue to update binary block.");

		if (blockno != 0)
		{
			std::vector<unsigned char> command;
			command.push_back(static_cast<unsigned char>(d_lastKeyType));
			command.push_back(blockno);
			command.insert(command.end(), (unsigned char*)buf, (unsigned char*)buf + buflen);

			unsigned char statusCode;
			getSTidSTRReaderCardAdapter()->sendCommand(0x00D2, command, statusCode);
		}

		INFO_("Returns final [out] buffer len {%d}", buflen);
		return buflen;
	}	

	size_t MifareSTidSTRCommands::updateBinaryIndex(unsigned char keyindex, unsigned char blockno, const void* buf, size_t buflen)
	{
		INFO_("Update binary index key index {0x%x(%u)} block {0x%x(%u)} [in] buffer len {%d}", keyindex, keyindex, blockno, blockno, buflen);

		INFO_SIMPLE_(" => Rescanning card to avoid bad authentication");
		scanMifare();
		INFO_SIMPLE_("Scan done ! Continue to update binary index.");

		if (blockno != 0)
		{
			std::vector<unsigned char> command;
			command.push_back(static_cast<unsigned char>(d_lastKeyType));
			command.push_back(keyindex);
			command.push_back(blockno);
			command.insert(command.end(), (unsigned char*)buf, (unsigned char*)buf + buflen);

			unsigned char statusCode;
			getSTidSTRReaderCardAdapter()->sendCommand(0x00D3, command, statusCode);
		}

		INFO_("Returns final [out] buffer len {%d}", buflen);
		return buflen;
	}
}


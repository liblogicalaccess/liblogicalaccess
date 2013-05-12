/**
 * \file mifarepcsccommands.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Mifare PC/SC commands.
 */

#include "../commands/mifarepcsccommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifarechip.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

namespace logicalaccess
{	
	MifarePCSCCommands::MifarePCSCCommands()
		: MifareCommands()
	{
		
	}

	MifarePCSCCommands::~MifarePCSCCommands()
	{
		
	}

	bool MifarePCSCCommands::loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol)
	{
		bool r = false;

		unsigned char result[256];
		size_t resultlen = 256;

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x82, (vol ? 0x00 : 0x20), static_cast<char>(keyno), static_cast<unsigned char>(keylen), reinterpret_cast<const unsigned char*>(key), keylen, result, &resultlen);
		if (!vol && (result[resultlen - 2] == 0x63) && (result[resultlen - 1] == 0x86))
		{
			if (keyno == 0)
			{
				r = loadKey(keyno, keytype, key, keylen, true);
			}
		}
		else
		{
			r = true;
		}

		return r;
	}

	void MifarePCSCCommands::loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifareKeyType keytype)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(key, std::invalid_argument, "key cannot be null.");

		boost::shared_ptr<MifareLocation> mLocation = boost::dynamic_pointer_cast<MifareLocation>(location);
		boost::shared_ptr<MifareKey> mKey = boost::dynamic_pointer_cast<MifareKey>(key);

		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
		EXCEPTION_ASSERT_WITH_LOG(mKey, std::invalid_argument, "key must be a MifareKey.");

		boost::shared_ptr<KeyStorage> key_storage = key->getKeyStorage();

		if (boost::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
		{
			loadKey(0, keytype, key->getData(), key->getLength());
		}
		else if (boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
		{
			// Don't load the key when reader memory, except if specified
			if (!key->isEmpty())
			{
				boost::shared_ptr<ReaderMemoryKeyStorage> rmKs = boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
				loadKey(rmKs->getKeySlot(), keytype, key->getData(), key->getLength(), rmKs->getVolatile());
			}
		}
		else
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "The key storage type is not supported for this card/reader.");
		}
	}

	bool MifarePCSCCommands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
	{
		bool r = false;

		unsigned char command[2];
		size_t commandlen = sizeof(command);

		command[0] = static_cast<unsigned char>(keytype);
		command[1] = keyno;

		unsigned char result[256];
		size_t resultlen = 256;

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x88, 0x00, blockno, command, commandlen, result, &resultlen);
		r = true;

		return r;
	}

	void MifarePCSCCommands::authenticate(unsigned char blockno, boost::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype)
	{
		if (boost::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
		{
			authenticate(blockno, 0, keytype);
		}
		else if (boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
		{
			boost::shared_ptr<ReaderMemoryKeyStorage> rmKs = boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
			authenticate(blockno, rmKs->getKeySlot(), keytype);
		}
		else
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "The key storage type is not supported for this card/reader.");
		}
	}

	size_t MifarePCSCCommands::readBinary(unsigned char blockno, size_t len, void* buf, size_t buflen)
	{
		if ((len >= 256) || (len > buflen) || (!buf))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
		}

		size_t r = 0;

		unsigned char result[256];
		size_t resultlen = 256;

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xB0, 0x00, blockno, static_cast<unsigned char>(len), result, &resultlen);

		r = resultlen - 2;
		if (r > buflen)
		{
			r = buflen;
		}
		memcpy(buf, result, r);

		return r;
	}

	size_t MifarePCSCCommands::updateBinary(unsigned char blockno, const void* buf, size_t buflen)
	{
		if ((buflen >= 256) || (!buf))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
		}

		size_t r = 0;

		unsigned char result[256];
		size_t resultlen = sizeof(result);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD6, 0x00, blockno, static_cast<unsigned char>(buflen), reinterpret_cast<const unsigned char*>(buf), buflen, result, &resultlen);
		r = buflen;

		return r;
	}	
}


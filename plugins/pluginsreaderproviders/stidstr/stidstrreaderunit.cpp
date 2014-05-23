/**
 * \file stidstrreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR reader unit.
 */

#include "stidstrreaderunit.hpp"
#include "stidstrreaderprovider.hpp"


#include <iostream>
#include <iomanip>
#include <sstream>

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"

#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/stidstrreadercardadapter.hpp"
#include "stidstrledbuzzerdisplay.hpp"

#include "commands/desfireev1stidstrcommands.hpp"
#include "commands/mifarestidstrcommands.hpp"
#include "desfireprofile.hpp"
#include <boost/filesystem.hpp>
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "desfireev1chip.hpp"
#include "mifarechip.hpp"

namespace logicalaccess
{
	STidSTRReaderUnit::STidSTRReaderUnit()
		: ReaderUnit()
	{
		//LOG(LogLevel::INFOS) << "Constructor");
		d_readerUnitConfig.reset(new STidSTRReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<STidSTRReaderCardAdapter> (new STidSTRReaderCardAdapter(STID_CMD_READER)));
		d_ledBuzzerDisplay.reset(new STidSTRLEDBuzzerDisplay());
		boost::shared_ptr<SerialPortDataTransport> dataTransport(new SerialPortDataTransport());
#ifndef _WINDOWS
		dataTransport->setPortBaudRate(B38400);
#else
		dataTransport->setPortBaudRate(CBR_38400);
#endif
		setDataTransport(dataTransport);
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/STidSTRReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	STidSTRReaderUnit::~STidSTRReaderUnit()
	{
		//LOG(LogLevel::INFOS) << "Destructor");
		disconnectFromReader();
	}

	std::string STidSTRReaderUnit::getName() const
	{
		return getDataTransport()->getName();
	}

	std::string STidSTRReaderUnit::getConnectedName()
	{
		return getName();
	}

	void STidSTRReaderUnit::setCardType(std::string cardType)
	{
		LOG(LogLevel::INFOS) << "Setting card type {" << cardType << "}";
		d_card_type = cardType;
	}

	bool STidSTRReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool oldValue = Settings::getInstance()->IsLogEnabled;
		if (oldValue && !Settings::getInstance()->SeeWaitInsertionLog)
		{
			Settings::getInstance()->IsLogEnabled = false;		// Disable logs for this part (otherwise too much log output in file)
		}

		LOG(LogLevel::INFOS) << "Waiting insertion... max wait {" << maxwait << "}";
		bool inserted = false;
		unsigned int currentWait = 0;

		try
		{
			do
			{
				boost::shared_ptr<Chip> chip = scanARaw(); // scan14443A() => Obsolete. It's just used for testing purpose !
				if (!chip)
				{
					chip = scan14443B();
				}

				if (chip)
				{
					LOG(LogLevel::INFOS) << "Chip detected !";
					d_insertedChip = chip;
					inserted = true;
				}

				if (!inserted)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					currentWait += 100;
				}
			} while (!inserted && (maxwait == 0 || currentWait < maxwait));
		}
		catch (...)
		{
			Settings::getInstance()->IsLogEnabled = oldValue;
			throw;
		}

		LOG(LogLevel::INFOS) << "Returns card inserted ? {" << inserted << "} function timeout expired ? {" << (maxwait != 0 && currentWait >= maxwait) << "}";
		Settings::getInstance()->IsLogEnabled = oldValue;

		return inserted;
	}

	bool STidSTRReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool oldValue = Settings::getInstance()->IsLogEnabled;
		if (oldValue && !Settings::getInstance()->SeeWaitRemovalLog)
		{
			Settings::getInstance()->IsLogEnabled = false;		// Disable logs for this part (otherwise too much log output in file)
		}

		LOG(LogLevel::INFOS) << "Waiting removal... max wait {" << maxwait << "}";
		bool removed = false;
		unsigned int currentWait = 0;
		try
		{
			if (d_insertedChip)
			{
				do
				{
					boost::shared_ptr<Chip> chip = scanARaw(); // scan14443A() => Obsolete. It's just used for testing purpose !
					if (!chip)
					{
						chip = scan14443B();
					}

					if (chip)
					{
						if (chip->getChipIdentifier() != d_insertedChip->getChipIdentifier())
						{
							LOG(LogLevel::INFOS) << "Card found but not same chip ! The previous card has been removed !";
							d_insertedChip.reset();
							removed = true;
						}
					}
					else
					{
						LOG(LogLevel::INFOS) << "Card removed !";
						d_insertedChip.reset();
						removed = true;
					}

					if (!removed)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
						currentWait += 100;
					}
				} while (!removed && (maxwait == 0 || currentWait < maxwait));
			}
		}
		catch (...)
		{
			Settings::getInstance()->IsLogEnabled = oldValue;
			throw;
		}

		LOG(LogLevel::INFOS) << "Returns card removed ? {" << removed << "} - function timeout expired ? {" << (maxwait != 0 && currentWait >= maxwait) << "}";

		Settings::getInstance()->IsLogEnabled = oldValue;

		return removed;
	}

	bool STidSTRReaderUnit::connect()
	{
		LOG(LogLevel::WARNINGS) << "Connect do nothing with STid STR reader";
		return true;
	}

	void STidSTRReaderUnit::disconnect()
	{
		LOG(LogLevel::WARNINGS) << "Disconnect do nothing with STid STR reader";
	}

	bool STidSTRReaderUnit::connectToReader()
	{
		bool ret = getDataTransport()->connect();
		if (ret)
		{
			// Negotiate sessions according to communication options
			if ((getSTidSTRConfiguration()->getCommunicationMode() & STID_CM_SIGNED) == STID_CM_SIGNED)
			{
				LOG(LogLevel::INFOS) << "Signed communication required, negotiating HMAC...";
				authenticateHMAC();
			}
			if ((getSTidSTRConfiguration()->getCommunicationMode() & STID_CM_CIPHERED) == STID_CM_CIPHERED)
			{
				LOG(LogLevel::INFOS) << "Ciphered communication required, negotiating AES...";
				authenticateAES();
			}
		}

		return ret;
	}

	void STidSTRReaderUnit::disconnectFromReader()
	{
		getDataTransport()->disconnect();
	}

	std::vector<unsigned char> STidSTRReaderUnit::getPingCommand() const
	{
		std::vector<unsigned char> buffer;

		buffer.push_back(0x00);
		buffer.push_back(0x08);

		return buffer;
	}

	boost::shared_ptr<Chip> STidSTRReaderUnit::createChip(std::string type)
	{
		LOG(LogLevel::INFOS) << "Creating chip... chip type {" << type << "}";
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			LOG(LogLevel::INFOS) << "Chip created successfully !";
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<Commands> commands;

			if (type == "Mifare1K" || type == "Mifare4K" || type == "Mifare")
			{
				LOG(LogLevel::INFOS) << "Mifare classic Chip created";
				rca.reset(new STidSTRReaderCardAdapter(STID_CMD_MIFARE_CLASSIC));
				commands.reset(new MifareSTidSTRCommands());
			}
			else if (type == "DESFire" || type == "DESFireEV1")
			{
				LOG(LogLevel::INFOS) << "Mifare DESFire Chip created";
				rca.reset(new STidSTRReaderCardAdapter(STID_CMD_DESFIRE));
				commands.reset(new DESFireEV1STidSTRCommands());
				boost::dynamic_pointer_cast<DESFireEV1STidSTRCommands>(commands)->setProfile(boost::dynamic_pointer_cast<DESFireProfile>(chip->getProfile()));
			}
			else
				return chip;

			rca->setDataTransport(getDataTransport());
			if (commands)
			{
				commands->setReaderCardAdapter(rca);
				commands->setChip(chip);
				chip->setCommands(commands);
			}
		}
		else
		{
			LOG(LogLevel::ERRORS) << "Unable to create the chip with this type !";
		}
		return chip;
	}

	boost::shared_ptr<Chip> STidSTRReaderUnit::getSingleChip()
	{
		//LOG(LogLevel::INFOS) << "Getting the first detect chip...");
		boost::shared_ptr<Chip> chip = d_insertedChip;

		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > STidSTRReaderUnit::getChipList()
	{
		//LOG(LogLevel::INFOS) << "Getting all detected chips...");
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<STidSTRReaderCardAdapter> STidSTRReaderUnit::getDefaultSTidSTRReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<STidSTRReaderCardAdapter>(adapter);
	}

	string STidSTRReaderUnit::getReaderSerialNumber()
	{
		LOG(LogLevel::WARNINGS) << "Do nothing with STid STR reader";
		string ret;
		return ret;
	}

	bool STidSTRReaderUnit::isConnected()
	{
		if (d_insertedChip)
			LOG(LogLevel::INFOS) << "Is connected {1}";
		else
			LOG(LogLevel::INFOS) << "Is connected {0}";
		return bool(d_insertedChip);
	}
	
	void STidSTRReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		ReaderUnit::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void STidSTRReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		ReaderUnit::unSerialize(node);
	}

	boost::shared_ptr<STidSTRReaderProvider> STidSTRReaderUnit::getSTidSTRReaderProvider() const
	{
		return boost::dynamic_pointer_cast<STidSTRReaderProvider>(getReaderProvider());
	}

	boost::shared_ptr<Chip> STidSTRReaderUnit::scan14443A()
	{
		LOG(LogLevel::INFOS) << "Scanning 14443A chips...";
		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> response = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0002, std::vector<unsigned char>());
		if (response.size() > 0)
		{
			bool haveCard = (response[0] == 0x01);
			if (haveCard && response.size() > 4)
			{
				STidCmdType stidCardType = static_cast<STidCmdType>(response[1]);

				LOG(LogLevel::INFOS) << "Response detected stid card type {" << stidCardType << "}";

				//unsigned char cardSize = response.constData<unsigned char>()[2];
				//unsigned char chipInfo = response.constData<unsigned char>()[3];
				unsigned char uidLen = response[4];

				LOG(LogLevel::INFOS) << "UID length {" << uidLen << "}";

				if (uidLen > 0)
				{
					std::vector<unsigned char> uid = std::vector<unsigned char>(response.begin() + 5, response.begin() + 5 + uidLen);

					LOG(LogLevel::INFOS) << "UID " << BufferHelper::getHex(uid) << "-{" << BufferHelper::getStdString(uid) << "}";
					std::string cardType = "UNKNOWN";
					switch (stidCardType)
					{
					case STID_CMD_MIFARE_CLASSIC:
						LOG(LogLevel::INFOS) << "Mifare classic type !";
						cardType = "Mifare";
						break;

					case STID_CMD_DESFIRE:
						LOG(LogLevel::INFOS) << "Mifare DESFire type !";
						cardType = "DESFireEV1";
						break;

					default:
						LOG(LogLevel::INFOS) << "Unknown type !";
						cardType = "UNKNOWN";
						break;
					}

					chip = createChip(cardType);
					chip->setChipIdentifier(uid);

					LOG(LogLevel::INFOS) << "Scanning for specific chip type, mandatory for STid reader...";
					// Scan for specific chip type, mandatory for STid reader...
					if (cardType == "DESFire" || cardType == "DESFireEV1")
					{
						boost::shared_ptr<DESFireEV1Chip> dchip = boost::dynamic_pointer_cast<DESFireEV1Chip>(chip);
						boost::dynamic_pointer_cast<DESFireEV1STidSTRCommands>(dchip->getDESFireEV1Commands())->scanDESFire();
					}
					else if (cardType == "Mifare" || cardType == "Mifare1K" || cardType == "Mifare4K")
					{
						boost::shared_ptr<MifareChip> mchip = boost::dynamic_pointer_cast<MifareChip>(chip);
						boost::dynamic_pointer_cast<MifareSTidSTRCommands>(mchip->getMifareCommands())->scanMifare();
					}
				}
				else
				{
					LOG(LogLevel::ERRORS) << "No UID retrieved !";
				}
			}
			else if (!haveCard)
			{
				LOG(LogLevel::INFOS) << "No card detected !";
			}
			else
			{
				LOG(LogLevel::ERRORS) << "Command length should be > 4";
			}
		}
		else
		{
			LOG(LogLevel::ERRORS) << "No response !";
		}

		return chip;
	}

	boost::shared_ptr<Chip> STidSTRReaderUnit::scanARaw()
	{
		LOG(LogLevel::INFOS) << "Scanning 14443A RAW chips...";
		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> command;
		command.push_back(0x00);

		std::vector<unsigned char> response = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000F, command);
		if (response.size() > 0)
		{
			bool haveCard = (response[0] == 0x01);
			if (haveCard && response.size() > 5)
			{
				unsigned short atqa = (response[1] << 8) | response[2];
				LOG(LogLevel::INFOS) << "ATQA (Answer To Request Type A) value {0x" << std::hex << atqa << std::dec << "(" << atqa << ")}";

				// Voir documentation AN10833 de NXP
				std::string cardType = "UNKNOWN";
				switch (atqa)
				{
				case 0x002:
					LOG(LogLevel::INFOS) << "Mifare classic 4K !";
					cardType = "Mifare4K";
					break;
				case 0x004:
					LOG(LogLevel::INFOS) << "Mifare classic 1K !"; // Ou Mifare Mini
					cardType = "Mifare1K";
					break;
				case 0x0042:
				case 0x0044:
					LOG(LogLevel::INFOS) << "Mifare Ultralight";
					cardType = "MifareUltralight";
					break;
				case 0x0344:
					LOG(LogLevel::INFOS) << "Mifare DESFire type !";
					cardType = "DESFireEV1";
					break;
				default:
					LOG(LogLevel::INFOS) << "Unknown type !";
					cardType = "UNKNOWN";
					break;
				}

				//unsigned char sak = response[3];
				//LOG(LogLevel::INFOS) << "SAK (Select Acknowloedge Type A) value {0x%x(%u)}", sak);


				unsigned char uidLen = response[4];
				LOG(LogLevel::INFOS) << "UID length {" << uidLen << "}";

				if (uidLen > 0)
				{
					std::vector<unsigned char> uid = std::vector<unsigned char>(response.begin() + 5, response.begin() + 5 + uidLen);

					LOG(LogLevel::INFOS) << "UID " << BufferHelper::getHex(uid) << "-{" << BufferHelper::getStdString(uid) << "}";

					chip = createChip(cardType);
					chip->setChipIdentifier(uid);

					LOG(LogLevel::INFOS) << "Scanning for specific chip type, mandatory for STid reader...";
					// Scan for specific chip type, mandatory for STid reader...
					if (cardType == "DESFire" || cardType == "DESFireEV1")
					{
						boost::shared_ptr<DESFireEV1Commands> chipcmd = boost::dynamic_pointer_cast<DESFireEV1Commands>(chip->getCommands());
						boost::dynamic_pointer_cast<DESFireEV1STidSTRCommands>(chipcmd)->scanDESFire();
					}
					else if (cardType == "Mifare" || cardType == "Mifare1K" || cardType == "Mifare4K")
					{
						boost::shared_ptr<MifareCommands> chipcmd = boost::dynamic_pointer_cast<MifareCommands>(chip->getCommands());
						boost::dynamic_pointer_cast<MifareSTidSTRCommands>(chipcmd)->scanMifare();
					}
				}
				else
				{
					LOG(LogLevel::ERRORS) << "No UID retrieved !";
				}
			}
			else if (!haveCard)
			{
				LOG(LogLevel::INFOS) << "No card detected !";
			}
			else
			{
				LOG(LogLevel::ERRORS) << "Command length should be > 5";
			}
		}
		else
		{
			LOG(LogLevel::ERRORS) << "No response !";
		}

		return chip;
	}

	boost::shared_ptr<Chip> STidSTRReaderUnit::scan14443B()
	{
		LOG(LogLevel::INFOS) << "Scanning 14443B chips...";
		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> response = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0009, std::vector<unsigned char>());
		if (response.size() > 0)
		{
			bool haveCard = (response[0] == 0x01);
			if (haveCard && response.size() > 4)
			{
				unsigned char uidLen = response[1];
				if (uidLen > 0)
				{
					std::vector<unsigned char> uid = std::vector<unsigned char>(response.begin() + 2, response.begin() + 5 + uidLen);

					LOG(LogLevel::INFOS) << "UID length {" << uidLen << "}";
					chip = createChip("UNKNOWN");
					chip->setChipIdentifier(uid);
				}
				else
				{
					LOG(LogLevel::ERRORS) << "No UID retrieved !";
				}
			}
			else if (!haveCard)
			{
				LOG(LogLevel::INFOS) << "No card detected !";
			}
			else
			{
				LOG(LogLevel::ERRORS) << "Command length should be > 4";
			}
		}
		else
		{
			LOG(LogLevel::ERRORS) << "No response !";
		}

		return chip;
	}

	void STidSTRReaderUnit::authenticateHMAC()
	{
		LOG(LogLevel::INFOS) << "Authenticating HMAC (signed communication)...";

		RAND_seed(getDataTransport().get(), sizeof(getDataTransport().get()));
		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

		std::vector<unsigned char> buf1;
		boost::shared_ptr<HMAC1Key> key = getSTidSTRConfiguration()->getHMACKey();
		if (key->isEmpty())
		{
			LOG(LogLevel::INFOS) << "Empty key... using the default one !";
			key.reset(new HMAC1Key("A0 87 75 4B 75 47 48 10 94 BE"));
		}

		unsigned int len = 20;
		std::vector<unsigned char> key16ks;
		key16ks.resize(len, 0x00);

		std::vector<unsigned char> rndB;
		rndB.resize(16);
		if (RAND_bytes(&rndB[0], static_cast<int>(rndB.size())) != 1)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
		}

		openssl::AESInitializationVector aesiv = openssl::AESInitializationVector::createNull();
		openssl::AESCipher cipher;

		std::vector<unsigned char> buf2, encbuf2;

		/* IF THE TIME BETWEEN authenticate 1 and 2 is TOO HIGH, WE RECEIVE AN ERROR INCORRECT DATA */
		/* BY DECLARING EVERYTHING BEFORE authenticate, IT SEEMS TO BE ENOUGH TO WORK FOR COM COMPONENTS */

		std::vector<unsigned char> rndA = authenticateReader1(true);
		unsigned char* keydata = key->getData();
		buf1.insert(buf1.end(), keydata, keydata + key->getLength());
		buf1.insert(buf1.end(), rndA.begin(), rndA.end());

		HMAC(EVP_sha1(), key->getData(), static_cast<int>(key->getLength()), &buf1[0], buf1.size(), &key16ks[0], &len);
		key16ks.resize(key16ks.size() - 4);
		openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(key16ks);

		buf2.insert(buf2.end(), rndA.begin(), rndA.end());
		buf2.insert(buf2.end(), rndB.begin(), rndB.end());
		cipher.cipher(buf2, encbuf2, aeskey, aesiv, false);

		std::vector<unsigned char> encbuf3 = authenticateReader2(encbuf2);

		std::vector<unsigned char> buf3;
		cipher.decipher(encbuf3, buf3, aeskey, aesiv, false);
		EXCEPTION_ASSERT_WITH_LOG(buf3.size()  == 32, LibLogicalAccessException, "Bad second reader response length.");

		std::vector<unsigned char> rndB2 = std::vector<unsigned char>(buf3.begin(), buf3.begin() + 16);
		EXCEPTION_ASSERT_WITH_LOG(rndB2  == rndB, LibLogicalAccessException, "Cannot negotiate the session: RndB'' != RndB.");
		std::vector<unsigned char> rndC= std::vector<unsigned char>(buf3.begin() + 16, buf3.begin() + 16 + 16);

		d_sessionKey_hmac.clear();
		d_sessionKey_hmac.push_back(rndB[0]);
		d_sessionKey_hmac.push_back(rndB[1]);
		d_sessionKey_hmac.push_back(rndB[2]);
		d_sessionKey_hmac.push_back(rndC[0]);
		d_sessionKey_hmac.push_back(rndC[1]);
		d_sessionKey_hmac.push_back(rndB[14]);
		d_sessionKey_hmac.push_back(rndB[15]);
		d_sessionKey_hmac.push_back(rndC[13]);
		d_sessionKey_hmac.push_back(rndC[14]);
		d_sessionKey_hmac.push_back(rndC[15]);
	}

	void STidSTRReaderUnit::authenticateAES()
	{
		LOG(LogLevel::INFOS) << "Authenticating AES (ciphered communication)...";

		RAND_seed(getDataTransport().get(), sizeof(getDataTransport().get()));
		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

		std::vector<unsigned char> rndB;
		rndB.resize(16);
		if (RAND_bytes(&rndB[0], static_cast<int>(rndB.size())) != 1)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
		}

		boost::shared_ptr<AES128Key> key = getSTidSTRConfiguration()->getAESKey();
		if (key->isEmpty())
		{
			LOG(LogLevel::INFOS) << "Empty key... using the default one !";
			key.reset(new AES128Key("E7 4A 54 0F A0 7C 4D B1 B4 64 21 12 6D F7 AD 36"));
		}

		unsigned char* keydata = key->getData();
		openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(std::vector<unsigned char>(keydata, keydata + key->getLength()));
		openssl::AESInitializationVector aesiv = openssl::AESInitializationVector::createNull();
		openssl::AESCipher cipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_CBC);

		std::vector<unsigned char> rndA;
		std::vector<unsigned char> buf1, encbuf1;
		std::vector<unsigned char> rndB2;

		/* IF THE TIME BETWEEN authenticate 1 and 2 is TOO HIGH, WE RECEIVE AN ERROR INCORRECT DATA */
		/* BY DECLARING EVERYTHING BEFORE authenticate, IT SEEMS TO BE ENOUGH TO WORK FOR COM COMPONENTS */

		std::vector<unsigned char> encrndA = authenticateReader1(false);		
		cipher.decipher(encrndA, rndA, aeskey, aesiv, false);
		buf1.insert(buf1.end(), rndA.begin(), rndA.end());
		buf1.insert(buf1.end(), rndB.begin(), rndB.end());
		cipher.cipher(buf1, encbuf1, aeskey, aesiv, false);

		std::vector<unsigned char> encrndB2 = authenticateReader2(encbuf1);

		cipher.decipher(encrndB2, rndB2, aeskey, aesiv, false);
		EXCEPTION_ASSERT_WITH_LOG(rndB2  == rndB, LibLogicalAccessException, "Cannot negotiate the session: RndB'' != RndB.");

		d_sessionKey_aes.clear();
		d_sessionKey_aes.push_back(rndA[0]);
		d_sessionKey_aes.push_back(rndA[1]);
		d_sessionKey_aes.push_back(rndA[2]);
		d_sessionKey_aes.push_back(rndA[3]);
		d_sessionKey_aes.push_back(rndB[0]);
		d_sessionKey_aes.push_back(rndB[1]);
		d_sessionKey_aes.push_back(rndB[2]);
		d_sessionKey_aes.push_back(rndB[3]);		
		d_sessionKey_aes.push_back(rndA[12]);
		d_sessionKey_aes.push_back(rndA[13]);
		d_sessionKey_aes.push_back(rndA[14]);
		d_sessionKey_aes.push_back(rndA[15]);
		d_sessionKey_aes.push_back(rndB[12]);
		d_sessionKey_aes.push_back(rndB[13]);
		d_sessionKey_aes.push_back(rndB[14]);
		d_sessionKey_aes.push_back(rndB[15]);
	}

	std::vector<unsigned char> STidSTRReaderUnit::authenticateReader1(bool isHMAC)
	{
		LOG(LogLevel::INFOS) << "Authenticating Reader 1... is HMAC {" << isHMAC << "}";
		std::vector<unsigned char> command, ret;
		command.push_back(isHMAC ? 0x01 : 0x02);

		STidCommunicationMode lastCM = getSTidSTRConfiguration()->getCommunicationMode();
		getSTidSTRConfiguration()->setCommunicationMode(STID_CM_RESERVED);
		try
		{
			ret = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0001, command);
		}
		catch(std::exception& e)
		{
			LOG(LogLevel::ERRORS) << "Exception {" << e.what() << "}";
			getSTidSTRConfiguration()->setCommunicationMode(lastCM);
			throw;
		}
		getSTidSTRConfiguration()->setCommunicationMode(lastCM);

		return ret;
	}

	std::vector<unsigned char> STidSTRReaderUnit::authenticateReader2(const std::vector<unsigned char>& data)
	{
		LOG(LogLevel::INFOS) << "Authenticating Reader 2...";
		std::vector<unsigned char> ret;
		STidCommunicationMode lastCM = getSTidSTRConfiguration()->getCommunicationMode();
		getSTidSTRConfiguration()->setCommunicationMode(STID_CM_RESERVED);
		try
		{
			ret = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0001, data);
		}
		catch(std::exception& e)
		{
			LOG(LogLevel::ERRORS) << "Exception {" << e.what() << "}";
			getSTidSTRConfiguration()->setCommunicationMode(lastCM);
			throw;
		}
		getSTidSTRConfiguration()->setCommunicationMode(lastCM);

		return ret;
	}

	void STidSTRReaderUnit::ResetAuthenticate()
	{
		LOG(LogLevel::INFOS) << "Reseting authentication...";
		getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000D, std::vector<unsigned char>());
	}

	void STidSTRReaderUnit::ChangeReaderKeys(const std::vector<unsigned char>& key_hmac, const std::vector<unsigned char>& key_aes)
	{
		LOG(LogLevel::INFOS) << "Changing reader keys...";
		std::vector<unsigned char> command;
		unsigned char keyMode = 0x00;
		if (key_hmac.size() > 0)
		{
			EXCEPTION_ASSERT_WITH_LOG(key_hmac.size() == 10, LibLogicalAccessException, "The authentication HMAC key must be 10-byte long.");
			keyMode |= 0x01;
			command.insert(command.end(), key_hmac.begin(), key_hmac.end());
		}
		if (key_aes.size() > 0)
		{
			EXCEPTION_ASSERT_WITH_LOG(key_aes.size() == 16, LibLogicalAccessException, "The enciphering AES key must be 16-byte long.");
			keyMode |= 0x02;
			command.insert(command.end(), key_aes.begin(), key_aes.end());
		}
		command.insert(command.begin(), &keyMode, &keyMode + 1);

		getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0003, command);
	}

	void STidSTRReaderUnit::setBaudRate(STidBaudrate baudrate)
	{
		LOG(LogLevel::INFOS) << "Setting baudrate... baudrate {0x" << std::hex << baudrate << std::dec << "(" << baudrate << ")}";
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(baudrate));
		getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0005, command);
	}

	void STidSTRReaderUnit::set485Address(unsigned char address)
	{
		LOG(LogLevel::INFOS) << "Setting RS485 address... address {0x" << std::hex << address << std::dec << "(" << address << ")}";
		std::vector<unsigned char> command;
		EXCEPTION_ASSERT_WITH_LOG(address <= 127, LibLogicalAccessException, "The RS485 address should be between 0 and 127.");
		command.push_back(address);
		getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0006, command);
	}

	STidSTRReaderUnit::STidSTRInformation STidSTRReaderUnit::getReaderInformaton()
	{
		LOG(LogLevel::INFOS) << "Getting reader information...";
		std::vector<unsigned char> response = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0008, std::vector<unsigned char>());

		EXCEPTION_ASSERT_WITH_LOG(response.size() >= 5, LibLogicalAccessException, "The GetInfos response should be 5-byte long.");

		STidSTRInformation readerInfo;
		readerInfo.version = response[0];
		readerInfo.baudrate = static_cast<STidBaudrate>(response[1]);
		readerInfo.rs485Address = response[2];
		readerInfo.voltage = (float)response[3] + ((float)response[4] / (float)10);

		LOG(LogLevel::INFOS) << "Returns version {0x" << std::hex << readerInfo.version << std::dec << "(" << readerInfo.version << ")}"
			<< "baudrate {0x" << std::hex << readerInfo.baudrate << std::dec << "(" << readerInfo.baudrate << ")}"
			<< "rs485 address {0x" << std::hex << readerInfo.rs485Address << std::dec << "(" << readerInfo.rs485Address << ")}"
			<< "voltage {0x" << std::hex << readerInfo.voltage << std::dec << "(" << readerInfo.voltage << ")}";

		return readerInfo;
	}

	void STidSTRReaderUnit::setAllowedCommModes(bool plainComm, bool signedComm, bool cipheredComm)
	{
		LOG(LogLevel::INFOS) << "Setting allowed communication modes... plain comm {" << plainComm << "} signed comm {" << signedComm << "} ciphered comm {" << cipheredComm << "}";
		std::vector<unsigned char> command;
		unsigned char allowedModes = 0x08 |
			((plainComm) ? 0x01 : 0x00) |
			((signedComm) ? 0x02 : 0x00) |
			((cipheredComm) ? 0x04 : 0x00);

		command.push_back(allowedModes);
		getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000A, command);
	}

	void STidSTRReaderUnit::setTamperSwitchSettings(bool useTamperSwitch, STidTamperSwitchBehavior behavior)
	{
		LOG(LogLevel::INFOS) << "Setting tamper switch settings... use tamper {" << useTamperSwitch << "} tamper behavior {0x" << std::hex << behavior << std::dec << "(" << behavior << ")}";
		std::vector<unsigned char> command;

		command.push_back((useTamperSwitch) ? 0x01 : 0x00);
		command.push_back(static_cast<unsigned char>(behavior));

		getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000B, command);
	}

	void STidSTRReaderUnit::getTamperSwitchInfos(bool& useTamperSwitch, STidTamperSwitchBehavior& behavior, bool& swChanged)
	{
		LOG(LogLevel::INFOS) << "Getting tamper switch infos...";
		std::vector<unsigned char> response = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000C, std::vector<unsigned char>());

		EXCEPTION_ASSERT_WITH_LOG(response.size() >= 3, LibLogicalAccessException, "The GetTamperSwitchInfos response should be 3-byte long.");

		useTamperSwitch = (response[0] == 0x01);
		behavior = static_cast<STidTamperSwitchBehavior>(response[1]);
		swChanged = (response[2] == 0x01);

		LOG(LogLevel::INFOS) << "Returns use tamper {" << useTamperSwitch << "} tamper behavior {0x" << std::hex << behavior << std::dec << "(" << behavior << ")}";
	}

	void STidSTRReaderUnit::loadSKB()
	{
		LOG(LogLevel::INFOS) << "Loading SKB...";
		unsigned char statusCode = 0;
		std::vector<unsigned char> response = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000E, std::vector<unsigned char>(), statusCode);

		EXCEPTION_ASSERT_WITH_LOG(response.size() == 0, LibLogicalAccessException, "Unable to load the SKB values. An unknown error occured.");
	}
}

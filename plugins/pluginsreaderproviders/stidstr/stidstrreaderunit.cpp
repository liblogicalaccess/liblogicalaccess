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
		//INFO_SIMPLE_("Constructor");
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
		//INFO_SIMPLE_("Destructor");
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
		INFO_("Setting card type {0x%s(%s)}", cardType.c_str(), cardType.c_str());
		d_card_type = cardType;
	}

	bool STidSTRReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool oldValue = Settings::getInstance()->IsLogEnabled;
		if (oldValue && !Settings::getInstance()->SeeWaitInsertionLog)
		{
			Settings::getInstance()->IsLogEnabled = false;		// Disable logs for this part (otherwise too much log output in file)
		}

		INFO_("Waiting insertion... max wait {%u}", maxwait);
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
					INFO_SIMPLE_("Chip detected !");
					d_insertedChip = chip;
					inserted = true;
				}

				if (!inserted)
				{
#ifdef _WINDOWS
					Sleep(100);
#elif defined(__unix__)
					usleep(100000);
#endif
					currentWait += 100;
				}
			} while (!inserted && (maxwait == 0 || currentWait < maxwait));
		}
		catch(...)
		{
			Settings::getInstance()->IsLogEnabled = oldValue;
			throw;
		}

		INFO_("Returns card inserted ? {%d} function timeout expired ? {%d}", inserted, (maxwait != 0 && currentWait >= maxwait));
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

		INFO_("Waiting removal... max wait {%u}", maxwait);
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
							INFO_SIMPLE_("Card found but not same chip ! The previous card has been removed !");
							d_insertedChip.reset();
							removed = true;
						}
					}
					else
					{
						INFO_SIMPLE_("Card removed !");
						d_insertedChip.reset();
						removed = true;
					}

					if (!removed)
					{
#ifdef _WINDOWS
						Sleep(100);
#elif defined(__unix__)
						usleep(100000);
#endif
						currentWait += 100;
					}
				} while (!removed && (maxwait == 0 || currentWait < maxwait));
			}
		}
		catch(...)
		{
			Settings::getInstance()->IsLogEnabled = oldValue;
			throw;
		}

		INFO_("Returns card removed ? {%d} - function timeout expired ? {%d}", removed, (maxwait != 0 && currentWait >= maxwait));

		Settings::getInstance()->IsLogEnabled = oldValue;

		return removed;
	}

	bool STidSTRReaderUnit::connect()
	{
		WARNING_SIMPLE_("Connect do nothing with STid STR reader");
		return true;
	}

	void STidSTRReaderUnit::disconnect()
	{
		WARNING_SIMPLE_("Disconnect do nothing with STid STR reader");
	}

	bool STidSTRReaderUnit::connectToReader()
	{
		bool ret = getDataTransport()->connect();
		if (ret)
		{
			// Negotiate sessions according to communication options
			if ((getSTidSTRConfiguration()->getCommunicationMode() & STID_CM_SIGNED) == STID_CM_SIGNED)
			{
				INFO_SIMPLE_("Signed communication required, negotiating HMAC...");
				authenticateHMAC();
			}
			if ((getSTidSTRConfiguration()->getCommunicationMode() & STID_CM_CIPHERED) == STID_CM_CIPHERED)
			{
				INFO_SIMPLE_("Ciphered communication required, negotiating AES...");
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
		INFO_("Creating chip... chip type {%s}", type.c_str());
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			INFO_SIMPLE_("Chip created successfully !");
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<Commands> commands;

			if (type == "Mifare1K" || type == "Mifare4K" || type == "Mifare")
			{
				INFO_SIMPLE_("Mifare classic Chip created");
				rca.reset(new STidSTRReaderCardAdapter(STID_CMD_MIFARE_CLASSIC));
				commands.reset(new MifareSTidSTRCommands());
			}
			else if (type == "DESFire" || type == "DESFireEV1")
			{
				INFO_SIMPLE_("Mifare DESFire Chip created");
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
			ERROR_SIMPLE_("Unable to create the chip with this type !");
		}
		return chip;
	}

	boost::shared_ptr<Chip> STidSTRReaderUnit::getSingleChip()
	{
		//INFO_SIMPLE_("Getting the first detect chip...");
		boost::shared_ptr<Chip> chip = d_insertedChip;

		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > STidSTRReaderUnit::getChipList()
	{
		//INFO_SIMPLE_("Getting all detected chips...");
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
		WARNING_SIMPLE_("Do nothing with STid STR reader");
		string ret;
		return ret;
	}

	bool STidSTRReaderUnit::isConnected()
	{
		if (d_insertedChip)
			INFO_SIMPLE_("Is connected {1}");
		else
			INFO_SIMPLE_("Is connected {0}");
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
		INFO_SIMPLE_("Scanning 14443A chips...");
		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> response = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0002, std::vector<unsigned char>());
		if (response.size() > 0)
		{
			bool haveCard = (response[0] == 0x01);
			if (haveCard && response.size() > 4)
			{
				STidCmdType stidCardType = static_cast<STidCmdType>(response[1]);

				INFO_("Response detected stid card type {0x%x(%d)}", stidCardType, stidCardType);

				//unsigned char cardSize = response.constData<unsigned char>()[2];
				//unsigned char chipInfo = response.constData<unsigned char>()[3];
				unsigned char uidLen = response[4];

				INFO_("UID length {%u}", uidLen);

				if (uidLen > 0)
				{
					std::vector<unsigned char> uid = std::vector<unsigned char>(response.begin() + 5, response.begin() + 5 + uidLen);

					INFO_("UID %s-{%s}", BufferHelper::getHex(uid).c_str(), BufferHelper::getStdString(uid).c_str());
					std::string cardType = "UNKNOWN";
					switch (stidCardType)
					{
					case STID_CMD_MIFARE_CLASSIC:
						INFO_SIMPLE_("Mifare classic type !");
						cardType = "Mifare";
						break;

					case STID_CMD_DESFIRE:
						INFO_SIMPLE_("Mifare DESFire type !");
						cardType = "DESFireEV1";
						break;

					default:
						INFO_SIMPLE_("Unknown type !");
						cardType = "UNKNOWN";
						break;
					}

					chip = createChip(cardType);
					chip->setChipIdentifier(uid);

					INFO_SIMPLE_("Scanning for specific chip type, mandatory for STid reader...");
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
					ERROR_SIMPLE_("No UID retrieved !");
				}
			}
			else if (!haveCard)
			{
				INFO_SIMPLE_("No card detected !");
			}
			else
			{
				ERROR_SIMPLE_("Command length should be > 4");
			}
		}
		else
		{
			ERROR_SIMPLE_("No response !");
		}

		return chip;
	}

	boost::shared_ptr<Chip> STidSTRReaderUnit::scanARaw()
	{
		INFO_SIMPLE_("Scanning 14443A RAW chips...");
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
				INFO_("ATQA (Answer To Request Type A) value {0x%x(%u)}", atqa, atqa);

				// Voir documentation AN10833 de NXP
				std::string cardType = "UNKNOWN";
				switch (atqa)
				{
				case 0x002:
					INFO_SIMPLE_("Mifare classic 4K !");
					cardType = "Mifare4K";
					break;
				case 0x004:
					INFO_SIMPLE_("Mifare classic 1K !"); // Ou Mifare Mini
					cardType = "Mifare1K";
					break;
				case 0x0042:
				case 0x0044:
					INFO_SIMPLE_("Mifare Ultralight");
					cardType = "MifareUltralight";
				case 0x0344:
					INFO_SIMPLE_("Mifare DESFire type !");
					cardType = "DESFireEV1";
					break;
				default:
					INFO_SIMPLE_("Unknown type !");
					cardType = "UNKNOWN";
					break;
				}

				//unsigned char sak = response[3];
				//INFO_("SAK (Select Acknowloedge Type A) value {0x%x(%u)}", sak);


				unsigned char uidLen = response[4];
				INFO_("UID length {%u}", uidLen);

				if (uidLen > 0)
				{
					std::vector<unsigned char> uid = std::vector<unsigned char>(response.begin() + 5, response.begin() + 5 + uidLen);

					INFO_("UID %s-{%s}", BufferHelper::getHex(uid).c_str(), BufferHelper::getStdString(uid).c_str());

					chip = createChip(cardType);
					chip->setChipIdentifier(uid);

					INFO_SIMPLE_("Scanning for specific chip type, mandatory for STid reader...");
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
					ERROR_SIMPLE_("No UID retrieved !");
				}
			}
			else if (!haveCard)
			{
				INFO_SIMPLE_("No card detected !");
			}
			else
			{
				ERROR_SIMPLE_("Command length should be > 5");
			}
		}
		else
		{
			ERROR_SIMPLE_("No response !");
		}

		return chip;
	}

	boost::shared_ptr<Chip> STidSTRReaderUnit::scan14443B()
	{
		INFO_SIMPLE_("Scanning 14443B chips...");
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

					INFO_("UID length {%u}", uidLen);
					chip = createChip("UNKNOWN");
					chip->setChipIdentifier(uid);
				}
				else
				{
					ERROR_SIMPLE_("No UID retrieved !");
				}
			}
			else if (!haveCard)
			{
				INFO_SIMPLE_("No card detected !");
			}
			else
			{
				ERROR_SIMPLE_("Command length should be > 4");
			}
		}
		else
		{
			ERROR_SIMPLE_("No response !");
		}

		return chip;
	}

	void STidSTRReaderUnit::authenticateHMAC()
	{
		INFO_SIMPLE_("Authenticating HMAC (signed communication)...");

		RAND_seed(getDataTransport().get(), sizeof(getDataTransport().get()));
		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

		std::vector<unsigned char> buf1;
		boost::shared_ptr<HMAC1Key> key = getSTidSTRConfiguration()->getHMACKey();
		if (key->isEmpty())
		{
			INFO_SIMPLE_("Empty key... using the default one !");
			key.reset(new HMAC1Key("A0 87 75 4B 75 47 48 10 94 BE"));
		}

		unsigned int len = 20;
		std::vector<unsigned char> key16ks;
		key16ks.resize(len, 0x00);

		std::vector<unsigned char> rndB;
		rndB.resize(16);
		if (RAND_bytes(&rndB[0], static_cast<int>(rndB.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
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
		INFO_SIMPLE_("Authenticating AES (ciphered communication)...");

		RAND_seed(getDataTransport().get(), sizeof(getDataTransport().get()));
		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

		std::vector<unsigned char> rndB;
		rndB.resize(16);
		if (RAND_bytes(&rndB[0], static_cast<int>(rndB.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
		}

		boost::shared_ptr<AES128Key> key = getSTidSTRConfiguration()->getAESKey();
		if (key->isEmpty())
		{
			INFO_SIMPLE_("Empty key... using the default one !");
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
		INFO_("Authenticating Reader 1... is HMAC {%d}", isHMAC);
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
			ERROR_("Exception {%s}", e.what());
			getSTidSTRConfiguration()->setCommunicationMode(lastCM);
			throw;
		}
		getSTidSTRConfiguration()->setCommunicationMode(lastCM);

		return ret;
	}

	std::vector<unsigned char> STidSTRReaderUnit::authenticateReader2(const std::vector<unsigned char>& data)
	{
		INFO_SIMPLE_("Authenticating Reader 2...");
		std::vector<unsigned char> ret;
		STidCommunicationMode lastCM = getSTidSTRConfiguration()->getCommunicationMode();
		getSTidSTRConfiguration()->setCommunicationMode(STID_CM_RESERVED);
		try
		{
			ret = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0001, data);
		}
		catch(std::exception& e)
		{
			ERROR_("Exception {%s}", e.what());
			getSTidSTRConfiguration()->setCommunicationMode(lastCM);
			throw;
		}
		getSTidSTRConfiguration()->setCommunicationMode(lastCM);

		return ret;
	}

	void STidSTRReaderUnit::ResetAuthenticate()
	{
		INFO_SIMPLE_("Reseting authentication...");
		getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000D, std::vector<unsigned char>());
	}

	void STidSTRReaderUnit::ChangeReaderKeys(const std::vector<unsigned char>& key_hmac, const std::vector<unsigned char>& key_aes)
	{
		INFO_SIMPLE_("Changing reader keys...");
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
		INFO_("Setting baudrate... baudrate {0x%x(%d)}", baudrate, baudrate);
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(baudrate));
		getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0005, command);
	}

	void STidSTRReaderUnit::set485Address(unsigned char address)
	{
		INFO_("Setting RS485 address... address {0x%x(%u)}", address, address);
		std::vector<unsigned char> command;
		EXCEPTION_ASSERT_WITH_LOG(address <= 127, LibLogicalAccessException, "The RS485 address should be between 0 and 127.");
		command.push_back(address);
		getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0006, command);
	}

	STidSTRReaderUnit::STidSTRInformation STidSTRReaderUnit::getReaderInformaton()
	{
		INFO_SIMPLE_("Getting reader information...");
		std::vector<unsigned char> response = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0008, std::vector<unsigned char>());

		EXCEPTION_ASSERT_WITH_LOG(response.size() >= 5, LibLogicalAccessException, "The GetInfos response should be 5-byte long.");

		STidSTRInformation readerInfo;
		readerInfo.version = response[0];
		readerInfo.baudrate = static_cast<STidBaudrate>(response[1]);
		readerInfo.rs485Address = response[2];
		readerInfo.voltage = (float)response[3] + ((float)response[4] / (float)10);

		INFO_("Returns version {0x%x(%u)} baudrate {0x%x(%u)} rs485 address {0x%x(%u)} voltage {0x%x(%u)}",
			readerInfo.version, readerInfo.version, readerInfo.baudrate, readerInfo.baudrate, readerInfo.rs485Address, readerInfo.rs485Address,
			readerInfo.voltage, readerInfo.voltage);

		return readerInfo;
	}

	void STidSTRReaderUnit::setAllowedCommModes(bool plainComm, bool signedComm, bool cipheredComm)
	{
		INFO_("Setting allowed communication modes... plain comm {%d} signed comm {%d} ciphered comm {%d}", plainComm, signedComm, cipheredComm);
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
		INFO_("Setting tamper switch settings... use tamper {%d} tamper behavior {0x%x(%u)}", useTamperSwitch, behavior, behavior);
		std::vector<unsigned char> command;

		command.push_back((useTamperSwitch) ? 0x01 : 0x00);
		command.push_back(static_cast<unsigned char>(behavior));

		getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000B, command);
	}

	void STidSTRReaderUnit::getTamperSwitchInfos(bool& useTamperSwitch, STidTamperSwitchBehavior& behavior, bool& swChanged)
	{
		INFO_SIMPLE_("Getting tamper switch infos...");
		std::vector<unsigned char> response = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000C, std::vector<unsigned char>());

		EXCEPTION_ASSERT_WITH_LOG(response.size() >= 3, LibLogicalAccessException, "The GetTamperSwitchInfos response should be 3-byte long.");

		useTamperSwitch = (response[0] == 0x01);
		behavior = static_cast<STidTamperSwitchBehavior>(response[1]);
		swChanged = (response[2] == 0x01);

		INFO_("Returns use tamper {%d} tamper behavior {0x%x(%u)}", useTamperSwitch, behavior, behavior);
	}

	void STidSTRReaderUnit::loadSKB()
	{
		INFO_SIMPLE_("Loading SKB...");
		unsigned char statusCode;
		std::vector<unsigned char> response = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000E, std::vector<unsigned char>(), statusCode);

		EXCEPTION_ASSERT_WITH_LOG(response.size() == 0, LibLogicalAccessException, "Unable to load the SKB values. An unknown error occured.");
	}
}

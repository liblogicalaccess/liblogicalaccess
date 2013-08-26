/**
 * \file stidstrreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR reader/card adapter.
 */

#include "stidstrreadercardadapter.hpp"
#include "logicalaccess/crypto/tomcrypt.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"

namespace logicalaccess
{	
	const unsigned char STidSTRReaderCardAdapter::SOF = 0x02;

	STidSTRReaderCardAdapter::STidSTRReaderCardAdapter(STidCmdType adapterType)
		: ReaderCardAdapter()
	{
		//INFO_SIMPLE_("Constructor");
		d_adapterType = adapterType;
	}

	STidSTRReaderCardAdapter::~STidSTRReaderCardAdapter()
	{
		//INFO_SIMPLE_("Destructor");
	}

	boost::shared_ptr<STidSTRReaderUnit> STidSTRReaderCardAdapter::getSTidSTRReaderUnit() const
	{
		return boost::dynamic_pointer_cast<STidSTRReaderUnit>(getDataTransport()->getReaderUnit());
	}

	std::vector<unsigned char> STidSTRReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
	{
		COM_("Sending command command %s command size {%d}...", BufferHelper::getHex(command).c_str(), command.size());
		std::vector<unsigned char> cmd;
		boost::shared_ptr<STidSTRReaderUnitConfiguration> readerConfig = getSTidSTRReaderUnit()->getSTidSTRConfiguration();
		//INFO_("Reader configuration {%s}", dynamic_cast<XmlSerializable*>(&(*readerConfig))->serialize().c_str());

		EXCEPTION_ASSERT_WITH_LOG(command.size() >= 2, LibLogicalAccessException, "The command size must be at least 2 byte long.");
		unsigned short commandCode = command[0] << 8 | command[1];

		cmd.push_back(SOF);
		std::vector<unsigned char> commandEncapsuled = sendMessage(commandCode, std::vector<unsigned char>(command.begin() + 2, command.end()));
		cmd.push_back((commandEncapsuled.size() & 0xff00) >> 8);
		cmd.push_back(commandEncapsuled.size() & 0xff);
		unsigned char CTRL1 = static_cast<unsigned char>(readerConfig->getCommunicationType());
		if (readerConfig->getCommunicationType() == STID_RS485)
		{
			CTRL1 |= (readerConfig->getRS485Address() << 1);
		}
		cmd.push_back(CTRL1);
		cmd.push_back(static_cast<unsigned char>(readerConfig->getCommunicationMode()));
		
		cmd.insert(cmd.end(), commandEncapsuled.begin(), commandEncapsuled.end());
		unsigned char first, second;
		ComputeCrcCCITT(0xFFFF, &cmd[1], cmd.size() - 1, &first, &second);
		cmd.push_back(second);
		cmd.push_back(first);

		return cmd;
	}

	std::vector<unsigned char> STidSTRReaderCardAdapter::sendMessage(unsigned short commandCode, const std::vector<unsigned char>& command)
	{
		COM_("Sending message with command code {0x%x(%u)} command %s command size {%d}...", commandCode, commandCode, BufferHelper::getHex(command).c_str(), command.size());
		std::vector<unsigned char> processedMsg;

		processedMsg.push_back(0x00); // RFU
		processedMsg.push_back(static_cast<unsigned char>(d_adapterType));	// Type
		processedMsg.push_back((commandCode & 0xff00) >> 8);	// Code
		processedMsg.push_back(commandCode & 0xff);	// Code

		processedMsg.push_back(0xAA);	// Reserved
		processedMsg.push_back(0x55);	// Reserved

		processedMsg.push_back((command.size() & 0xff00) >> 8);	// Data length
		processedMsg.push_back(command.size() & 0xff);	// Data length

		processedMsg.insert(processedMsg.end(), command.begin(), command.end());

		boost::shared_ptr<STidSTRReaderUnitConfiguration> readerConfig = getSTidSTRReaderUnit()->getSTidSTRConfiguration();

		// Cipher the data
		if ((readerConfig->getCommunicationMode() & STID_CM_CIPHERED) == STID_CM_CIPHERED)
		{
			COM_SIMPLE_("Need to cipher data ! Ciphering with AES...");
			COM_("Message before ciphering {%s}", BufferHelper::getHex(processedMsg).c_str());
			// 16-byte buffer aligned
			if ((processedMsg.size() % 16) != 0)
			{
				int pad = 16 - (processedMsg.size() % 16);
				for (int i = 0; i < pad; ++i)
				{
					processedMsg.push_back(0x00);
				}
			}

			std::vector<unsigned char> iv = getIV();
			openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(getSTidSTRReaderUnit()->getSessionKeyAES());
			openssl::AESInitializationVector aesiv = openssl::AESInitializationVector::createFromData(iv);
			openssl::AESCipher cipher;

			std::vector<unsigned char> encProcessedMsg;
			cipher.cipher(processedMsg, encProcessedMsg, aeskey, aesiv, false);
			
			if (encProcessedMsg.size() >= 16)
			{
				d_lastIV = std::vector<unsigned char>(encProcessedMsg.end() - 16, encProcessedMsg.end());
			}

			processedMsg = encProcessedMsg;
			processedMsg.insert(processedMsg.end(), iv.begin(), iv.end());

			COM_("Message after ciphering {%s}", BufferHelper::getHex(processedMsg).c_str());
		}
		else
		{
			COM_SIMPLE_("No need to cipher data !");
		}

		// Add the HMAC to the message
		if ((readerConfig->getCommunicationMode() & STID_CM_SIGNED) == STID_CM_SIGNED)
		{
			COM_SIMPLE_("Need to sign data ! Adding the HMAC...");
			COM_("Message before signing {%s}", BufferHelper::getHex(processedMsg).c_str());
			std::vector<unsigned char> hmacbuf = calculateHMAC(processedMsg);
			processedMsg.insert(processedMsg.end(), hmacbuf.begin(), hmacbuf.end());
			COM_("Message after signing {%s}", BufferHelper::getHex(processedMsg).c_str());
		}
		else
		{
			COM_SIMPLE_("No need to sign data !");
		}

		COM_("Final message %s message size {%d}", BufferHelper::getHex(processedMsg).c_str(), processedMsg.size());

		return processedMsg;
	}

	std::vector<unsigned char> STidSTRReaderCardAdapter::getIV()
	{
		if (d_lastIV.size() == 0)
		{
			RAND_seed(&d_lastCommandCode, sizeof(d_lastCommandCode));

			EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

			d_lastIV.resize(16);
			if (RAND_bytes(&d_lastIV[0], static_cast<int>(d_lastIV.size())) != 1)
			{
				COM_SIMPLE_("Cannot retrieve cryptographically strong bytes");
				throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
			}
		}
		return d_lastIV;
	}

	std::vector<unsigned char> STidSTRReaderCardAdapter::sendCommand(unsigned short commandCode, const std::vector<unsigned char>& command, long int timeout)
	{
		COM_("Sending command with command code {0x%x(%u)} command %s command size {%d} timeout {%d}...", commandCode, commandCode, BufferHelper::getHex(command).c_str(), command.size(), timeout);

		std::vector<unsigned char> buffer;
		buffer.push_back(static_cast<unsigned char>((commandCode & 0xff00) >> 8));
		buffer.push_back(static_cast<unsigned char>(commandCode & 0xff));
		buffer.insert(buffer.end(), command.begin(), command.end());

		d_lastCommandCode = commandCode;
		return ReaderCardAdapter::sendCommand(buffer, timeout);;
	}

	std::vector<unsigned char> STidSTRReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{
		COM_("Processing the received buffer %s size {%d}...", BufferHelper::getHex(answer).c_str(), answer.size());
		unsigned char statusCode = 0x00;

		COM_("Command size {%d}", answer.size());
		EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 7, std::invalid_argument, "A valid buffer size must be at least 7 bytes long");

		COM_("Command SOF {0x%x(%u)}", answer[0], answer[0]);
		EXCEPTION_ASSERT_WITH_LOG(answer[0] == SOF, std::invalid_argument, "The supplied buffer is not valid (bad SOF byte)");

		unsigned short messageSize = (answer[1] << 8) | answer[2];

		COM_("Inside message size {%u}", messageSize);
		EXCEPTION_ASSERT_WITH_LOG(static_cast<unsigned int>(messageSize + 7) <= answer.size(), std::invalid_argument, "The buffer is too small to contains the complete message.");

		boost::shared_ptr<STidSTRReaderUnitConfiguration> readerConfig = getSTidSTRReaderUnit()->getSTidSTRConfiguration();

		STidCommunicationType ctype = static_cast<STidCommunicationType>(answer[3] & 0x01);
		COM_("Communication response type {0x%x(%d)}", ctype, ctype);
		EXCEPTION_ASSERT_WITH_LOG(ctype == readerConfig->getCommunicationType(), std::invalid_argument, "The communication type doesn't match.");

		if (ctype == STID_RS485)
		{
			unsigned char rs485Address = static_cast<unsigned char>((answer[3] & 0xfe) >> 1);
			COM_("Communication response rs485 address {0x%x(%u)}", rs485Address, rs485Address);
			EXCEPTION_ASSERT_WITH_LOG(rs485Address == readerConfig->getRS485Address(), std::invalid_argument, "The rs485 reader address doesn't match.");
		}

		STidCommunicationMode cmode = static_cast<STidCommunicationMode>(answer[4]);
		COM_("Communication response mode {0x%x(%d)}", cmode, cmode);
		EXCEPTION_ASSERT_WITH_LOG(cmode == readerConfig->getCommunicationMode() || readerConfig->getCommunicationMode() == STID_CM_RESERVED, std::invalid_argument, "The communication type doesn't match.");
		
		std::vector<unsigned char> data = std::vector<unsigned char>(answer.begin() + 5, answer.begin() + 5 + messageSize);
		COM_("Communication response data %s", BufferHelper::getHex(data).c_str());

		unsigned char first, second;
		ComputeCrcCCITT(0xFFFF, &answer[1], 4 + messageSize, &first, &second);
		EXCEPTION_ASSERT_WITH_LOG(answer[5 + messageSize] == second && answer[5 + messageSize + 1] == first, std::invalid_argument, "The supplied buffer is not valid (CRC missmatch)");		

		return receiveMessage(data, statusCode);
	}

	std::vector<unsigned char> STidSTRReaderCardAdapter::calculateHMAC(const std::vector<unsigned char>& buf) const
	{
		// HMAC-SHA-1

		unsigned int len = 10;
		std::vector<unsigned char> r;
		r.resize(len, 0x00);

		std::vector<unsigned char> sessionkey = getSTidSTRReaderUnit()->getSessionKeyHMAC();
		HMAC(EVP_sha1(), &sessionkey[0], static_cast<int>(sessionkey.size()), &buf[0], buf.size(), &r[0], &len);		

		return r;
	}

	std::vector<unsigned char> STidSTRReaderCardAdapter::receiveMessage(const std::vector<unsigned char>& data, unsigned char& statusCode)
	{
		COM_("Processing the response... data %s data size {%d}", BufferHelper::getHex(data).c_str(), data.size());

		std::vector<unsigned char> tmpData = data;

		boost::shared_ptr<STidSTRReaderUnitConfiguration> readerConfig = getSTidSTRReaderUnit()->getSTidSTRConfiguration();

		// Check the message HMAC and remove it from the message
		if ((readerConfig->getCommunicationMode() & STID_CM_SIGNED) == STID_CM_SIGNED)
		{
			COM_SIMPLE_("Need to check for signed data...");
			EXCEPTION_ASSERT_WITH_LOG(data.size() >= 10, LibLogicalAccessException, "The buffer is too short to contains the message HMAC.");
			tmpData = std::vector<unsigned char>(data.begin(), data.end() - 10);
			EXCEPTION_ASSERT_WITH_LOG(std::vector<unsigned char>(data.end() - 10, data.end()) == calculateHMAC(tmpData), LibLogicalAccessException, "Wrong HMAC.");
			COM_("Data after removing signed data %s", BufferHelper::getHex(tmpData).c_str());
		}

		// Uncipher the data
		if ((readerConfig->getCommunicationMode() & STID_CM_CIPHERED) == STID_CM_CIPHERED)
		{
			COM_SIMPLE_("Need to check for ciphered data...");
			EXCEPTION_ASSERT_WITH_LOG(tmpData.size() >= 16, LibLogicalAccessException, "The buffer is too short to contains the IV.");

			std::vector<unsigned char> iv = std::vector<unsigned char>(tmpData.end() - 16, tmpData.end());
			tmpData.resize(tmpData.size() - 16);
			openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(getSTidSTRReaderUnit()->getSessionKeyAES());
			openssl::AESInitializationVector aesiv = openssl::AESInitializationVector::createFromData(iv);
			openssl::AESCipher cipher;

			std::vector<unsigned char> decTmpData;
			cipher.decipher(tmpData, decTmpData, aeskey, aesiv, false);

			if (tmpData.size() >= 16)
			{
				d_lastIV = std::vector<unsigned char>(tmpData.end() - 16, tmpData.end());
			}
			tmpData = decTmpData;

			COM_("Data after removing ciphered data %s", BufferHelper::getHex(tmpData).c_str());
		}
		
		EXCEPTION_ASSERT_WITH_LOG(tmpData.size() >= 6, LibLogicalAccessException, "The plain response message should be at least 6 bytes long.");

		size_t offset = 0;
		unsigned short ack = (tmpData[offset] << 8) | tmpData[offset+1];
		offset += 2;
		COM_("Acquiment value {0x%x(%u)}", ack, ack);
		EXCEPTION_ASSERT_WITH_LOG(ack == d_lastCommandCode, LibLogicalAccessException, "ACK doesn't match the last command code.");

		unsigned short msglength = (tmpData[offset] << 8) | tmpData[offset+1];
		offset += 2;
		COM_("Plain data length {%u}", msglength);

		EXCEPTION_ASSERT_WITH_LOG(static_cast<unsigned int>(msglength + 2) <= tmpData.size(), LibLogicalAccessException, "The buffer is too short to contains the complete plain message.");

		std::vector<unsigned char> plainData = std::vector<unsigned char>(tmpData.begin() + offset, tmpData.begin() + offset + msglength);
		offset += msglength;

		STidCmdType statusType = static_cast<STidCmdType>(tmpData[offset++]);
		COM_("Status type {%d != %d}", statusType, d_adapterType);

		EXCEPTION_ASSERT_WITH_LOG(statusType == d_adapterType, LibLogicalAccessException, "Bad message type for this reader/card adapter.");

		statusCode = tmpData[offset++];
		COM_("Plain data status code {0x%x(%u)}", statusCode, statusCode);
		CheckError(statusCode);

		return plainData;
	}	

	void STidSTRReaderCardAdapter::CheckError(unsigned char statusCode) const
	{
		if (statusCode == 0x00)
			return;

		char conv[64];
		std::string message = std::string("Communication error: ");
		sprintf(conv, "%x", statusCode);
		message += std::string(conv);
		message += std::string(". ");
		std::string msg = "";

		switch (d_adapterType)
		{
			case STID_CMD_DESFIRE:
			{
				message += std::string("DESFire - ");
				switch (statusCode)
				{
				case 0x01:
					msg = std::string("More than one tag in the RFID field.");
					break;
				case 0x02:
					msg = std::string("Incorrect tag type.");
					break;
				case 0x0C:
					msg = std::string("No change made to backup files.");
					break;
				case 0x0E:
					msg = std::string("Not enough EEPROM memory.");
					break;
				case 0x1C:
					msg = std::string("Incorrect Command code.");
					break;
				case 0x1E:
					msg = std::string("DESFire integrity error.");
					break;
				case 0x40:
					msg = std::string("Key does not exist.");
					break;
				case 0x7E:
					msg = std::string("Incorrect length.");
					break;
				case 0x9D:
					msg = std::string("Permission denied.");
					break;
				case 0x9E:
					msg = std::string("Incorrect setting.");
					break;
				case 0xA0:
					msg = std::string("Application not found.");
					break;
				case 0xA1:
					msg = std::string("Application integrity error.");
					break;
				case 0xAE:
					msg = std::string("Authentication error.");
					break;
				case 0xAF:
					msg = std::string("Frame expected.");
					break;
				case 0xBE:
					msg = std::string("Limit exceeded.");
					break;
				case 0xC1:
					msg = std::string("Card integrity error.");
					break;
				case 0xCA:
					msg = std::string("Command aborted.");
					break;
				case 0xCD:
					msg = std::string("Card disabled.");
					break;
				case 0xCE:
					msg = std::string("Maximum number of applications reached.");
					break;
				case 0xDE:
					msg = std::string("Duplicate AIDs or files.");
					break;
				case 0xEE:
					msg = std::string("EEPROM error.");
					break;
				case 0xF0:
					msg = std::string("File not found.");
					break;
				case 0xF1:
					msg = std::string("File integrity error.");
					break;
				default:
					msg = std::string("Unknown error");
				}
				break;
			}
			case STID_CMD_MIFARE_CLASSIC:
			{
				message += std::string("Mifare - ");
				switch (statusCode)
				{
				case 0x01:
					msg = std::string("MIFARE® time out error.");
					break;
				case 0x02:
					msg = std::string("More than one tag in the RFID field.");
					break;
				case 0x03:
					msg = std::string("Incorrect tag type.");
					break;
				case 0x05:
					msg = std::string("MIFARE® frame error.");
					break;
				case 0x06:
					msg = std::string("Incorrect settings error.");
					break;
				case 0x13:
					msg = std::string("MIFARE® data format error.");
					break;
				case 0x14:
					msg = std::string("MIFARE® authentication error.");
					break;
				case 0x27:
					msg = std::string("Invalid command error.");
					break;
				default:
					msg = std::string("Unknown error");
				}
				break;
			}
			default:
			{
				switch (statusCode)
				{
				case 0x01:
					msg = std::string("Authentication error with reader.");
					break;
				case 0x02:
					msg = std::string("Incorrect data setting.");
					break;
				case 0x03:
					msg = std::string("CRC error on the frame.");
					break;
				case 0x04:
					msg = std::string("Incorrect frame length received.");
					break;
				case 0x05:
					msg = std::string("Signature error at authentication.");
					break;
				case 0x06:
					msg = std::string("Time out error.");
					break;
				case 0x07:
					msg = std::string("Incorrect Command code.");
					break;
				case 0x08:
					msg = std::string("Incorrect command type.");
					break;
				case 0x11:
					msg = std::string("Communication mode not allowed with the reader.");
					break;
				case 0xF3:
					msg = std::string("Incorrect tag.");
					break;
				case 0xF4:
					msg = std::string("No SKB tag.");
					break;
				default:
					msg = std::string("Unknown error");
				}
				break;
			}
		}

		if (msg != "")
		{
			message += msg;
			THROW_EXCEPTION_WITH_LOG(CardException, message);
		}
	}
}

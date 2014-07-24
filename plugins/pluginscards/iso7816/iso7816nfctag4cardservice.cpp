/**
 * \file iso7816nfctag4cardservice.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief ISO7816 NFC Tag Type 4 card service.
 */

#include "iso7816nfctag4cardservice.hpp"
#include "logicalaccess/services/nfctag/ndefmessage.hpp"

namespace logicalaccess
{
	void ISO7816NFCTag4CardService::writeCapabilityContainer(unsigned short isoFID, unsigned short isoFIDNDEFFile, unsigned short NDEFFileSize)
	{
		boost::shared_ptr<logicalaccess::ISO7816Commands> iso7816command(boost::dynamic_pointer_cast<logicalaccess::ISO7816Commands>(getChip()->getCommands()));

		std::vector<unsigned char> CC;
		CC.push_back(0x00);//CCLEN
		CC.push_back(0x0f);

		CC.push_back(0x20);//Mapping version

		CC.push_back(0x00);//maximum data size can be read with READ BINARY (Default Value for DESFire)
		CC.push_back(0x3A);

		CC.push_back(0x00);//maximum data size can be write with UPDATE BINARY (Default Value for DESFire)
		CC.push_back(0x34);

		CC.push_back(0x04);//Tag , FileControl TLV (4 = NDEFfile)
		CC.push_back(0x06);//Length, FileControl TLV (6 = 6 bytes of data for this tag)
		CC.push_back((isoFIDNDEFFile & 0xff00) >> 8);//file identifier 0xe104-0xe10f or 0xe100/0xe101
		CC.push_back(isoFIDNDEFFile & 0xff);//file identifier 
		CC.push_back((NDEFFileSize & 0xff00) >> 8);//MemorySize
		CC.push_back(NDEFFileSize & 0xff);//MemorySize
		CC.push_back(0x00);//read access
		CC.push_back(0x00);//write access

		iso7816command->selectFile(isoFID);
		iso7816command->updateBinary(&CC[0], CC.size(), 0, isoFID);
	}

	void ISO7816NFCTag4CardService::writeNDEFFile(std::vector<unsigned char> data, unsigned short isoFIDNDEFFile)
	{
		boost::shared_ptr<logicalaccess::ISO7816Commands> iso7816command(boost::dynamic_pointer_cast<logicalaccess::ISO7816Commands>(getChip()->getCommands()));

		struct NDEFMessage
		{
			bool MessageBegin;
			bool MessageEnd;

		};

		//std::vector<unsigned char> data;
		data.push_back(0x00); //NLEN
		data.push_back(0x14); //NDEF Length

		data.push_back(0xd1); //Record header
		data.push_back(0x01); //type length
		data.push_back(0x10); //payload length
		data.push_back(0x54); //Text
		data.push_back(0x02); //Status byte
		data.push_back(0x65); // 'e' Payload Data 'Hello, world!'
		data.push_back(0x6e); // 'n'

		data.push_back(0x48);
		data.push_back(0x65);
		data.push_back(0x6c);
		data.push_back(0x6c);
		data.push_back(0x6f);
		data.push_back(0x2c);
		data.push_back(0x20);
		data.push_back(0x77);
		data.push_back(0x6f);
		data.push_back(0x72);
		data.push_back(0x6c);
		data.push_back(0x64);
		data.push_back(0x21);

		iso7816command->selectFile(isoFIDNDEFFile);
		iso7816command->updateBinary(&data[0], data.size(), 0, isoFIDNDEFFile);
	}

	NdefMessage ISO7816NFCTag4CardService::readNDEFFile(unsigned short isoFIDApplication, unsigned short isoFIDNDEFFile)
	{
		boost::shared_ptr<logicalaccess::ISO7816Commands> iso7816command(boost::dynamic_pointer_cast<logicalaccess::ISO7816Commands>(getChip()->getCommands()));

		size_t length = 0x02;
		std::vector<unsigned char> data(length);
		iso7816command->selectFile(isoFIDApplication);
		iso7816command->selectFile(isoFIDNDEFFile);
		iso7816command->readBinary(&data[0], length, 0, isoFIDNDEFFile);

		if (length != 0x02)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Impossible to read NDEF Length");
		length = data[1];
		data.resize(length);
		iso7816command->readBinary(&data[0], length, 2, isoFIDNDEFFile);
		return logicalaccess::NdefMessage(data);
	}
}

/**
 * \file iso15693pcsccommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO 15693 commands for PC/SC readers.
 */

#include "../commands/iso15693pcsccommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "iso15693chip.hpp"


namespace logicalaccess
{
	ISO15693PCSCCommands::ISO15693PCSCCommands()
		: ISO15693Commands()
	{

	}

	ISO15693PCSCCommands::~ISO15693PCSCCommands()
	{

	}	

	void ISO15693PCSCCommands::stayQuiet()
	{
		unsigned char command[3];
		command[0] = 0x01;
		command[1] = 0x00;
		command[2] = 0x01;
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x01, 3, command, sizeof(command), result, &resultlen);
	}

	bool ISO15693PCSCCommands::readBlock(size_t block, void* data, size_t datalen, size_t le)
	{
		bool ret = false;
		unsigned char result[255];
		unsigned char p1 = (block & 0xffff) >> 8;
		unsigned char p2 = static_cast<unsigned char>(block & 0xff);
		size_t resultlen = sizeof(result);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0xb0, p1, p2, static_cast<unsigned char>(le), result, &resultlen);

		if (datalen >= resultlen-2)
		{
			memcpy(data, result, resultlen-2);
			ret = true;
		}

		return ret;
	}

	bool ISO15693PCSCCommands::writeBlock(size_t block, const void* data, size_t datalen)
	{
		bool ret = false;
		unsigned char result[255];
		unsigned char p1 = (block & 0xffff) >> 8;
		unsigned char p2 = static_cast<unsigned char>(block & 0xff);
		size_t resultlen = sizeof(result);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0xd6, p1, p2, static_cast<unsigned char>(datalen), reinterpret_cast<const unsigned char*>(data), datalen, result, &resultlen);

		if (datalen >= resultlen-2)
		{
			ret = true;
		}

		return ret;
	}

	void ISO15693PCSCCommands::lockBlock(size_t block)
	{
		unsigned char command[5];
		command[0] = 0x01;
		command[1] = 0x00;
		command[2] = 0x00;
		command[3] = (block & 0xffff) >> 8;
		command[4] = static_cast<unsigned char>(block & 0xff);
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x02, 5, command, sizeof(command), result, &resultlen);
	}

	void ISO15693PCSCCommands::writeAFI(size_t afi)
	{
		unsigned char command[4];
		command[0] = 0x01;
		command[1] = 0x02;
		command[2] = 0x00;
		command[3] = (unsigned char)afi;
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x01, 4, command, sizeof(command), result, &resultlen);
	}

	void ISO15693PCSCCommands::lockAFI()
	{
		unsigned char command[3];
		command[0] = 0x01;
		command[1] = 0x02;
		command[2] = 0x00;
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x02, 3, command, sizeof(command), result, &resultlen);
	}

	void ISO15693PCSCCommands::writeDSFID(size_t dsfid)
	{
		unsigned char command[4];
		command[0] = 0x01;
		command[1] = 0x03;
		command[2] = 0x00;
		command[3] = (unsigned char)dsfid;
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x01, 4, command, sizeof(command), result, &resultlen);
	}

	void ISO15693PCSCCommands::lockDSFID()
	{
		unsigned char command[3];
		command[0] = 0x01;
		command[1] = 0x03;
		command[2] = 0x00;
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x02, 3, command, sizeof(command), result, &resultlen);
	}

	ISO15693Commands::SystemInformation ISO15693PCSCCommands::getSystemInformation()
	{
		ISO15693Commands::SystemInformation systeminfo;
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		memset(&systeminfo, 0x00, sizeof(systeminfo));

		// AFI information
		try
		{
			getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x02, 0x00, 0x00, result, &resultlen);		
			systeminfo.hasAFI = true;
			systeminfo.AFI = result[0];
		}
		catch(LibLOGICALACCESSException&)
		{
			systeminfo.hasAFI = false;
		}
			
		resultlen = sizeof(result);

		// DSFID information
		try
		{
			getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x03, 0x00, 0x00, result, &resultlen);			
			systeminfo.hasDSFID = true;
			systeminfo.DSFID = result[0];
		}
		catch(LibLOGICALACCESSException&)
		{
			systeminfo.hasDSFID = false;
		}

		resultlen = sizeof(result);

		// PICC memory size information
		try
		{
			getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x04, 0x00, 0x00, result, &resultlen);				
			systeminfo.hasVICCMemorySize = true;
			systeminfo.blockSize = (result[1] & 0x1f) + 1;
			systeminfo.nbBlocks = result[0] + 1;
		}
		catch(LibLOGICALACCESSException&)
		{
			systeminfo.hasVICCMemorySize = false;
		}

		resultlen = sizeof(result);

		// IC reference information
		try
		{
			getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x05, 0x00, 0x00, result, &resultlen);
			systeminfo.hasICReference = true;
			systeminfo.ICReference = result[0];
		}
		catch(LibLOGICALACCESSException&)
		{
			systeminfo.hasICReference = false;
		}

		return systeminfo;
	}

	unsigned char ISO15693PCSCCommands::getSecurityStatus(size_t block)
	{
		unsigned char command[5];
		command[0] = 0x01;
		command[1] = 0x00;
		command[2] = 0x00;
		command[3] = (block & 0xffff) >> 8;
		command[4] = static_cast<unsigned char>(block & 0xff);
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x03, 5, command, sizeof(command), 0x01, result, &resultlen);

		return result[0];
	}	
}


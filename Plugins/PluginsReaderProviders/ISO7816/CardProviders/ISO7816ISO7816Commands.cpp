/**
 * \file ISO7816ISO7816Commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 commands.
 */

#include "../Commands/ISO7816ISO7816Commands.h"

#include <cstring>

namespace logicalaccess
{
	ISO7816ISO7816Commands::ISO7816ISO7816Commands()
		: ISO7816Commands()
	{

	}

	ISO7816ISO7816Commands::~ISO7816ISO7816Commands()
	{

	}

	void ISO7816ISO7816Commands::setP1P2(size_t offset, short efid, unsigned char& p1, unsigned char& p2)
	{
		p1 = 0x00;
		p2 = 0x00;

		if (efid == 0)
		{
			p1 = 0x7f & (offset >> 8);
		}
		else
		{
			p1 = 0x80 | (0x0f & efid);
		}
		p2 = 0xff & offset;
	}

	bool ISO7816ISO7816Commands::readBinay(void* data, size_t& dataLength, size_t offset, short efid)
	{
		bool ret = false;
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		unsigned char p1, p2;

		setP1P2(offset, efid, p1, p2);
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x00, 0xB0, p1, p2, static_cast<unsigned char>(dataLength), result, &resultlen);

		if (dataLength >= resultlen-2)
		{			
			memcpy(data, result, resultlen-2);
			ret = true;
		}
		dataLength = resultlen-2;

		return ret;
	}
	
	void ISO7816ISO7816Commands::writeBinay(const void* data, size_t dataLength, size_t offset, short efid)
	{
		unsigned char p1, p2;

		setP1P2(offset, efid, p1, p2);
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x00, 0xD0, p1, p2, static_cast<unsigned char>(dataLength), reinterpret_cast<const unsigned char*>(data), dataLength);
	}

	void ISO7816ISO7816Commands::updateBinay(const void* data, size_t dataLength, size_t offset, short efid)
	{
		unsigned char p1, p2;

		setP1P2(offset, efid, p1, p2);
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x00, 0xD6, p1, p2, static_cast<unsigned char>(dataLength), reinterpret_cast<const unsigned char*>(data), dataLength);
	}

	void ISO7816ISO7816Commands::eraseBinay(size_t offset, short efid)
	{
		unsigned char p1, p2;

		setP1P2(offset, efid, p1, p2);
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x00, 0x0E, p1, p2, NULL, NULL);
	}

	bool ISO7816ISO7816Commands::getData(void* data, size_t& dataLength, unsigned short dataObject)
	{
		bool ret = false;
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x00, 0xCA, (0xff & (dataObject >> 8)), (0xff & dataObject), result, &resultlen);
		if (dataLength >= resultlen-2)
		{			
			memcpy(data, result, resultlen-2);
			ret = true;
		}
		dataLength = resultlen-2;

		return ret;
	}

	void ISO7816ISO7816Commands::putData(const void* data, size_t dataLength, unsigned short dataObject)
	{
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x00, 0xDA, (0xff & (dataObject >> 8)), (0xff & dataObject), static_cast<unsigned char>(dataLength), reinterpret_cast<const unsigned char*>(data), dataLength);
	}

	void ISO7816ISO7816Commands::selectFile(unsigned short efid)
	{
		unsigned char p1 = 0x00;
		unsigned char p2 = 0x00;
		unsigned char data[2];
		data[0] = 0xff & (efid >> 8);
		data[1] = 0xff & efid;

		selectFile(p1, p2, data, sizeof(data));
	}

	void ISO7816ISO7816Commands::selectFile(unsigned char* dfname, size_t dfnamelen)
	{
		unsigned char p1 = 0x04;
		unsigned char p2 = 0x00;

		selectFile(p1, p2, dfname, dfnamelen);
	}

	void ISO7816ISO7816Commands::selectFile(unsigned char p1, unsigned char p2, unsigned char* data, size_t datalen)
	{
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x00, 0xA4, p1, p2, static_cast<unsigned char>(datalen), data, datalen);
	}
}

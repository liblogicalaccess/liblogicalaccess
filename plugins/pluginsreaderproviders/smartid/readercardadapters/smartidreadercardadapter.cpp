/**
 * \file smartidreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SmartID reader/card adapter.
 */

#include "smartidreadercardadapter.hpp"

#include <cstring>

namespace logicalaccess
{	
	const unsigned char SmartIDReaderCardAdapter::DLE = 0x10;
	const unsigned char SmartIDReaderCardAdapter::STX = 0x02;
	const unsigned char SmartIDReaderCardAdapter::ETX = 0x03;
	const unsigned char SmartIDReaderCardAdapter::NAK = 0x15;

	/* Protocol status */
	const unsigned char SmartIDReaderCardAdapter::RET_OK = 0x00;
	const unsigned char SmartIDReaderCardAdapter::RET_OK_CHAIN = 0x80;
	const unsigned char SmartIDReaderCardAdapter::RET_NOTAG = 0x01;
	const unsigned char SmartIDReaderCardAdapter::RET_FORMATERR = 0x60;
	const unsigned char SmartIDReaderCardAdapter::RET_E2PROMERR = 0x61;
	const unsigned char SmartIDReaderCardAdapter::RET_ENVNOTFOUND = 0x62;
	const unsigned char SmartIDReaderCardAdapter::RET_NY_IMPLEMENTED = 0x64;

	SmartIDReaderCardAdapter::SmartIDReaderCardAdapter()
		: ISO7816ReaderCardAdapter()
	{

	}

	SmartIDReaderCardAdapter::~SmartIDReaderCardAdapter()
	{
		
	}

	void SmartIDReaderCardAdapter::sendAPDUCommand(const std::vector<unsigned char>& command, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> res = sendCommand(0x52, command);
		if (result != NULL && resultlen != NULL)
		{
			if (*resultlen >= res.size())
			{
				*resultlen = res.size();
				memcpy(result, &res[0], res.size());
			}
		}
	}

	std::vector<unsigned char> SmartIDReaderCardAdapter::sendCommand(unsigned char cmd, const std::vector<unsigned char>& command, long int timeout)
	{
		EXCEPTION_ASSERT_WITH_LOG(command.size() < 256, std::invalid_argument, "Data size cannot exceed 255 bytes");
		
		std::vector<unsigned char> buffer;
		buffer.push_back(DLE);
		buffer.push_back(STX);
		buffer.push_back(cmd);
		buffer.push_back(static_cast<unsigned char>(command.size()));

		if (command.size() == DLE)
		{
			buffer.push_back(static_cast<unsigned char>(command.size()));
		}

		for (size_t i = 0; i < command.size(); ++i)
		{
			unsigned char c = command[i];
			buffer.push_back(c);

			if (c == DLE)
			{
				buffer.push_back(c);
			}
		}

		unsigned char bcc = static_cast<unsigned char>(cmd ^ command.size());

		for (size_t i = 0; i < command.size(); ++i)
		{
			bcc ^= command[i];
		}

		buffer.push_back(bcc);

		if (bcc == DLE)
		{
			buffer.push_back(bcc);
		}

		buffer.push_back(DLE);
		buffer.push_back(ETX);

		return sendCommand(buffer, timeout);
	}

	std::vector<unsigned char> SmartIDReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		std::vector<unsigned char> r;
		d_lastCommand = command;
		d_lastResult.clear();

		if (command.size() > 0)
		{
			getSmartIDReaderUnit()->connectToReader();
			
			boost::shared_ptr<SerialPortXml> port = getSmartIDReaderUnit()->getSerialPort();
			port->getSerialPort()->write(command);

			unsigned char sta = RET_OK;
			if (!receiveCommand(r, sta, timeout))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, "No response received");
			}

			EXCEPTION_ASSERT_WITH_LOG(sta == RET_OK, LibLogicalAccessException, "Negative reponse");
			d_lastResult = r;
		}

		return r;
	}

	std::vector<unsigned char> SmartIDReaderCardAdapter::handleCommandBuffer(const std::vector<unsigned char>& cmdbuf, unsigned char& sta)
	{
		if (cmdbuf[0] == NAK)
		{
			sta = NAK;

			return std::vector<unsigned char>();
		}

		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() > 4, std::invalid_argument, "A valid command buffer size must be at least 5 bytes long");
		EXCEPTION_ASSERT_WITH_LOG((cmdbuf[0] == DLE) && (cmdbuf[1] == STX), std::invalid_argument, "The supplied command buffer is not valid");

		sta = cmdbuf[2];
		size_t len = cmdbuf[3];

		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() - 4 >= len, std::invalid_argument, "The real message length is below the expected message length");

		std::vector<unsigned char> data;
		std::vector<unsigned char> buf = std::vector<unsigned char>(cmdbuf.begin() + 4 + ((len == DLE) ? 1 : 0), cmdbuf.end());

		int offset = 0;
		for (size_t i = 0; i < len; ++i)
		{
			unsigned char c = buf[offset++];

			if (c == DLE)
			{
				c = buf[offset++];

				EXCEPTION_ASSERT_WITH_LOG(c == DLE, std::invalid_argument, "Malformed command message");
			}

			data.push_back(c);
		}

		char bcc = buf[offset++];

		char bcccheck = static_cast<unsigned char>(sta ^ len);

		for (size_t i = 0; i < len; ++i)
		{
			bcccheck ^= data[i];
		}

		EXCEPTION_ASSERT_WITH_LOG(bcc == bcccheck, std::invalid_argument, "Checksum mismatch");

		if (bcc == DLE)
		{
			EXCEPTION_ASSERT_WITH_LOG(buf[offset++] == DLE, std::invalid_argument, "BCC is equal to DLE but is not doubled !");
		}

		EXCEPTION_ASSERT_WITH_LOG((buf[offset++] == DLE) && (buf[offset++] == ETX), std::invalid_argument, "Missing end of command message");

		return data;
	}

	bool SmartIDReaderCardAdapter::receiveCommand(std::vector<unsigned char>& buf, unsigned char& sta, long int timeout)
	{		
		std::vector<unsigned char> res;
		boost::shared_ptr<SerialPortXml> port = getSmartIDReaderUnit()->getSerialPort();

		boost::posix_time::time_duration ptimeout = boost::posix_time::milliseconds(timeout + 100);
		while (port->getSerialPort()->select(ptimeout))
		{
			std::vector<unsigned char> tmpbuf;

			while (port->getSerialPort()->read(tmpbuf, 256) > 0)
			{
				res.insert(res.end(), tmpbuf.begin(), tmpbuf.end());

				try
				{
					buf = handleCommandBuffer(res, sta);

					return true;
				}
				catch (std::invalid_argument&)
				{
				}
			}
		}

		return false;
	}


	void SmartIDReaderCardAdapter::tclHalt()
	{
		sendCommand(0x45, std::vector<unsigned char>());
	}

	std::vector<unsigned char> SmartIDReaderCardAdapter::requestA()
	{
		return sendCommand(0x50, std::vector<unsigned char>());
	}

	std::vector<unsigned char> SmartIDReaderCardAdapter::requestATS()
	{
		return sendCommand(0x51, std::vector<unsigned char>());
	}

	void SmartIDReaderCardAdapter::haltA()
	{
		tclHalt();
	}

	std::vector<unsigned char> SmartIDReaderCardAdapter::anticollisionA()
	{
		/* TODO: Something ? */
		return std::vector<unsigned char>();
	}

	std::vector<unsigned char> SmartIDReaderCardAdapter::requestB(unsigned char afi)
	{
		std::vector<unsigned char> buf;
		buf.push_back(afi);

		return sendCommand(0x61, buf);
	}

	void SmartIDReaderCardAdapter::haltB()
	{
		tclHalt();
	}

	void SmartIDReaderCardAdapter::attrib()
	{
		sendCommand(0x62, std::vector<unsigned char>());
	}

	std::vector<unsigned char> SmartIDReaderCardAdapter::anticollisionB(unsigned char afi)
	{
		std::vector<unsigned char> buf;
		buf.push_back(afi);

		return sendCommand(0x60, buf);
	}	
}

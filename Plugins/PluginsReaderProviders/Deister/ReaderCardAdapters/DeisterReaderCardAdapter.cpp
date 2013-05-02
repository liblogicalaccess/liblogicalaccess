/**
 * \file DeisterReaderCardAdapter.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Deister reader/card adapter.
 */

#include "DeisterReaderCardAdapter.h"
#include "logicalaccess/crypto/tomcrypt.h"


namespace LOGICALACCESS
{	
	const unsigned char DeisterReaderCardAdapter::SHFT = 0xF8;
	const unsigned char DeisterReaderCardAdapter::SOM = 0xFB;
	const unsigned char DeisterReaderCardAdapter::SOC = 0xFD;
	const unsigned char DeisterReaderCardAdapter::STOP = 0xFE;
	const unsigned char DeisterReaderCardAdapter::Dummy = 0xFF;
	const unsigned char DeisterReaderCardAdapter::SHFT_SHFT = 0x00;
	const unsigned char DeisterReaderCardAdapter::SHFT_SOM = 0x01;
	const unsigned char DeisterReaderCardAdapter::SHFT_SOC = 0x02;
	const unsigned char DeisterReaderCardAdapter::SHFT_STOP = 0x03;

	DeisterReaderCardAdapter::DeisterReaderCardAdapter()
		: ReaderCardAdapter()
	{
		d_source = 0x11;
		d_destination = 0x3f; 
	}

	DeisterReaderCardAdapter::~DeisterReaderCardAdapter()
	{
		
	}

	bool DeisterReaderCardAdapter::sendCommandWithoutResponse(const std::vector<unsigned char>& command)
	{
		bool ret = false;
		unsigned char tmpcmd[3];
		tmpcmd[0] = d_source;
		tmpcmd[1] = d_destination;
		tmpcmd[2] = SOC;
		std::vector<unsigned char> cmd = prepareDataForDevice(command);		
		cmd.insert(cmd.begin(), tmpcmd, tmpcmd + 3);
		unsigned char first, second;
		ComputeCrcKermit(&cmd[0], cmd.size(), &first, &second);
		tmpcmd[0] = Dummy;
		tmpcmd[1] = Dummy;
		cmd.insert(cmd.begin(), tmpcmd, tmpcmd + 2);
		cmd.push_back(first);
		cmd.push_back(second);
		cmd.push_back(STOP);
		d_lastCommand = cmd;
		d_lastResult.clear();

		if (command.size() > 0)
		{
			getDeisterReaderUnit()->connectToReader();

			boost::shared_ptr<SerialPortXml> port = getDeisterReaderUnit()->getSerialPort();
			port->getSerialPort()->write(cmd);

			ret = true;
		}

		return ret;
	}

	std::vector<unsigned char> DeisterReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		std::vector<unsigned char> r;
		if (sendCommandWithoutResponse(command))
		{
			if (!receiveCommand(r, timeout))
			{
				r.clear();
				//THROW_EXCEPTION_WITH_LOG(CardException, "No response received");
			}

			d_lastResult = r;
		}

		return r;
	}

	std::vector<unsigned char> DeisterReaderCardAdapter::handleCommandBuffer(const std::vector<unsigned char>& cmdbuf)
	{
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() >= 10, std::invalid_argument, "A valid command buffer size must be at least 10 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf[0] == Dummy, std::invalid_argument, "The supplied command buffer is not valid (bad dummy byte)");
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf[1] == Dummy, std::invalid_argument, "The supplied command buffer is not valid (bad dummy byte)");
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf[2] == SOM, std::invalid_argument, "The supplied command buffer is not valid (bad start message byte)");
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf[3] == d_source, std::invalid_argument, "The destination address is not valid");
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf[4] == d_destination, std::invalid_argument, "The source address is not valid");
		//unsigned char cmd = cmdbuf.constData()[5];
		unsigned  char errorcode = cmdbuf[6];
		EXCEPTION_ASSERT_WITH_LOG(errorcode == 0x00 || errorcode == 0x10, std::invalid_argument, "The command return an error");
		
		std::vector<unsigned char> buf = std::vector<unsigned char>(cmdbuf.begin() + 7, cmdbuf.end());
		size_t i = 0;
		for (; i < buf.size(); ++i)
		{
			if (buf[i] == STOP)
			{
				break;
			}
		}
		EXCEPTION_ASSERT_WITH_LOG(i < buf.size(), std::invalid_argument, "The supplied command buffer is not valid (no stop byte)");
		buf.resize(i);
		std::vector<unsigned char> data = prepareDataFromDevice(buf);
		EXCEPTION_ASSERT_WITH_LOG(data.size() >= 2, std::invalid_argument, "The supplied command buffer is not valid (no CRC)");
		data.insert(data.begin(), cmdbuf.begin() + 2, cmdbuf.begin() + 2 + 5);
		unsigned char first, second;
		ComputeCrcKermit(&data[0], data.size() - 2, &first, &second);
		EXCEPTION_ASSERT_WITH_LOG(data[data.size()-2] == first && data[data.size()-1] == second, std::invalid_argument, "The supplied command buffer is not valid (CRC missmatch)");		
		// Remove header and CRC
		data = std::vector<unsigned char>(data.begin() + 5, data.end() - 2);

		return data;
	}

	bool DeisterReaderCardAdapter::receiveCommand(std::vector<unsigned char>& buf, long int timeout)
	{		
		std::vector<unsigned char> res;
		boost::shared_ptr<SerialPortXml> port = getDeisterReaderUnit()->getSerialPort();

		boost::posix_time::time_duration ptimeout = boost::posix_time::milliseconds(timeout);
		while (port->getSerialPort()->select(ptimeout))
		{
			std::vector<unsigned char> tmpbuf;

			while (port->getSerialPort()->read(tmpbuf, 256) > 0)
			{
				res.insert(res.end(), tmpbuf.begin(), tmpbuf.end());
				tmpbuf.clear();

				try
				{
					buf = handleCommandBuffer(res);

					return true;
				}
				catch (std::invalid_argument&)
				{
				}
			}
		}

		return false;
	}

	std::vector<unsigned char> DeisterReaderCardAdapter::prepareDataForDevice(const std::vector<unsigned char>& data) const
	{
		std::vector<unsigned char> prepareData;
		for (size_t i = 0; i < data.size(); i++)
		{
			switch (data[i])
			{
			case SHFT:
				prepareData.push_back(SHFT);
				prepareData.push_back(SHFT_SHFT);
				break;

			case SOM:
				prepareData.push_back(SHFT);
				prepareData.push_back(SHFT_SOM);
				break;

			case SOC:
				prepareData.push_back(SHFT);
				prepareData.push_back(SHFT_SOC);
				break;

			case STOP:
				prepareData.push_back(SHFT);
				prepareData.push_back(SHFT_STOP);
				break;

			default:
				prepareData.push_back(data[i]);
				break;
			}
		}
		return prepareData;
	}

	std::vector<unsigned char> DeisterReaderCardAdapter::prepareDataFromDevice(const std::vector<unsigned char>& data) const
	{
		std::vector<unsigned char> prepareData;
		for (size_t i = 0; i < data.size(); i++)
		{
			if (data[i] == SHFT)
			{
				EXCEPTION_ASSERT_WITH_LOG(i++ < data.size(), std::invalid_argument, "Bad data buffer (no second shft byte)");
				switch (data[i])
				{
				case SHFT_SHFT:
					prepareData.push_back(SHFT);
					break;

				case SHFT_SOM:
					prepareData.push_back(SOM);
					break;

				case SHFT_SOC:
					prepareData.push_back(SOC);
					break;

				case SHFT_STOP:
					prepareData.push_back(STOP);
					break;

				default:
					THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad data buffer (unknow second shft byte)");
					break;
				}
			}
			else
			{
				prepareData.push_back(data[i]);
			}
		}
		return prepareData;
	}
}

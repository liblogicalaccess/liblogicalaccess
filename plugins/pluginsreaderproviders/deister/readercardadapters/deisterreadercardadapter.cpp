/**
 * \file deisterreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Deister reader/card adapter.
 */

#include "deisterreadercardadapter.hpp"
#include "logicalaccess/crypto/tomcrypt.h"


namespace logicalaccess
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

	std::vector<unsigned char> DeisterReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
	{
		std::vector<unsigned char> cmd;
		cmd.push_back(Dummy);
		cmd.push_back(Dummy);
		cmd.push_back(SOC);
		cmd.push_back(d_destination);
		cmd.push_back(d_source);
		std::vector<unsigned char> preparedCmd = prepareDataForDevice(command);
		cmd.insert(cmd.end(), preparedCmd.begin(), preparedCmd.end());
		unsigned char first, second;
		ComputeCrcKermit(&cmd[2], cmd.size() - 2, &first, &second);
		cmd.push_back(first);
		cmd.push_back(second);
		cmd.push_back(STOP);

		return cmd;
	}

	std::vector<unsigned char> DeisterReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{
		EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 10, std::invalid_argument, "A valid buffer size must be at least 10 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(answer[0] == Dummy, std::invalid_argument, "The supplied buffer is not valid (bad dummy byte)");
		EXCEPTION_ASSERT_WITH_LOG(answer[1] == Dummy, std::invalid_argument, "The supplied buffer is not valid (bad dummy byte)");
		EXCEPTION_ASSERT_WITH_LOG(answer[2] == SOM, std::invalid_argument, "The supplied buffer is not valid (bad start message byte)");
		EXCEPTION_ASSERT_WITH_LOG(answer[3] == d_source, std::invalid_argument, "The destination address is not valid");
		EXCEPTION_ASSERT_WITH_LOG(answer[4] == d_destination, std::invalid_argument, "The source address is not valid");
		//unsigned char cmd = cmdbuf.constData()[5];
		unsigned  char errorcode = answer[6];
		EXCEPTION_ASSERT_WITH_LOG(errorcode == 0x00 || errorcode == 0x10, std::invalid_argument, "The command return an error");
		
		std::vector<unsigned char> buf = std::vector<unsigned char>(answer.begin() + 7, answer.end());
		size_t i = 0;
		for (; i < buf.size(); ++i)
		{
			if (buf[i] == STOP)
			{
				break;
			}
		}
		EXCEPTION_ASSERT_WITH_LOG(i < buf.size(), std::invalid_argument, "The supplied buffer is not valid (no stop byte)");
		buf.resize(i);
		std::vector<unsigned char> data = prepareDataFromDevice(buf);
		EXCEPTION_ASSERT_WITH_LOG(data.size() >= 2, std::invalid_argument, "The supplied buffer is not valid (no CRC)");
		data.insert(data.begin(), answer.begin() + 2, answer.begin() + 2 + 5);
		unsigned char first, second;
		ComputeCrcKermit(&data[0], data.size() - 2, &first, &second);
		EXCEPTION_ASSERT_WITH_LOG(data[data.size()-2] == first && data[data.size()-1] == second, std::invalid_argument, "The supplied buffer is not valid (CRC missmatch)");		
		// Remove header and CRC
		data = std::vector<unsigned char>(data.begin() + 5, data.end() - 2);

		return data;
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

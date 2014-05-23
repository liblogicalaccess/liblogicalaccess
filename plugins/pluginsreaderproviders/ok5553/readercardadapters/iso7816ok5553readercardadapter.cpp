
/**	
 * \file ok5553readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief OK5553 reader/card adapter.
 */

#include "iso7816ok5553readercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{		
	ISO7816OK5553ReaderCardAdapter::ISO7816OK5553ReaderCardAdapter()
		: ISO7816ReaderCardAdapter()
	{
		d_prefix = true;
		d_ok5553_reader_card_adapter.reset(new OK5553ReaderCardAdapter ());
	}

	ISO7816OK5553ReaderCardAdapter::~ISO7816OK5553ReaderCardAdapter()
	{
		
	}

	std::vector<unsigned char> ISO7816OK5553ReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char /*lc*/, const unsigned char* data, size_t datalen, unsigned char /*le*/)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);
		if (data != NULL && datalen > 0)
		{
			command.insert(command.end(), data, data + datalen);
		}

		return ISO7816ReaderCardAdapter::sendCommand(command);
	}

	std::vector<unsigned char> ISO7816OK5553ReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char /*lc*/, const unsigned char* data, size_t datalen)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);
		if (data != NULL && datalen > 0)
		{
			command.insert(command.end(), data, data + datalen);
		}

		return ISO7816ReaderCardAdapter::sendCommand(command);
	}	

	std::vector<unsigned char> ISO7816OK5553ReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, const unsigned char* data, size_t datalen)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);
		if (data != NULL && datalen > 0)
		{
			command.insert(command.end(), data, data + datalen);
		}

		return ISO7816ReaderCardAdapter::sendCommand(command);
	}

	std::vector<unsigned char> ISO7816OK5553ReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char /*le*/)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);

		return ISO7816ReaderCardAdapter::sendCommand(command);
	}

	std::vector<unsigned char> ISO7816OK5553ReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char /*lc*/, unsigned char /*le*/)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);

		return ISO7816ReaderCardAdapter::sendCommand(command);
	}

	std::vector<unsigned char> ISO7816OK5553ReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);
		return ISO7816ReaderCardAdapter::sendCommand(command);
	}



	std::vector<unsigned char> ISO7816OK5553ReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
	{
		char tmp[3];
		std::vector<unsigned char> data;
		data.push_back(static_cast<unsigned char>('t'));
		sprintf(tmp, "%.2X",(unsigned int)(command.size() + 1));
		data.push_back(static_cast<unsigned char>(tmp[0]));
		data.push_back(static_cast<unsigned char>(tmp[1]));
		data.push_back('0');
		data.push_back('F');
		if (d_prefix)
		{
			data.push_back('0');
			data.push_back('2');
			d_prefix = !d_prefix;
		}
		else
		{
			data.push_back('0');
			data.push_back('3');
			d_prefix = !d_prefix;
		}
		for (size_t i = 0; i < command.size(); i++)
		{
			sprintf(tmp, "%.2X", command[i]);
			data.push_back(static_cast<unsigned char>(tmp[0]));
			data.push_back(static_cast<unsigned char>(tmp[1]));
		}

		return d_ok5553_reader_card_adapter->adaptCommand(data);
	}

	std::vector<unsigned char> ISO7816OK5553ReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{
		LOG(LogLevel::COMS) << "Processing response : " << BufferHelper::getHex(answer);
		return answerReverse(handleAnswer(d_ok5553_reader_card_adapter->adaptAnswer(OK5553ReaderUnit::asciiToHex(answer))));
	}

	std::vector<unsigned char> ISO7816OK5553ReaderCardAdapter::answerReverse (const std::vector<unsigned char>& answer)
	{
		LOG(LogLevel::COMS) << "Reversing response : " << BufferHelper::getHex(answer);
		std::vector<unsigned char> res;

		if (answer.size() > 1)
		{
			res = std::vector<unsigned char>(answer.begin() + 1, answer.end());
			res.push_back (0x00);
			res.push_back (answer[0]);
		}
		else if (answer.size() == 1)
		{
			res.push_back(answer[0]);
		}
		
		return res;
	}

	std::vector<unsigned char> ISO7816OK5553ReaderCardAdapter::handleAnswer (const std::vector<unsigned char>& answer)
	{
		LOG(LogLevel::COMS) << "Handling response : " << BufferHelper::getHex(answer);
		std::vector<unsigned char> res;

		if (answer.size() > 2)
		{
			if (answer[0] == answer.size()-1)
			{
				res.insert(res.begin(), answer.begin()+2, answer.end());
			}
			else
			{
				THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The supply answer size is not good.");
			}
		}
		return res;
	}
}

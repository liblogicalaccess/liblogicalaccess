
/**	
 * \file rplethreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth reader/card adapter.
 */

#include "iso7816rplethreadercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{		
	ISO7816RplethReaderCardAdapter::ISO7816RplethReaderCardAdapter()
		: ISO7816ReaderCardAdapter()
	{
		d_prefix = true;
		d_rpleth_reader_card_adapter.reset(new RplethReaderCardAdapter ());
	}

	ISO7816RplethReaderCardAdapter::~ISO7816RplethReaderCardAdapter()
	{
		
	}

	void ISO7816RplethReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char /*lc*/, const unsigned char* data, size_t datalen, unsigned char /*le*/, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);
		if (data != NULL && datalen > 0)
		{
			command.insert(command.end(), data, data + datalen);
		}

		ISO7816ReaderCardAdapter::sendAPDUCommand(command, result, resultlen);
	}

	void ISO7816RplethReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char /*lc*/, const unsigned char* data, size_t datalen, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);
		if (data != NULL && datalen > 0)
		{
			command.insert(command.end(), data, data + datalen);
		}

		ISO7816ReaderCardAdapter::sendAPDUCommand(command, result, resultlen);
	}	

	void ISO7816RplethReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, const unsigned char* data, size_t datalen, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);
		if (data != NULL && datalen > 0)
		{
			command.insert(command.end(), data, data + datalen);
		}

		ISO7816ReaderCardAdapter::sendAPDUCommand(command, result, resultlen);
	}

	void ISO7816RplethReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char /*le*/, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);

		ISO7816ReaderCardAdapter::sendAPDUCommand(command, result, resultlen);
	}

	void ISO7816RplethReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char /*lc*/, unsigned char /*le*/, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);

		ISO7816ReaderCardAdapter::sendAPDUCommand(command, result, resultlen);
	}


	void ISO7816RplethReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char /*lc*/, const unsigned char* data, size_t datalen, unsigned char /*le*/)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);
		if (data != NULL && datalen > 0)
		{
			command.insert(command.end(), data, data + datalen);
		}
		ISO7816ReaderCardAdapter::sendAPDUCommand(command, NULL, NULL);
	}

	void ISO7816RplethReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char /*lc*/, const unsigned char* data, size_t datalen)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);
		if (data != NULL && datalen > 0)
		{
			command.insert(command.end(), data, data + datalen);
		}
		ISO7816ReaderCardAdapter::sendAPDUCommand(command, NULL, NULL);
	}

	void ISO7816RplethReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char /*le*/)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);
		ISO7816ReaderCardAdapter::sendAPDUCommand(command, NULL, NULL);
	}


	void ISO7816RplethReaderCardAdapter::sendAPDUCommand(unsigned char /*cla*/, unsigned char ins, unsigned char /*p1*/, unsigned char /*p2*/, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(ins);
		ISO7816ReaderCardAdapter::sendAPDUCommand(command, result, resultlen);
	}



	std::vector<unsigned char> ISO7816RplethReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
	{
		char tmp [3];
		std::vector<unsigned char> data;
		data.push_back(static_cast<unsigned char>('t'));
		sprintf(tmp, "%.2X",command.size()+1);
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

		return d_rpleth_reader_card_adapter->adaptCommand(data);
	}

	std::vector<unsigned char> ISO7816RplethReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{
		return answerReverse(handleAnswer(RplethReaderUnit::asciiToHex(d_rpleth_reader_card_adapter->adaptAnswer(answer))));
	}

	std::vector<unsigned char> ISO7816RplethReaderCardAdapter::answerReverse (const std::vector<unsigned char>& answer)
	{
		std::vector<unsigned char> res;

		if (answer.size() > 0)
		{
			res = std::vector<unsigned char>(answer.begin() + 1, answer.end());
			res.push_back (0x00);
			res.push_back (answer[0]);
		}
		
		return res;
	}

	std::vector<unsigned char> ISO7816RplethReaderCardAdapter::handleAnswer (const std::vector<unsigned char>& answer)
	{
		std::vector<unsigned char> res;

		if (answer.size() > 2)
		{
			if (answer[0] == answer.size())
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

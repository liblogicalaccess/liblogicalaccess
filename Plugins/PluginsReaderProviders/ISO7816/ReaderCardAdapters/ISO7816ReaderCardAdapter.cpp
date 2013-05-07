/**
 * \file ISO7816ReaderCardAdapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 reader/card adapter.
 */

#include "ISO7816ReaderCardAdapter.h"

#include <cstring>

namespace LOGICALACCESS
{
	void ISO7816ReaderCardAdapter::sendAPDUCommand(const std::vector<unsigned char>& command, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> res = sendCommand(command, 0);

		if (result != NULL && resultlen != NULL)
		{
			if (*resultlen >= res.size())
			{
				*resultlen = res.size();
				memcpy(result, &res[0], res.size());
			}
		}
	}

	void ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const unsigned char* data, size_t datalen, unsigned char le, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);
		command.push_back(lc);
		if (data != NULL && datalen > 0)
		{
			command.insert(command.end(), data, data + datalen);
		}
		command.push_back(le);

		sendAPDUCommand(command, result, resultlen);
	}

	void ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const unsigned char* data, size_t datalen, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);
		command.push_back(lc);
		if (data != NULL && datalen > 0)
		{
			command.insert(command.end(), data, data + datalen);
		}

		sendAPDUCommand(command, result, resultlen);
	}	

	void ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, const unsigned char* data, size_t datalen, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);
		if (data != NULL && datalen > 0)
		{
			command.insert(command.end(), data, data + datalen);
		}

		sendAPDUCommand(command, result, resultlen);
	}

	void ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char le, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);
		command.push_back(le);

		sendAPDUCommand(command, result, resultlen);
	}

	void ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, unsigned char le, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);
		command.push_back(lc);
		command.push_back(le);

		sendAPDUCommand(command, result, resultlen);
	}

	void ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const unsigned char* data, size_t datalen, unsigned char le)
	{
		sendAPDUCommand(cla, ins, p1, p2, lc, data, datalen, le, NULL, NULL);
	}

	void ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const unsigned char* data, size_t datalen)
	{
		sendAPDUCommand(cla, ins, p1, p2, lc, data, datalen, NULL, NULL);
	}

	void ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char le)
	{
		sendAPDUCommand(cla, ins, p1, p2, le, NULL, (size_t*)NULL);
	}

	void ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char* result, size_t* resultlen)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);

		sendAPDUCommand(command, result, resultlen);
	}
}

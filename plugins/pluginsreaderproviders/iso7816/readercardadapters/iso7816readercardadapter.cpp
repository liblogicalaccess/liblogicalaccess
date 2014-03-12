/**
 * \file iso7816readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 reader/card adapter.
 */

#include "iso7816readercardadapter.hpp"

#include <cstring>

namespace logicalaccess
{
	std::vector<unsigned char> ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const std::vector<unsigned char> data, unsigned char le)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);
		command.push_back(lc);
		command.insert(command.end(), data.begin(), data.end());
		command.push_back(le);

		return sendCommand(command);
	}

	std::vector<unsigned char> ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const std::vector<unsigned char> data)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);
		command.push_back(lc);
		command.insert(command.end(), data.begin(), data.end());

		return sendCommand(command);
	}	

	std::vector<unsigned char> ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, const std::vector<unsigned char> data)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);
		command.insert(command.end(), data.begin(), data.end());

		return sendCommand(command);
	}

	std::vector<unsigned char> ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char le)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);
		command.push_back(le);

		return sendCommand(command);
	}

	std::vector<unsigned char> ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, unsigned char le)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);
		command.push_back(lc);
		command.push_back(le);

		return sendCommand(command);
	}

	std::vector<unsigned char> ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2)
	{
		std::vector<unsigned char> command;
		command.push_back(cla);
		command.push_back(ins);
		command.push_back(p1);
		command.push_back(p2);

		return sendCommand(command);
	}
}

/**
 * \file rplethlcddisplay.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth LCD Display.
 */

#include "rplethlcddisplay.hpp"

namespace logicalaccess
{
	RplethLCDDisplay::RplethLCDDisplay()
	{
		
	}

	void RplethLCDDisplay::setMessage(std::string message)
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back (static_cast<unsigned char>(Device::LCD));
		command.push_back (static_cast<unsigned char>(LcdCommand::DISPLAY));
		command.push_back (static_cast<unsigned char>(message.size()));
		command.insert(command.end(), message.begin(), message.end());
		getRplethReaderCardAdapter()->sendRplethCommand (command, false);
	}

	void RplethLCDDisplay::setMessage(std::string message, int time)
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back (static_cast<unsigned char>(Device::LCD));
		command.push_back (static_cast<unsigned char>(LcdCommand::DISPLAYT));
		command.push_back (static_cast<unsigned char>(message.size()+1));
		command.insert(command.end(), message.begin(), message.end());
		command.push_back (static_cast<unsigned char>(time));
		getRplethReaderCardAdapter()->sendRplethCommand (command, false);
	}

	void RplethLCDDisplay::setMessage(unsigned char /*rowid*/, std::string message)
	{
		setMessage(message);
	}

	void RplethLCDDisplay::setDisplayTime(int time)
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back (static_cast<unsigned char>(Device::LCD));
		command.push_back (static_cast<unsigned char>(LcdCommand::DISPLAYTIME));
		command.push_back (static_cast<unsigned char>(0x01));
		command.push_back (static_cast<unsigned char>(time));
		getRplethReaderCardAdapter()->sendRplethCommand (command, false);
	}

	void RplethLCDDisplay::blink()
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back (static_cast<unsigned char>(Device::LCD));
		command.push_back (static_cast<unsigned char>(LcdCommand::BLINK));
		command.push_back (static_cast<unsigned char>(0x00));
		getRplethReaderCardAdapter()->sendRplethCommand (command, false);
	}

	void RplethLCDDisplay::autoScroll()
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back (static_cast<unsigned char>(Device::LCD));
		command.push_back (static_cast<unsigned char>(LcdCommand::SCROLL));
		command.push_back (static_cast<unsigned char>(0x00));
		getRplethReaderCardAdapter()->sendRplethCommand (command, false);
	}
}

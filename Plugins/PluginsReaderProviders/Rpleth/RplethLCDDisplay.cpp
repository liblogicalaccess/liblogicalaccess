/**
 * \file A3MLGM5600LCDDisplay.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief A3MLGM5600 LCD Display.
 */

#include "RplethLCDDisplay.h"

namespace LOGICALACCESS
{
	RplethLCDDisplay::RplethLCDDisplay()
	{
		
	}

	void RplethLCDDisplay::setMessage(string message)
	{
		vector<unsigned char> command;
		vector<unsigned char> answer;
		command.push_back (static_cast<unsigned char>(Device::LCD));
		command.push_back (static_cast<unsigned char>(LcdCommand::DISPLAY));
		command.push_back (static_cast<unsigned char>(message.size()));
		command.insert(command.end(), message.begin(), message.end());
		getRplethReaderCardAdapter()->sendCommand (command, 0);
	}

	void RplethLCDDisplay::setMessage(string message, int time)
	{
		vector<unsigned char> command;
		vector<unsigned char> answer;
		command.push_back (static_cast<unsigned char>(Device::LCD));
		command.push_back (static_cast<unsigned char>(LcdCommand::DISPLAYT));
		command.push_back (static_cast<unsigned char>(message.size()+1));
		command.insert(command.end(), message.begin(), message.end());
		command.push_back (static_cast<unsigned char>(time));
		getRplethReaderCardAdapter()->sendCommand (command, 0);
	}

	void RplethLCDDisplay::setDisplayTime(int time)
	{
		vector<unsigned char> command;
		vector<unsigned char> answer;
		command.push_back (static_cast<unsigned char>(Device::LCD));
		command.push_back (static_cast<unsigned char>(LcdCommand::DISPLAYTIME));
		command.push_back (static_cast<unsigned char>(0x01));
		command.push_back (static_cast<unsigned char>(time));
		getRplethReaderCardAdapter()->sendCommand (command, 0);
	}

	void RplethLCDDisplay::blink()
	{
		vector<unsigned char> command;
		vector<unsigned char> answer;
		command.push_back (static_cast<unsigned char>(Device::LCD));
		command.push_back (static_cast<unsigned char>(LcdCommand::BLINK));
		command.push_back (static_cast<unsigned char>(0x00));
		getRplethReaderCardAdapter()->sendCommand (command, 0);
	}

	void RplethLCDDisplay::autoScroll()
	{
		vector<unsigned char> command;
		vector<unsigned char> answer;
		command.push_back (static_cast<unsigned char>(Device::LCD));
		command.push_back (static_cast<unsigned char>(LcdCommand::SCROLL));
		command.push_back (static_cast<unsigned char>(0x00));
		getRplethReaderCardAdapter()->sendCommand (command, 0);
	}
}

/**
 * \file rplethledbuzzerdisplay.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth LED/Buzzer Display.
 */

#include "rplethledbuzzerdisplay.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
	RplethLEDBuzzerDisplay::RplethLEDBuzzerDisplay()
		: LEDBuzzerDisplay()
	{
		d_red_led = false;
		d_green_led = false;
		d_buzzer = false;
	}

	void RplethLEDBuzzerDisplay::setRedLed(bool status)
	{
		d_red_led = status;
		setLED();
	}

	void RplethLEDBuzzerDisplay::setGreenLed(bool status)
	{
		d_green_led = status;
		setLED();
	}

	void RplethLEDBuzzerDisplay::setBuzzer(bool status)
	{
		d_buzzer = status;
		
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::BEEP));
		command.push_back(static_cast<unsigned char>(0x01));
		command.push_back(static_cast<unsigned char>(status));
		getRplethReaderCardAdapter()->sendRplethCommand(command);
	}

	void RplethLEDBuzzerDisplay::setLED()
	{
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(Device::HID));
		if (d_green_led)
			command.push_back(static_cast<unsigned char>(HidCommand::BLINKLED1));
		else
			command.push_back(static_cast<unsigned char>(HidCommand::BLINKLED2));
		command.push_back(static_cast<unsigned char>(0x00));
		getRplethReaderCardAdapter()->sendRplethCommand(command);
	}
}

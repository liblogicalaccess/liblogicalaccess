/**
 * \file SmartIDLEDBuzzerDisplay.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief SmartID LED/Buzzer Display.
 */

#include "SmartIDLEDBuzzerDisplay.h"

namespace LOGICALACCESS
{
	SmartIDLEDBuzzerDisplay::SmartIDLEDBuzzerDisplay()
		: LEDBuzzerDisplay()
	{
		d_red_led = false;
		d_green_led = false;
		d_buzzer = false;
	}	

	void SmartIDLEDBuzzerDisplay::setGreenLed(bool status)
	{
		setGreenLed(status, false);
	}

	void SmartIDLEDBuzzerDisplay::setRedLed(bool status)
	{
		setRedLed(status, false);
	}

	void SmartIDLEDBuzzerDisplay::setBuzzer(bool status)
	{
		setBuzzer(status, false);
	}

	void SmartIDLEDBuzzerDisplay::setPort()
	{
		std::vector<unsigned char> data;
		data.push_back(static_cast<unsigned char>((d_red_led ? 1 : 0) | (d_green_led ? 16 : 0) | (d_buzzer ? 2 : 0)));

		getSmartIDReaderCardAdapter()->sendCommand(0x54, data);
	}

	void SmartIDLEDBuzzerDisplay::setPort(bool red, bool green, bool buzzer)
	{
		d_red_led = red;
		d_green_led = green;
		d_buzzer = buzzer;

		setPort();
	}

	void SmartIDLEDBuzzerDisplay::setRedLed(bool status, bool deferred)
	{
		d_red_led = status;

		if (!deferred)
		{
			setPort();
		}
	}

	void SmartIDLEDBuzzerDisplay::setGreenLed(bool status, bool deferred)
	{
		d_green_led = status;

		if (!deferred)
		{
			setPort();
		}
	}

	void SmartIDLEDBuzzerDisplay::setBuzzer(bool status, bool deferred)
	{
		d_buzzer = status;

		if (!deferred)
		{
			setPort();
		}
	}
}

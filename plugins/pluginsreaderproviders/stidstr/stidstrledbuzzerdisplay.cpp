/**
 * \file stidstrledbuzzerdisplay.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR LED/Buzzer Display.
 */

#include "stidstrledbuzzerdisplay.hpp"

namespace logicalaccess
{
	STidSTRLEDBuzzerDisplay::STidSTRLEDBuzzerDisplay()
		: LEDBuzzerDisplay()
	{
		//INFO_("Constructor");
		d_red_led = false;
		d_green_led = false;
		d_buzzer = false;
	}	

	void STidSTRLEDBuzzerDisplay::setGreenLed(bool status)
	{
		setGreenLed(status, false);
	}

	void STidSTRLEDBuzzerDisplay::setRedLed(bool status)
	{
		setRedLed(status, false);
	}

	void STidSTRLEDBuzzerDisplay::setBuzzer(bool /*status*/)
	{
		WARNING_SIMPLE_("Buzzer not implement on STid STR");
		// No buzzer on STid STR, do nothing
	}

	void STidSTRLEDBuzzerDisplay::setPort()
	{
		INFO_("Sending LED/Buzzer command for: red{%d} green{%d} buzzer{%d}...", d_red_led, d_green_led, d_buzzer);
		std::vector<unsigned char> data;
		data.push_back(static_cast<unsigned char>((d_red_led ? 2 : 0) | (d_green_led ? 1 : 0)));
		data.push_back(0xFF); // LED duration
		data.push_back((d_buzzer) ? 0xFF : 0x00); // Buzzer duration

		unsigned char statusCode;
		getSTidSTRReaderCardAdapter()->sendCommand(0x0007, data, statusCode);
	}

	void STidSTRLEDBuzzerDisplay::setPort(bool red, bool green, bool buzzer)
	{
		d_red_led = red;
		d_green_led = green;
		d_buzzer = buzzer;

		setPort();
	}

	void STidSTRLEDBuzzerDisplay::setRedLed(bool status, bool deferred)
	{
		INFO_("Set Red LED {%d} deferred{%d}", status, deferred);
		d_red_led = status;

		if (!deferred)
		{
			setPort();
		}
	}

	void STidSTRLEDBuzzerDisplay::setGreenLed(bool status, bool deferred)
	{
		INFO_("Set Green LED {%d} deferred{%d}", status, deferred);
		d_green_led = status;

		if (!deferred)
		{
			setPort();
		}
	}

	void STidSTRLEDBuzzerDisplay::setBuzzer(bool status, bool deferred)
	{
		INFO_("Set Buzzer {%d} deferred{%d}", status, deferred);
		d_buzzer = status;

		if (!deferred)
		{
			setPort();
		}
	}
}

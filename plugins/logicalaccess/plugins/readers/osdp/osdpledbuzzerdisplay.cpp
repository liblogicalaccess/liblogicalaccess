/**
* \file osdpledbuzzerdisplay.cpp
* \author Maxime C. <maxime@leosac.com>
* \brief OSDP LED/Buzzer Display.
*/

#include <logicalaccess/plugins/readers/osdp/osdpledbuzzerdisplay.hpp>
#include <logicalaccess/plugins/readers/osdp/readercardadapters/osdpreadercardadapter.hpp>
#include <logicalaccess/plugins/readers/osdp/osdpcommands.hpp>
#include <logicalaccess/resultchecker.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
OSDPLEDBuzzerDisplay::OSDPLEDBuzzerDisplay()
    : LEDBuzzerDisplay()
{
}

void OSDPLEDBuzzerDisplay::setRedLed(bool status)
{
    setLED(status ? OSDPColor::Red : OSDPColor::Black);
}

void OSDPLEDBuzzerDisplay::setGreenLed(bool status)
{
    setLED(status ? OSDPColor::Green : OSDPColor::Black);
}

void OSDPLEDBuzzerDisplay::setLED(OSDPColor color)
{
    s_led_cmd led;
    led.reader = getOSDPReaderCardAdapter()->getRS485Address();
    led.ledNumber = 0;
    led.tempControlCode = TemporaryControleCode::NOP;
    led.tempOnTime = 0;
    led.tempOffTime = 0;
    led.tempOnColor = OSDPColor::Black;
    led.tempOffColor = OSDPColor::Black;
    led.timerLSB = 0x00;
    led.timerMSB = 0x00;
    led.permControlCode = PermanentControlCode::SetPermanentState;
    led.permOnTime = 0;
    led.permOffTime = 0;
    led.permOnColor = color;
    led.permOffColor = color;
    getOSDPReaderCardAdapter()->getOSDPCommands()->led(led);
}

void OSDPLEDBuzzerDisplay::setBuzzer(bool status)
{
    if (status)
    {
        s_buz_cmd buz;
        buz.reader = getOSDPReaderCardAdapter()->getRS485Address();
        buz.toneCode = 0x00;
        buz.onTime = 10; // x * 100ms
        buz.offTime = 0;
        buz.count = 1;
        getOSDPReaderCardAdapter()->getOSDPCommands()->buz(buz);
    }
}

std::shared_ptr<OSDPReaderCardAdapter>
OSDPLEDBuzzerDisplay::getOSDPReaderCardAdapter() const
{
    return std::dynamic_pointer_cast<OSDPReaderCardAdapter>(getReaderCardAdapter());
}
}
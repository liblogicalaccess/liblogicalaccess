/**
 * \file rplethledbuzzerdisplay.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth LED/Buzzer Display.
 */

#include "rplethledbuzzerdisplay.hpp"
#include "readercardadapters/rplethreadercardadapter.hpp"

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
        setLED(REDLED, status);
    }

    void RplethLEDBuzzerDisplay::setGreenLed(bool status)
    {
        d_green_led = status;
        setLED(GREENLED, status);
    }

    void RplethLEDBuzzerDisplay::setBuzzer(bool status)
    {
        d_buzzer = status;

        ByteVector command;
        command.push_back(static_cast<unsigned char>(HID));
        command.push_back(static_cast<unsigned char>(BEEP));
        command.push_back(static_cast<unsigned char>(0x01));
        command.push_back(static_cast<unsigned char>(status));
        getRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }

    void RplethLEDBuzzerDisplay::setLED(HidCommand led, bool status) const
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(HID));
        command.push_back(static_cast<unsigned char>(led));
        command.push_back(static_cast<unsigned char>(0x01));
        command.push_back(static_cast<unsigned char>(status));
        getRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }

    std::shared_ptr<RplethReaderCardAdapter> RplethLEDBuzzerDisplay::getRplethReaderCardAdapter() const
    { return std::dynamic_pointer_cast<RplethReaderCardAdapter>(getReaderCardAdapter()); }
}
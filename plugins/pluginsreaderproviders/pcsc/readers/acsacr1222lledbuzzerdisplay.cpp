/**
* \file acsacr1222lledbuzzerdisplay.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief ACS ACR 1222L LED/Buzzer Display.
*/

#include "acsacr1222lledbuzzerdisplay.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"
#include "logicalaccess/resultchecker.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
    ACSACR1222LLEDBuzzerDisplay::ACSACR1222LLEDBuzzerDisplay()
        : LEDBuzzerDisplay()
    {
        d_red_led = false;
        d_green_led = false;
        d_blue_led = false;
        d_orange_led = false;
    }

    void ACSACR1222LLEDBuzzerDisplay::setRedLed(bool status)
    {
        d_red_led = status;
        setLED();
    }

    void ACSACR1222LLEDBuzzerDisplay::setGreenLed(bool status)
    {
        d_green_led = status;
        setLED();
    }

    void ACSACR1222LLEDBuzzerDisplay::setBlueLed(bool status)
    {
        d_blue_led = status;
        setLED();
    }

    void ACSACR1222LLEDBuzzerDisplay::setOrangeLed(bool status)
    {
        d_orange_led = status;
        setLED();
    }

    void ACSACR1222LLEDBuzzerDisplay::setBuzzer(bool status)
    {
        if (status)
        {
            std::shared_ptr<ResultChecker> checker = getPCSCReaderCardAdapter()->getResultChecker();
            getPCSCReaderCardAdapter()->setResultChecker(std::shared_ptr<ResultChecker>());

            std::vector<unsigned char> data;
            data.push_back(0x10);   // x * 100ms

            getPCSCReaderCardAdapter()->sendAPDUCommand(0xE0, 0x00, 0x00, 0x28, data);

            getPCSCReaderCardAdapter()->setResultChecker(checker);
        }
    }

    void ACSACR1222LLEDBuzzerDisplay::setLED()
    {
        unsigned char p1 = 0x00;
        if (d_green_led)
            p1 |= 0x01;
        if (d_blue_led)
            p1 |= 0x02;
        if (d_orange_led)
            p1 |= 0x04;
        if (d_red_led)
            p1 |= 0x08;

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x00, 0x44, p1, 0x00);
    }
}
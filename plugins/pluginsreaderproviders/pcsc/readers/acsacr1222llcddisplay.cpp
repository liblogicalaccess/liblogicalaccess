/**
* \file acsacr1222llcddisplay.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief ACS ACR 1222L LCD Display.
*/

#include "acsacr1222llcddisplay.hpp"

namespace logicalaccess
{
    ACSACR1222LLCDDisplay::ACSACR1222LLCDDisplay()
    {
    }

    void ACSACR1222LLCDDisplay::setMessage(std::string message)
    {
        LLA_LOG_CTX("ACS_ACR_1222L SetMessage");
		LOG(DEBUGS) << "HOHO";
        clear();

        for (unsigned char i = 0; i < 2 && (i * 15) < static_cast<unsigned char>(message.size()); i++)
        {
            setMessage(i, message.substr(i * 15, (i + 1) * 15));
        }
    }

    void ACSACR1222LLCDDisplay::setMessage(unsigned char rowid, std::string message)
    {
        std::vector<unsigned char> data;
        // The first character will not be displayed on the LCD screen :(
        data.push_back(' ');

        data.insert(data.end(), message.begin(), message.end());
        // Make sure we don't overflow the LCD line
        if (data.size() > 16)
        {
            data = std::vector<unsigned char>(data.begin(), data.begin() + 16);
        }
        else // or underflow it (resulting in potential garbage on screen)
        {
            while (data.size() < 16)
            {
                data.push_back(' ');
            }
        }

        // Always use fonts Set A, not bold.
        unsigned char option = 0x00;
        unsigned char position = (rowid ? 0x40 : 0x00);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, option, 0x68, position, data);
    }

    void ACSACR1222LLCDDisplay::clear()
    {
        LLA_LOG_CTX("ACS_ACR_1222L Clearing LCD screen ");
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x00, 0x60, 0x00, 0x00);
    }

    void ACSACR1222LLCDDisplay::scroll(unsigned char x, unsigned char y, unsigned char xRange, unsigned char yRange, unsigned char speedPixel, LCDScrollPeriod speedPeriod, LCDScrollDirection direction)
    {
        std::vector<unsigned char> data;
        data.push_back(x);
        data.push_back(y);
        data.push_back(xRange);
        data.push_back(yRange);
        data.push_back(speedPixel | (speedPeriod << 4));
        data.push_back(direction);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x00, 0x6D, 0x00, data);
    }

    void ACSACR1222LLCDDisplay::pauseScrolling()
    {
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x00, 0x6E, 0x00, 0x00);
    }

    void ACSACR1222LLCDDisplay::stopScrolling()
    {
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x00, 0x6F, 0x00, 0x00);
    }

    void ACSACR1222LLCDDisplay::setContrast(unsigned char value)
    {
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x00, 0x6C, value, 0x00);
    }

    void ACSACR1222LLCDDisplay::setBacklight(bool enable)
    {
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x00, 0x64, (enable ? 0xff : 0x00), 0x00);
    }
}
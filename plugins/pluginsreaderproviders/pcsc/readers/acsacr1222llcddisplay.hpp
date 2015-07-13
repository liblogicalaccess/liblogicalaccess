/**
* \file acsacr1222llcddisplay.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief ACS ACR 1222L LCD Display.
*/

#ifndef LOGICALACCESS_ACSACR1222LLCDDISPLAY_HPP
#define LOGICALACCESS_ACSACR1222LLCDDISPLAY_HPP

#include "logicalaccess/readerproviders/lcddisplay.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
    /**
    *\brief The LCD scrolling speed period
    */
    typedef enum {
        ScrollPeriodUnit1 = 0x00,
        ScrollPeriodUnit3 = 0x01,
        ScrollPeriodUnit5 = 0x02,
        ScrollPeriodUnit7 = 0x03,
        ScrollPeriodUnit17 = 0x04,
        ScrollPeriodUnit19 = 0x05,
        ScrollPeriodUnit21 = 0x06,
        ScrollPeriodUnit23 = 0x07,
        ScrollPeriodUnit129 = 0x08,
        ScrollPeriodUnit131 = 0x09,
        ScrollPeriodUnit133 = 0x10,
        ScrollPeriodUnit135 = 0x11,
        ScrollPeriodUnit145 = 0x12,
        ScrollPeriodUnit147 = 0x13,
        ScrollPeriodUnit149 = 0x14,
        ScrollPeriodUnit151 = 0x15
    } LCDScrollPeriod;

    /**
    *\brief The LCD scrolling direction
    */
    typedef enum {
        ScrollFromLeftToRight = 0,
        ScrollFromRightToLeft = 1,
        ScrollFromTopToBottom = 2,
        ScrollFromBottomToTop = 3
    } LCDScrollDirection;

    /**
    * \brief A ACS ACR 1222L LCD Display class.
    */
    class LIBLOGICALACCESS_API ACSACR1222LLCDDisplay : public LCDDisplay
    {
    public:

        /**
        * \brief Constructor.
        */
        ACSACR1222LLCDDisplay();

        /**
        * \brief Write a message on screen.
        * \param message The message to show.
        */
        virtual void setMessage(std::string message);

        /**
        * \brief Write a message on screen.
        * \param rowid The row id for the message.
        * \param message The message to show.
        */
        virtual void setMessage(unsigned char rowid, std::string message);

        std::shared_ptr<PCSCReaderCardAdapter> getPCSCReaderCardAdapter() { return std::dynamic_pointer_cast<PCSCReaderCardAdapter>(getReaderCardAdapter()); };

        void clear();

        void scroll(unsigned char x = 0, unsigned char y = 0, unsigned char xRange = 128, unsigned char yRange = 32, unsigned char speedPixel = 2, LCDScrollPeriod speedPeriod = ScrollPeriodUnit23, LCDScrollDirection direction = ScrollFromLeftToRight);

        void pauseScrolling();

        void stopScrolling();

        void setContrast(unsigned char value);

        void setBacklight(bool enable);
    };
}

#endif /* LOGICALACCESS_ACSACR1222LLCDDISPLAY_HPP */
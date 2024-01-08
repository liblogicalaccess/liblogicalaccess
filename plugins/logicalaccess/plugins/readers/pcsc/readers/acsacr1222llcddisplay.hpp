/**
* \file acsacr1222llcddisplay.hpp
* \author Maxime C. <maxime@leosac.com>
* \brief ACS ACR 1222L LCD Display.
*/

#ifndef LOGICALACCESS_ACSACR1222LLCDDISPLAY_HPP
#define LOGICALACCESS_ACSACR1222LLCDDISPLAY_HPP

#include <logicalaccess/readerproviders/lcddisplay.hpp>
#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>

#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
/**
*\brief The LCD scrolling speed period
*/
typedef enum {
    ScrollPeriodUnit1   = 0x00,
    ScrollPeriodUnit3   = 0x01,
    ScrollPeriodUnit5   = 0x02,
    ScrollPeriodUnit7   = 0x03,
    ScrollPeriodUnit17  = 0x04,
    ScrollPeriodUnit19  = 0x05,
    ScrollPeriodUnit21  = 0x06,
    ScrollPeriodUnit23  = 0x07,
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
class LLA_READERS_PCSC_API ACSACR1222LLCDDisplay : public LCDDisplay
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
    void setMessage(std::string message) override;

    /**
    * \brief Write a message on screen.
    * \param rowid The row id for the message.
    * \param message The message to show.
    */
    void setMessage(unsigned char rowid, std::string message) override;

    std::shared_ptr<PCSCReaderCardAdapter> getPCSCReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<PCSCReaderCardAdapter>(getReaderCardAdapter());
    }

    /**
     * The implementation simply write to line of SPACE character.
     */
    void clear() override;

    void scroll(unsigned char x = 0, unsigned char y = 0, unsigned char xRange = 128,
                unsigned char yRange = 32, unsigned char speedPixel = 2,
                LCDScrollPeriod speedPeriod  = ScrollPeriodUnit23,
                LCDScrollDirection direction = ScrollFromLeftToRight) const;

    void pauseScrolling() const;

    void stopScrolling() const;

    void setContrast(unsigned char value) const;

    void setBacklight(bool enable) const;
};
}

#endif /* LOGICALACCESS_ACSACR1222LLCDDISPLAY_HPP */
/**
* \file acsacr1222lledbuzzerdisplay.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief ACS ACR 1222L LED/Buzzer Display.
*/

#ifndef LOGICALACCESS_ACSACR1222LLEDBUZZERDISPLAY_HPP
#define LOGICALACCESS_ACSACR1222LLEDBUZZERDISPLAY_HPP

#include "logicalaccess/logicalaccess_api.hpp"
#include "logicalaccess/readerproviders/ledbuzzerdisplay.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"
#include <string>
#include <vector>

namespace logicalaccess
{
    /**
    * \brief A ACS ACR 1222L LED/Buzzer Display class.
    */
    class LIBLOGICALACCESS_API ACSACR1222LLEDBuzzerDisplay : public LEDBuzzerDisplay
    {
    public:
        /**
        * \brief Constructor.
        */
        ACSACR1222LLEDBuzzerDisplay();

        /**
        * \brief Set the green led to a status.
        * \param status True to show the green led, false otherwise.
        */
        virtual void setGreenLed(bool status);

        /**
        * \brief Set the red led to a status.
        * \param status True to show the red led, false otherwise.
        */
        virtual void setRedLed(bool status);

        /**
        * \brief Set the blue led to a status.
        * \param status True to show the blue led, false otherwise.
        */
        virtual void setBlueLed(bool status);

        /**
        * \brief Set the orange led to a status.
        * \param status True to show the orange led, false otherwise.
        */
        virtual void setOrangeLed(bool status);

        /**
        * \brief Set the buzzer to a status.
        * \param status True to play the buzzer, false otherwise.
        */
        virtual void setBuzzer(bool status);

        /**
        * \brief Upload led status.
        * \see setRedLed
        * \see setGreenLed
        */
        void setLED();

        std::shared_ptr<PCSCReaderCardAdapter> getPCSCReaderCardAdapter() { return std::dynamic_pointer_cast<PCSCReaderCardAdapter>(getReaderCardAdapter()); };

    protected:

        /**
        * \brief The red led status.
        */
        bool d_red_led;

        /**
        * \brief The green led status.
        */
        bool d_green_led;

        /**
        * \brief The blue led status.
        */
        bool d_blue_led;

        /**
        * \brief The orange led status.
        */
        bool d_orange_led;
    };
}

#endif /* LOGICALACCESS_ACSACR1222LLEDBUZZERDISPLAY_HPP */
/**
 * \file a3mlgm5600ledbuzzerdisplay.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 LED/Buzzer Display.
 */

#ifndef LOGICALACCESS_A3MLGM5600LEDBUZZERDISPLAY_HPP
#define LOGICALACCESS_A3MLGM5600LEDBUZZERDISPLAY_HPP

#include "logicalaccess/logicalaccess_api.hpp"
#include "logicalaccess/readerproviders/ledbuzzerdisplay.hpp"
#include "a3mlgm5600_fwd.hpp"
#include <string>
#include <vector>

namespace logicalaccess
{
    /**
     * \brief A A3MLGM5600 LED/Buzzer Display class.
     */
    class LIBLOGICALACCESS_API A3MLGM5600LEDBuzzerDisplay : public LEDBuzzerDisplay
    {
    public:
        /**
         * \brief Constructor.
         */
        A3MLGM5600LEDBuzzerDisplay();

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

        std::shared_ptr<A3MLGM5600ReaderCardAdapter> getA3MLGM5600ReaderCardAdapter() { return std::dynamic_pointer_cast<A3MLGM5600ReaderCardAdapter>(getReaderCardAdapter()); };

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
         * \brief The buzzer status.
         */
        bool d_buzzer;
    };
}

#endif /* LOGICALACCESS_A3MLGM5600LEDBUZZERDISPLAY_HPP */
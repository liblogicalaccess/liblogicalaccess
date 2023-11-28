/**
 * \file stidstrledbuzzerdisplay.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief STidSTR LED/Buzzer Display.
 */

#ifndef LOGICALACCESS_STIDSTRLEDBUZZERDISPLAY_HPP
#define LOGICALACCESS_STIDSTRLEDBUZZERDISPLAY_HPP

#include <string>
#include <vector>
#include <logicalaccess/readerproviders/ledbuzzerdisplay.hpp>
#include <logicalaccess/plugins/readers/stidstr/stidstr_fwd.hpp>

namespace logicalaccess
{
/**
 * \brief A STidSTR LED/Buzzer Display class.
 */
class LLA_READERS_STIDSTR_API STidSTRLEDBuzzerDisplay : public LEDBuzzerDisplay
{
  public:
    /**
     * \brief Constructor.
     */
    STidSTRLEDBuzzerDisplay();

    /**
     * \brief Set the green led to a status.
     * \param status True to show the green led, false otherwise.
     */
    void setGreenLed(bool status) override;

    /**
     * \brief Set the red led to a status.
     * \param status True to show the red led, false otherwise.
     */
    void setRedLed(bool status) override;

    /**
     * \brief Set the buzzer to a status.
     * \param status True to play the buzzer, false otherwise.
     */
    void setBuzzer(bool status) override;

    /**
     * \brief Set the red led status.
     * \param status true if the led must be switched on, false if it must be switched
     * down.
     * \param deferred if set to true, the led status won't be changed until the next call
     * to setPort() or the next call to setRedLed(), setGreenLed() or setBuzzer() with the
     * deferred parameter set to false. Default is false.
     */
    void setRedLed(bool status, bool deferred);

    /**
     * \brief Set the green led status.
     * \param status true if the led must be switched on, false if it must be switched
     * down.
     * \param deferred if set to true, the led status won't be changed until the next call
     * to setPort() or the next call to setRedLed(), setGreenLed() or setBuzzer() with the
     * deferred parameter set to false. Default is false.
     */
    void setGreenLed(bool status, bool deferred);

    /**
     * \brief Set the buzzer status.
     * \param status true if the buzzer must be turned on, false if it must be turned
     * down.
     * \param deferred if set to true, the buzzer status won't be changed until the next
     * call to setPort() or the next call to setRedLed(), setGreenLed() or setBuzzer()
     * with the deferred parameter set to false. Default is false.
     */
    void setBuzzer(bool status, bool deferred);

    /**
     * \brief Upload led and buzzer status.
     * \see setRedLed
     * \see setGreenLed
     * \see setBuzzer
     */
    void setPort() const;

    /**
     * \brief Upload led and buzzer status.
     * \param red The red led status.
     * \param green The green led status.
     * \param buzzer The buzzer status.
     * \see setRedLed
     * \see setGreenLed
     * \see setBuzzer
     */
    void setPort(bool red, bool green, bool buzzer);

    std::shared_ptr<STidSTRReaderCardAdapter> getSTidSTRReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<STidSTRReaderCardAdapter>(
            getReaderCardAdapter());
    }

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

#endif /* LOGICALACCESS_STIDSTRLEDBUZZERDISPLAY_H */
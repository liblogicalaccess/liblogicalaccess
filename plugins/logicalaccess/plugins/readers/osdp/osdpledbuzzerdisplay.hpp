/**
* \file osdpledbuzzerdisplay.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief OSDP LED/Buzzer Display.
*/

#ifndef LOGICALACCESS_OSDPLEDBUZZERDISPLAY_HPP
#define LOGICALACCESS_OSDPLEDBUZZERDISPLAY_HPP


#include <logicalaccess/readerproviders/ledbuzzerdisplay.hpp>
#include <logicalaccess/plugins/readers/osdp/readercardadapters/osdpreadercardadapter.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
/**
* \brief A OSDP LED/Buzzer Display class.
*/
class LLA_READERS_OSDP_API OSDPLEDBuzzerDisplay : public LEDBuzzerDisplay
{
  public:
    /**
    * \brief Constructor.
    */
    OSDPLEDBuzzerDisplay();

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
    
    void setLED(OSDPColor color);

    /**
    * \brief Set the buzzer to a status.
    * \param status True to play the buzzer, false otherwise.
    */
    void setBuzzer(bool status) override;

    std::shared_ptr<OSDPReaderCardAdapter> getOSDPReaderCardAdapter() const;
};
}

#endif /* LOGICALACCESS_OSDPLEDBUZZERDISPLAY_HPP */
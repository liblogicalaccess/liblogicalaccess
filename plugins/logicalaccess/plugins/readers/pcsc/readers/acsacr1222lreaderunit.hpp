/**
* \file acsacrreaderunit.hpp
* \author Maxime C. <maxime@leosac.com>
* \brief ACS ACR Reader unit.
*/

#ifndef LOGICALACCESS_ACSACR1222LREADERUNIT_HPP
#define LOGICALACCESS_ACSACR1222LREADERUNIT_HPP

#include <logicalaccess/plugins/readers/pcsc/readers/acsacrreaderunit.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/acsacr1222lreaderunitconfiguration.hpp>

namespace logicalaccess
{
/**k
* \brief The ACS ACR 1222L reader unit class.
*/
class LLA_READERS_PCSC_API ACSACR1222LReaderUnit : public PCSCReaderUnit
{
  public:
    /**
    * \brief Constructor.
    */
    explicit ACSACR1222LReaderUnit(const std::string &name);

    /**
    * \brief Destructor.
    */
    virtual ~ACSACR1222LReaderUnit();

    /**
    * \brief Get the PC/SC reader unit type.
    * \return The PC/SC reader unit type.
    */
    PCSCReaderUnitType getPCSCType() const override;

    /**
    * \brief Get a string hexadecimal representation of the reader serial number
    * \return The reader serial number or an empty string on error.
    */
    std::string getReaderSerialNumber() override;

    /**
    * \brief Connect to the card.
    * \return True if the card was connected without error, false otherwise.
    *
    * If the card handle was already connected, connect() first call disconnect(). If you
    * intend to do a reconnection, call reconnect() instead.
    */
    bool connect() override;

    /**
     * \brief Set the default reader LED and Buzzer behavior.
     * \param showPICCPollingStatus Show the PICC polling status.
     * \param beepOnCardEvent Make a beep whenever a card insertion or removal event is
     * detected.
     * \param beepOnChipReset Make a beep when the contactless chip is reset.
     * \param blinkOnCardOperation Blink the LED whenever the card is being accessed.
     */
    void setDefaultLEDBuzzerBehavior(bool showPICCPollingStatus, bool beepOnCardEvent,
                                     bool beepOnChipReset, bool blinkOnCardOperation);

    /**
    * \brief Get the current default reader LED and Buzzer behavior.
    * \param showPICCPollingStatus Show the PICC polling status.
    * \param beepOnCardEvent Make a beep whenever a card insertion or removal event is
    * detected.
    * \param beepOnChipReset Make a beep when the contactless chip is reset.
    * \param blinkOnCardOperation Blink the LED whenever the card is being accessed.
    */
    void getDefaultLEDBuzzerBehavior(bool &showPICCPollingStatus, bool &beepOnCardEvent,
                                     bool &beepOnChipReset, bool &blinkOnCardOperation);

    /**
     * Query the reader to retrieve the firmware version.
     */
    std::string getFirmwareVersion();

    /**
     * Reimplemented so we can enable a "no-card" connection.
     */
    bool waitRemoval(unsigned int maxwait) override;

    /**
     * Reimplemented so we can enable a "no-card" connection.
     */
    bool connectToReader() override;

    /**
    * Reimplemented so we can enable a "no-card" connection.
    */
    void disconnectFromReader() override;

    /**
    * We establish a perma connection with one of 3 SAM available
    * on the card reader.
    * We have to overload this method to return the handle
    * managed by the readerunit corresponding to our "perma connection SAM".
    */
    std::shared_ptr<LCDDisplay> getLCDDisplay() override;

    /**
     * Same situation than getLCDDisplay
     */
    std::shared_ptr<LEDBuzzerDisplay> getLEDBuzzerDisplay() override;

    std::shared_ptr<ACSACR1222LReaderUnitConfiguration> getACSACR1222LConfiguration();

  protected:
    std::shared_ptr<PCSCReaderCardAdapter> getReaderControlReaderCardAdapter();
    std::shared_ptr<PCSCReaderCardAdapter> d_readerControlReaderCardAdapter;

    std::shared_ptr<ResultChecker> createDefaultResultChecker() const override;

  private:
    void establish_background_connection();
    void kill_background_connection();
    std::shared_ptr<PCSCReaderUnit> sam_used_as_perma_connection_;
};
}

#endif
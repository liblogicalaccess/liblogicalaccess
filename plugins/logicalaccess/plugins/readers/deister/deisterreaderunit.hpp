/**
 * \file deisterreaderunit.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Deister Reader unit.
 */

#ifndef LOGICALACCESS_DEISTERREADERUNIT_HPP
#define LOGICALACCESS_DEISTERREADERUNIT_HPP

#include <logicalaccess/readerproviders/readerunit.hpp>
#include <logicalaccess/plugins/readers/deister/deisterreaderunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/deister/lla_readers_deister_api.hpp>

namespace logicalaccess
{
class Profile;
class DeisterReaderCardAdapter;
class DeisterReaderProvider;

/**
 * \brief The deister card types.
 */
typedef enum {
    DCT_UNKNOWN           = 0x00, /**< Unknown */
    DCT_MIFARE            = 0x14, /**< ISO14443A mifare  */
    DCT_MIFARE_ULTRALIGHT = 0x15, /**< ISO14443A mifare UltraLight  */
    DCT_DESFIRE           = 0x16, /**< ISO14443A mifare DESFire  */
    DCT_ICODE1            = 0x01, /**< ICode 1  */
    DCT_STM_LRI_512       = 0x02, /**< ISO15693 STM LRI 512  */
    DCT_TAGIT             = 0x03, /**< Tag-It  */
    DCT_ICODE2            = 0x04, /**< ISO15693 iCode 2 (SLI)  */
    DCT_INFINEON_MYD      = 0x05, /**< ISO15693 Infineon my-d  */
    DCT_EM4135            = 0x06, /**< ISO15693 EM4135  */
    DCT_TAGIT_HFI         = 0x07, /**< ISO15693 TagIt (HFI)  */
    DCT_GENERIC_ISO15693  = 0x08, /**< Other ISO15693 Transponders  */
    DCT_ICLASS            = 0x18, /**< Inside/iClass  */
    DCT_GENERIC_ISO14443B = 0x1D, /**< ISO14443B  */
    DCT_EM4102            = 0x1B, /**< 125Khz EM4102  */
    DCT_PROXLITE          = 0x1A, /**< 125Khz ProxLite  */
    DCT_SMARTFRAME        = 0x1C, /**< 125Khz Sperrbit SmartFrame  */
    DCT_PROX              = 0x19  /**< 125Khz HID Prox  */
} DeisterCardType;

/**
 * \brief The Deister reader unit class. This reader support DeBus protocol.
 */
class LLA_READERS_DEISTER_API DeisterReaderUnit : public ReaderUnit
{
  public:
    /**
     * \brief Constructor.
     */
    DeisterReaderUnit();

    /**
     * \brief Destructor.
     */
    virtual ~DeisterReaderUnit();

    /**
     * \brief Get the reader unit name.
     * \return The reader unit name.
     */
    std::string getName() const override;

    /**
     * \brief Get the connected reader unit name.
     * \return The connected reader unit name.
     */
    std::string getConnectedName() override;

    /**
     * \brief Set the card type.
     * \param cardType The card type.
     */
    void setCardType(std::string cardType) override;

    /**
     * \brief Wait for a card insertion.
     * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero,
     * then the call never times out.
     * \return True if a card was inserted, false otherwise. If a card was inserted, the
     * name of the reader on which the insertion was detected is accessible with
     * getReader().
     * \warning If the card is already connected, then the method always fail.
     */
    bool waitInsertion(unsigned int maxwait) override;

    /**
     * \brief Wait for a card removal.
     * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero,
     * then the call never times out.
     * \return True if a card was removed, false otherwise. If a card was removed, the
     * name of the reader on which the removal was detected is accessible with
     * getReader().
     */
    bool waitRemoval(unsigned int maxwait) override;

    /**
     * \brief Create the chip object from card type.
     * \param type The card type.
     * \return The chip.
     */
    std::shared_ptr<Chip> createChip(std::string type) override;

    /**
     * \brief Get the first and/or most accurate chip found.
     * \return The single chip.
     */
    std::shared_ptr<Chip> getSingleChip() override;

    /**
     * \brief Get chip available in the RFID rang.
     * \return The chip list.
     */
    std::vector<std::shared_ptr<Chip>> getChipList() override;

    /**
     * \brief Get the current chip in air.
     * \return The chip in air.
     */
    std::shared_ptr<Chip> getChipInAir();

    /**
     * \brief Get the default Deister reader/card adapter.
     * \return The default Deister reader/card adapter.
     */
    virtual std::shared_ptr<DeisterReaderCardAdapter>
    getDefaultDeisterReaderCardAdapter();

    /**
     * \brief Connect to the card.
     * \return True if the card was connected without error, false otherwise.
     *
     * If the card handle was already connected, connect() first call disconnect(). If you
     * intend to do a reconnection, call reconnect() instead.
     */
    bool connect() override;

    /**
     * \brief Disconnect from the Deister.
     * \see connect
     *
     * Calling this method on a disconnected Deister has no effect.
     */
    void disconnect() override;

    /**
     * \brief Check if the card is connected.
     * \return True if the card is connected, false otherwise.
     */
    bool isConnected() override;

    /**
     * \brief Connect to the reader. Implicit connection on first command sent.
     * \return True if the connection successed.
     */
    bool connectToReader() override;

    /**
     * \brief Disconnect from reader.
     */
    void disconnectFromReader() override;

    /**
     * \brief Get the reader ping command.
     * \return The ping command.
     */
    ByteVector getPingCommand() const override;

    /**
     * \brief Get a string hexadecimal representation of the reader serial number
     * \return The reader serial number or an empty string on error.
     */
    std::string getReaderSerialNumber() override;

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

    /**
     * \brief Get the Deister reader unit configuration.
     * \return The Deister reader unit configuration.
     */
    std::shared_ptr<DeisterReaderUnitConfiguration> getDeisterConfiguration()
    {
        return std::dynamic_pointer_cast<DeisterReaderUnitConfiguration>(
            getConfiguration());
    }

    /**
     * \brief Get the Deister reader provider.
     * \return The Deister reader provider.
     */
    std::shared_ptr<DeisterReaderProvider> getDeisterReaderProvider() const;

  protected:
    static std::string getCardTypeFromDeisterType(DeisterCardType deisterCardType);
};
}

#endif
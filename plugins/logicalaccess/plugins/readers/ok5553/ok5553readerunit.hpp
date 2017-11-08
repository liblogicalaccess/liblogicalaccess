/**
 * \file ok5553readerunit.hpp
 * \author Maxime C. <maxime-dppev@islog.com>
 * \brief OK5553 Reader unit.
 */

#ifndef LOGICALACCESS_OK5553READERUNIT_HPP
#define LOGICALACCESS_OK5553READERUNIT_HPP

#include <logicalaccess/readerproviders/readerunit.hpp>
#include <logicalaccess/readerproviders/serialportxml.hpp>
#include <logicalaccess/plugins/readers/ok5553/ok5553readerunitconfiguration.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace logicalaccess
{
class Profile;
class OK5553ReaderCardAdapter;
class OK5553ReaderProvider;

/**
 * \brief ChipType code in OK5553 protocol.
 */
typedef enum { MIFARE = 0x02, MIFAREULTRALIGHT = 0x05, DESFIRE = 0x06 } ChipType;

/**
 * \brief The OK5553 reader unit class.
 */
class LIBLOGICALACCESS_API OK5553ReaderUnit : public ReaderUnit
{
  public:
    /**
     * \brief Constructor.
     */
    OK5553ReaderUnit();

    /**
     * \brief Destructor.
     */
    virtual ~OK5553ReaderUnit();

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
    std::shared_ptr<Chip> getChipInAir(unsigned int maxwait = 2000);

    /**
     * \brief Get the default OK5553 reader/card adapter.
     * \return The default OK5553 reader/card adapter.
     */
    virtual std::shared_ptr<OK5553ReaderCardAdapter> getDefaultOK5553ReaderCardAdapter();

    /**
     * \brief Connect to the card.
     * \return True if the card was connected without error, false otherwise.
     *
     * If the card handle was already connected, connect() first call disconnect(). If you
     * intend to do a reconnection, call reconnect() instead.
     */
    bool connect() override;

    /**
     * \brief Disconnect from the reader.
     * \see connect
     *
     * Calling this method on a disconnected reader has no effect.
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
     * \brief Get the OK5553 reader unit configuration.
     * \return The OK5553 reader unit configuration.
     */
    std::shared_ptr<OK5553ReaderUnitConfiguration> getOK5553Configuration()
    {
        return std::dynamic_pointer_cast<OK5553ReaderUnitConfiguration>(
            getConfiguration());
    }

    /**
     * \brief Get the OK5553 reader provider.
     * \return The OK5553 reader provider.
     */
    std::shared_ptr<OK5553ReaderProvider> getOK5553ReaderProvider() const;

    /**
     * \brief Convert a Ascii vector in a Hex vector.
     * \param source The vector to convert.
     * \return The vector converted.
     */
    static ByteVector asciiToHex(const ByteVector &source);

    /**
     * \brief Send a reqA.
     * \return The answer of the card.
     */
    ByteVector reqA();

    /**
     * \brief Send a rats.
     * \return The answer of the card.
     */
    ByteVector rats();

  protected:
    /**
     * \brief Last successed RATS result.
     */
    ByteVector d_successedRATS;

    /**
     * \brief The new identifier that will be used for the next waitInsertion after the
     * waitRemoval.
     */
    ByteVector removalIdentifier;
};
}

#endif
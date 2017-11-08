/**
 * \file a3mlgm5600readerunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 Reader unit.
 */

#ifndef LOGICALACCESS_A3MLGM5600READERUNIT_HPP
#define LOGICALACCESS_A3MLGM5600READERUNIT_HPP

#include <logicalaccess/readerproviders/readerunit.hpp>
#include <logicalaccess/readerproviders/serialportxml.hpp>
#include <logicalaccess/plugins/readers/a3mlgm5600/a3mlgm5600readerunitconfiguration.hpp>

namespace logicalaccess
{
class Profile;
class A3MLGM5600ReaderCardAdapter;
class A3MLGM5600ReaderProvider;

/**
 * \brief The A3MLGM5600 reader unit class.
 */
class LIBLOGICALACCESS_API A3MLGM5600ReaderUnit : public ReaderUnit
{
  public:
    /**
     * \brief Constructor.
     */
    A3MLGM5600ReaderUnit();

    /**
     * \brief Destructor.
     */
    virtual ~A3MLGM5600ReaderUnit();

    /**
     * \brief Get the reader unit name.
     * \return The reader unit name.
     */
    std::string getName() const override;

    /**
     * \brief Get the reader unit connected name.
     * \return The reader unit connected name.
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
     * \brief Get the default A3MLGM5600 reader/card adapter.
     * \return The default A3MLGM5600 reader/card adapter.
     */
    virtual std::shared_ptr<A3MLGM5600ReaderCardAdapter>
    getDefaultA3MLGM5600ReaderCardAdapter();

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
     * \brief Get the PAD key buffer.
     * \return The PAD key buffer.
     */
    std::string getPADKey();

    /**
     * \brief High level Mifare request
     * \return The selected card serial number.
     */
    ByteVector hlRequest();

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
     * \brief Reset the RF field.
     * \param offtime The offtime, in 100 milliseconds. Default is 1.
     */
    void resetRF(int offtime = 1);

    /**
     * \brief Get the A3MLGM5600 reader unit configuration.
     * \return The A3MLGM5600 reader unit configuration.
     */
    std::shared_ptr<A3MLGM5600ReaderUnitConfiguration> getA3MLGM5600Configuration()
    {
        return std::dynamic_pointer_cast<A3MLGM5600ReaderUnitConfiguration>(
            getConfiguration());
    }

    /**
     * \brief Get the A3MLGM5600 reader provider.
     * \return The A3MLGM5600 reader provider.
     */
    std::shared_ptr<A3MLGM5600ReaderProvider> getA3MLGM5600ReaderProvider() const;

    /**
     * \brief Get the reader unit singleton instance.
     * \return The reader unit instance.
     */
    static std::shared_ptr<A3MLGM5600ReaderUnit> getSingletonInstance();

  protected:
    /**
     * \brief The device address.
     */
    unsigned char d_deviceAddress;
};
}

#endif
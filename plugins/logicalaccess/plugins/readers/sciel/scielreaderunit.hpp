/**
 * \file scielreaderunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SCIEL Reader unit.
 */

#ifndef LOGICALACCESS_SCIELREADERUNIT_HPP
#define LOGICALACCESS_SCIELREADERUNIT_HPP

#include <logicalaccess/readerproviders/readerunit.hpp>
#include <logicalaccess/plugins/readers/sciel/scielreaderunitconfiguration.hpp>
#include <logicalaccess/cards/chip.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <list>

namespace logicalaccess
{
class Profile;
class SCIELReaderCardAdapter;
class SCIELReaderProvider;

/**
 * \brief The SCIEL reader unit class.
 */
class LLA_READERS_SCIEL_API SCIELReaderUnit : public ReaderUnit
{
  public:
    /**
     * \brief Constructor.
     */
    SCIELReaderUnit();

    /**
     * \brief Destructor.
     */
    virtual ~SCIELReaderUnit();

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
     * \brief Get the raw read chip list without any threshold modification.
     * \return The reader chip list.
     */
    virtual std::list<std::shared_ptr<Chip>> getReaderChipList();

    /**
     * \brief Get chip available in the RFID rang.
     * \return The chip list.
     */
    std::vector<std::shared_ptr<Chip>> getChipList() override;

    /**
     * \brief Refresh the chip list from stack.
     * \return The chip list from the stack, without area filtering.
     */
    std::list<std::shared_ptr<Chip>> refreshChipList();

    /**
     * \brief Get the default SCIEL reader/card adapter.
     * \return The default SCIEL reader/card adapter.
     */
    virtual std::shared_ptr<SCIELReaderCardAdapter> getDefaultSCIELReaderCardAdapter();

    /**
     * \brief Connect to the card.
     * \return True if the card was connected without error, false otherwise.
     *
     * If the card handle was already connected, connect() first call disconnect(). If you
     * intend to do a reconnection, call reconnect() instead.
     */
    bool connect() override;

    /**
     * \brief Disconnect from the SCIEL.
     * \see connect
     *
     * Calling this method on a disconnected SCIEL has no effect.
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
     * \brief Get the SCIEL reader unit configuration.
     * \return The SCIEL reader unit configuration.
     */
    std::shared_ptr<SCIELReaderUnitConfiguration> getSCIELConfiguration()
    {
        return std::dynamic_pointer_cast<SCIELReaderUnitConfiguration>(
            getConfiguration());
    }

    /**
     * \brief Get the SCIEL reader provider.
     * \return The SCIEL reader provider.
     */
    std::shared_ptr<SCIELReaderProvider> getSCIELReaderProvider() const;

    /**
     * \brief Get the reader ping command.
     * \return The ping command.
     */
    ByteVector getPingCommand() const override;

    /**
     * \brief Retrieve reader identifier.
     * \return True on success, false otherwise. False means it's not a SCIEL reader
     * connected to COM port or COM port inaccessible.
     */
    bool retrieveReaderIdentifier();

    /**
     * \brief Get the SCIEL reader identifier.
     * \return The SCIEL reader identifier.
     */
    ByteVector getSCIELIdentifier() const;

    /**
     * \brief Get the time before an identifier is removed from the contextual list.
     * \return The time in seconds.
     */
    unsigned char getTimeRemovalFromContextualStack();

    /**
     * \brief Set the time before an identifier is removed from the contextual list.
     * \param timeRemoval The time in seconds.
     */
    void setTimeRemovalFromContextualStack(unsigned char timeRemoval);

    /**
     * \brief Get the reception level.
     * \return The reception level.
     */
    unsigned char getReceptionLevel();

    /**
     * \brief Set the reception level.
     * \param level The reception level.
     */
    void setReceptionLevel(unsigned char level);

    /**
     * \brief Get the AD convertor value.
     * \return The AD convertor value.
     */
    unsigned char getADConvertorValue();

  protected:
    /**
     * \brief Get the ELA tag power status from flag.
     * \param flag The flag.
     * \return The tag power status.
     */
    static ChipPowerStatus getELAPowerStatus(unsigned char flag);

    /**
     * \brief Create chip object from buffer.
     * \param buffer The buffer.
     * \return The chip object.
     */
    std::shared_ptr<Chip> createChipFromBuffer(ByteVector buffer);

    /**
     * \brief The SCIEL reader identifier.
     */
    ByteVector d_scielIdentifier;

    /**
     * \brief The chip list from stack.
     */
    std::vector<std::shared_ptr<Chip>> d_chipList;

    /**
     * \brief The chip list in tag-in area.
     */
    std::map<std::string, int> d_tagInArea;

    /**
     * \brief The chip list in tag-out area.
     */
    std::map<std::string, int> d_tagOutArea;

    /**
     * \brief The chip list in safety area.
     */
    std::map<std::string, int> d_safetyArea;

    /**
     * \brief The chip list in ignore area.
     */
    std::map<std::string, int> d_ignoreArea;

    /**
     * \brief The global chip list counter for removal timeout.
     */
    std::map<std::string, int> d_tagGoneTimeout;

    /**
     * \brief Finder class to find a chip into the chip list
     */
    class Finder
    {
      public:
        explicit Finder(std::shared_ptr<Chip> chip)
            : m_chip(chip)
        {
        }

        bool operator()(std::shared_ptr<Chip> chip) const
        {
            return (chip && m_chip &&
                    chip->getChipIdentifier() == m_chip->getChipIdentifier());
        }

      protected:
        std::shared_ptr<Chip> m_chip;
    };
};
}

#endif
#pragma once

#include "../pcscreaderunit.hpp"
#include "CL1356PlusUtils.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/logicalaccess_api.hpp"
#include <string>

namespace logicalaccess
{

/**
 * Implementation class for the CL1356+ card reader from ID3.
 *
 * This reader supports handpicking the card to work with. Its
 * the first reader in LLA whose `getChipList()` may actually
 * returns more than one chip.
 *
 * How to use ?
 *
 * If you do not plan to put multiple cards on the reader, then the
 * API is similar to other reader, and everything should work
 * as expected.
 *
 * If you plan to put multiple card on the reader, and then pick
 * one to work with, read below:
 *
 *    WaitInsertion() will wait for any card before returning, unless
 *    a card type has been forced: in that case it will wait for a card
 *    of the matching type.
 *
 *    Connect() will connect to the first card, unless a card type has been
 *    forced, in which case it will attempt to connect to a card matching
 *    the type.
 *
 *    getChipList() returns a list of Chip object. Theses are the available
 *    chip that have been detected by the reader. HOWEVER THE CHIP OBJECTS
 *    ARE NOT USABLE AS IS. It is required to call selectChip(idx) before,
 *    otherwise the reader will operate against the wrong card.
 *
 */
class LIBLOGICALACCESS_API ID3ReaderUnit : public PCSCReaderUnit
{
  public:
    virtual ~ID3ReaderUnit()
    {
    }
    ID3ReaderUnit(const std::string &name);

    /**
     * @warning Calling getChipList() is a disruptive operation. It will
     * power ON/OFF the various available card to guess their type, etc.
     */
    virtual std::vector<std::shared_ptr<Chip>> getChipList() override;

    /**
     * High level chip selection.
     *
     * The chip object must be part of the list returned by
     * getChipList().
     */
    std::shared_ptr<Chip> selectCard(std::shared_ptr<Chip>);

    virtual PCSCReaderUnitType getPCSCType() const override;

    virtual bool process_insertion(const std::string &cardType, int maxwait,
                                   const ElapsedTimeCounter &elapsed) override;

#ifndef SWIG
    using PCSCReaderUnit::connect;
#endif
    virtual bool connect(PCSCShareMode share_mode) override;

    virtual void disconnect() override;

  protected:
    std::shared_ptr<CardProbe> createCardProbe() override;

    /**
     * Select a card by its index.
     *
     * Indexing starts at 0.
     */
    std::shared_ptr<Chip> selectCard(uint8_t idx);

    /**
     * Select the correct card (if needed) based on the forced card type, if any.
     * If forced card type is not set, this function is a noop.
     *
     * Return false on failure. True otherwise.
     */
    bool select_correct_card();

    /**
     * Power ON or OFF the currently selected card.
     */
    void power_card(bool power_on);

    /**
     * Get the ATR for the idx'th card
     */
    ByteVector getAtr(int idx);

    /**
     * Returns a list of cards information for cards
     * that are available in the field.
     *
     * The public method equivalent is getChipList() that returns
     * a list of Chip object instead.
     */
    std::vector<CL1356PlusUtils::Info> listCards();

    /**
     * Unfreeze the card tracking.
     *
     * This is required because command such as listCard while
     * pause automatic card tracking.
     */
    void unfreeze();

    /**
     * Cache the list of chips when getChipList() is called.
     *
     * This allows to offers a selectChip(Chip) call, in addition
     * to the selectChip(n)
     */
    std::vector<std::shared_ptr<Chip>> chips_;

    /**
     * A RAII object that setup an adapter
     * that wrap proprietary commands.
     */
    class APDUWrapperGuard
    {
      public:
        APDUWrapperGuard(ID3ReaderUnit *reader);
        ~APDUWrapperGuard();

      private:
        class Adapter : public ReaderCardAdapter
        {
            virtual std::vector<unsigned char>
            adaptCommand(const std::vector<unsigned char> &command) override;

            virtual std::vector<unsigned char>
            adaptAnswer(const std::vector<unsigned char> &answer) override;
        };

        ID3ReaderUnit *reader_;

        // Previous adapter. Restored on object destruction.
        std::shared_ptr<ReaderCardAdapter> adapter_;
    };
};
}

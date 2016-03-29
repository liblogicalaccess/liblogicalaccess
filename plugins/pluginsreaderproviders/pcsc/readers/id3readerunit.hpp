#pragma once

#include "../pcscreaderunit.hpp"
#include "CL1356PlusUtils.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/logicalaccess_api.hpp"
#include <string>

namespace logicalaccess
{
/**
 * Contains a few information about a card.
 *
 * This is used by the ID3ReaderUnit object.
 */
struct LIBLOGICALACCESS_API ChipInformation
{
    ByteVector identifier_;
};

class LIBLOGICALACCESS_API ID3ReaderUnit : public PCSCReaderUnit
{
  public:
    virtual ~ID3ReaderUnit()
    {
    }
    ID3ReaderUnit(const std::string &name);

    /**
     * Returns the list of cards that are available in the field.
     */
    std::vector<CL1356PlusUtils::Info> listCards();

    /**
     * Get the ATR for the idx'th card
     */
    ByteVector getAtr(int idx);

    virtual std::vector<std::shared_ptr<Chip>> getChipList() override;

    /**
     * Select a card by its index in the vector returned
     * by a previous call to listCards().
     *
     * Indexing starts at 0.
     */
    std::shared_ptr<Chip> selectCard(uint8_t idx);

    void toggleCardOperation(bool e);

    PCSCReaderUnitType getPCSCType() const override;

    /**
     * Unfreeze the card tracking.
     *
     * This is required because command such as listCard while
     * pause automatic card tracking.
     */
    void unfreeze();

    virtual bool process_insertion(const std::string &cardType, int maxwait,
                                   const ElapsedTimeCounter &elapsed) override;

    using PCSCReaderUnit::connect;
    virtual bool connect(PCSCShareMode share_mode) override;

    virtual void disconnect() override;

    void power_card(bool power_on);


  private:
    /**
     * Select the correct card (if needed) based on the forced card type, if any.
     * If forced card type is not set, this function is a noop.
     *
     * Return false on failure. True otherwise.
     */
    bool select_correct_card();

  protected:
    std::shared_ptr<CardProbe> createCardProbe() override;

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

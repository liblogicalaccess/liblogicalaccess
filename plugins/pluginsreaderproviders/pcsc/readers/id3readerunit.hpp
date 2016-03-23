#pragma once

#include "../pcscreaderunit.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/logicalaccess_api.hpp"
#include "CL1356PlusUtils.hpp"
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
     * Select a card by its index in the vector returned
     * by a previous call to listCards().
     *
     * Indexing starts at 0.
     */
    std::shared_ptr<Chip> selectCard(uint8_t idx);

    void toggleCardOperation(bool e);

    PCSCReaderUnitType getPCSCType() const override;

        void unfreeze();

  private:
    /**
     * Extract the information about 1 card from the data
     * returned by the list card proprietary APDU.
     * See CL1356ACommandsReference.pdf
     */
    ChipInformation extract_card_info(ByteVector::iterator &itr,
                                      const ByteVector::iterator &end);

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

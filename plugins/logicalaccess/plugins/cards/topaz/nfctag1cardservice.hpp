/**
* \file nfctag1cardservice.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief NFC Tag Type 1 card service.
*/

#ifndef LOGICALACCESS_NFCTAG1CARDSERVICE_HPP
#define LOGICALACCESS_NFCTAG1CARDSERVICE_HPP

#include <logicalaccess/services/nfctag/nfctagcardservice.hpp>
#include <logicalaccess/services/nfctag/ndefmessage.hpp>
#include <logicalaccess/plugins/cards/topaz/topazchip.hpp>

namespace logicalaccess
{
#define NFC_TAG1_CARDSERVICE "NFCTag1"

/**
* \brief The NFC Tag 1 storage card service base class.
*/
class LIBLOGICALACCESS_API NFCTag1CardService : public NFCTagCardService
{
  public:
    /**
    * \brief Constructor.
    * \param chip The chip.
    */
    explicit NFCTag1CardService(std::shared_ptr<Chip> chip)
        : NFCTagCardService(chip)
    {
    }

    virtual ~NFCTag1CardService()
    {
    }

    std::string getCSType() override
    {
        return NFC_TAG1_CARDSERVICE;
    }

    std::shared_ptr<NdefMessage> readNDEF() override;

    void writeNDEF(std::shared_ptr<NdefMessage> records) override;

    void eraseNDEF() override;

  protected:
    std::shared_ptr<TopazChip> getTopazChip() const
    {
        return std::dynamic_pointer_cast<TopazChip>(getChip());
    }
};
}

#endif /* LOGICALACCESS_NFCTAG1CARDSERVICE_HPP */
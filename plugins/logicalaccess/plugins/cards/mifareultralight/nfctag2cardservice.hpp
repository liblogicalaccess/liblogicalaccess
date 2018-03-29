/**
* \file nfctag2cardservice.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief NFC Tag 2 card service.
*/

#ifndef LOGICALACCESS_NFCTAG2CARDSERVICE_HPP
#define LOGICALACCESS_NFCTAG2CARDSERVICE_HPP

#include <logicalaccess/services/nfctag/nfctagcardservice.hpp>
#include <logicalaccess/services/nfctag/ndefmessage.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightchip.hpp>

namespace logicalaccess
{
#define CARDSERVICE_NFC_TAG2 "NFCTag2"

/**
* \brief The NFC Tag 2 storage card service base class.
*/
class LIBLOGICALACCESS_API NFCTag2CardService : public NFCTagCardService
{
  public:
    /**
    * \brief Constructor.
    * \param chip The chip.
    */
    explicit NFCTag2CardService(std::shared_ptr<Chip> chip)
        : NFCTagCardService(chip)
    {
    }

    virtual ~NFCTag2CardService()
    {
    }

    std::string getCSType() override
    {
        return CARDSERVICE_NFC_TAG2;
    }

    std::shared_ptr<NdefMessage> readNDEF() override;

    void writeNDEF(std::shared_ptr<NdefMessage> records) override;

    void eraseNDEF() override;

    void writeCapabilityContainer() const;

  protected:
    std::shared_ptr<MifareUltralightChip> getMifareUltralightChip() const
    {
        return std::dynamic_pointer_cast<MifareUltralightChip>(getChip());
    }
};
}

#endif /* LOGICALACCESS_NFCTAG2CARDSERVICE_HPP */
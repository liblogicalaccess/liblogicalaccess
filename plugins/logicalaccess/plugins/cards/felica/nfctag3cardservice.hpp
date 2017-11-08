/**
* \file nfctag3cardservice.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief NFC Tag Type 3 card service.
*/

#ifndef LOGICALACCESS_NFCTAG3CARDSERVICE_HPP
#define LOGICALACCESS_NFCTAG3CARDSERVICE_HPP

#include <logicalaccess/services/nfctag/nfctagcardservice.hpp>
#include <logicalaccess/services/nfctag/ndefmessage.hpp>
#include <logicalaccess/plugins/cards/felica/felicachip.hpp>

namespace logicalaccess
{
/**
* \brief The NFC Tag 3 storage card service base class.
*/
class LIBLOGICALACCESS_API NFCTag3CardService : public NFCTagCardService
{
  public:
    /**
    * \brief Constructor.
    * \param chip The chip.
    */
    explicit NFCTag3CardService(std::shared_ptr<Chip> chip)
        : NFCTagCardService(chip)
    {
    }

    virtual ~NFCTag3CardService()
    {
    }

    std::shared_ptr<NdefMessage> readNDEF() override;

    void writeNDEF(std::shared_ptr<NdefMessage> records) override;

    void eraseNDEF() override;

  protected:
    std::shared_ptr<FeliCaChip> getFeliCaChip() const
    {
        return std::dynamic_pointer_cast<FeliCaChip>(getChip());
    }
};
}

#endif /* LOGICALACCESS_NFCTAG3CARDSERVICE_HPP */
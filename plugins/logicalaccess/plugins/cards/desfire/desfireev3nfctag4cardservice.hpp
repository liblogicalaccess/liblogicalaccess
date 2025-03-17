/**
 * \file desfireev3nfctag4cardservice.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire EV3 NFC Tag 4 card service.
 */

#ifndef LOGICALACCESS_DESFIREEV3NFCTAG4CARDSERVICE_HPP
#define LOGICALACCESS_DESFIREEV3NFCTAG4CARDSERVICE_HPP

#include <logicalaccess/plugins/cards/desfire/desfireev1nfctag4cardservice.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev3chip.hpp>

namespace logicalaccess
{
class DESFireChip;

#define NFC_CARDSERVICE_DESFIREEV3 "DESFireEV3NFCTag4"

/**
 * \brief The DESFire EV3 NFC Tag card service base class.
 */
class LLA_CARDS_DESFIRE_API DESFireEV3NFCTag4CardService
    : public DESFireEV1NFCTag4CardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The chip.
     */
    explicit DESFireEV3NFCTag4CardService(std::shared_ptr<Chip> chip)
        : DESFireEV1NFCTag4CardService(chip)
    {
    }

    ~DESFireEV3NFCTag4CardService()
    {
    }

    std::string getCSType() override
    {
        return NFC_CARDSERVICE_DESFIREEV3;
    }

    void writeSDM(const std::string& baseUri);

    void writeSDMFile(const std::string& baseUri, 
                  const std::string& paramVcuid = "",
                  const std::string& paramPicc = "picc",
                  const std::string& paramReadCtr = "",
                  const std::string& paramMAC = "mac",
                  unsigned short isoFIDNDEFFile = 0xe104);

  protected:
    std::shared_ptr<DESFireEV3Chip> getDESFireEV3Chip() const
    {
        return std::dynamic_pointer_cast<DESFireEV3Chip>(getChip());
    }

    void createNDEFFile(const DESFireAccessRights& dar, unsigned short isoFIDNDEFFile) override;
};
}

#endif /* LOGICALACCESS_DESFIREEV3NFCTAG4CARDSERVICE_HPP */
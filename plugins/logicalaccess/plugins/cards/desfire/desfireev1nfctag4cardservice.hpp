/**
 * \file desfirenfctag4cardservice.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief DESFire NFC Tag 4 card service.
 */

#ifndef LOGICALACCESS_DESFIREEV1NFCTAG4CARDSERVICE_HPP
#define LOGICALACCESS_DESFIREEV1NFCTAG4CARDSERVICE_HPP

#include <logicalaccess/plugins/cards/iso7816/iso7816nfctag4cardservice.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>

namespace logicalaccess
{
class DESFireChip;

#define NFC_CARDSERVICE_DESFIREEV1 "DESFireEV1NFCTag4"

/**
 * \brief The DESFire storage card service base class.
 */
class LLA_CARDS_DESFIRE_API DESFireEV1NFCTag4CardService
    : public ISO7816NFCTag4CardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The chip.
     */
    explicit DESFireEV1NFCTag4CardService(std::shared_ptr<Chip> chip);

    ~DESFireEV1NFCTag4CardService()
    {
    }

    std::string getCSType() override
    {
        return NFC_CARDSERVICE_DESFIREEV1;
    }

    void createNFCApplication(unsigned int aid,
                              unsigned short isoFIDApplication         = 0xe105,
                              unsigned short isoFIDCapabilityContainer = 0xe103,
                              unsigned short isoFIDNDEFFile            = 0xe104,
                              unsigned short NDEFFileSize              = 0xff);

    void deleteNFCApplication(unsigned int aid) const;

    std::shared_ptr<NdefMessage>
    readNDEFFile(const ByteVector &appDFName = {0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01},
                 unsigned short isoFIDNDEFFile    = 0xe104) override;

    void writeNDEF(std::shared_ptr<NdefMessage> records) override;

    void eraseNDEF() override;

    void setPICCKey(std::shared_ptr<DESFireKey> key)
    {
        d_picc_key = key;
    }

    void setAppEmptyKey(std::shared_ptr<DESFireKey> key)
    {
        d_app_empty_key = key;
    }

    void setAppNewKey(std::shared_ptr<DESFireKey> key)
    {
        d_app_new_key = key;
    }

  protected:
    std::shared_ptr<DESFireEV1Chip> getDESFireChip() const
    {
        return std::dynamic_pointer_cast<DESFireEV1Chip>(getChip());
    }

    std::shared_ptr<ISO7816Commands> getISO7816Commands() const override;

    virtual void createNDEFFile(const DESFireAccessRights& dar, unsigned short isoFIDNDEFFile);

    std::shared_ptr<DESFireKey> d_picc_key;
    std::shared_ptr<DESFireKey> d_app_empty_key;
    std::shared_ptr<DESFireKey> d_app_new_key;
};
}

#endif /* LOGICALACCESS_DESFIRENFCTAG4CARDSERVICE_HPP */
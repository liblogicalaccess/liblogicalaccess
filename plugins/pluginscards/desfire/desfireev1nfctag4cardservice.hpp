/**
 * \file desfirenfctag4cardservice.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief DESFire NFC Tag 4 card service.
 */

#ifndef LOGICALACCESS_DESFIREEV1NFCTAG4CARDSERVICE_HPP
#define LOGICALACCESS_DESFIREEV1NFCTAG4CARDSERVICE_HPP

#include <iso7816/iso7816nfctag4cardservice.hpp>
#include "desfireev1chip.hpp"

namespace logicalaccess
{
class DESFireChip;

/**
 * \brief The DESFire storage card service base class.
 */
class LIBLOGICALACCESS_API DESFireEV1NFCTag4CardService : public ISO7816NFCTag4CardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The chip.
     */
    explicit DESFireEV1NFCTag4CardService(std::shared_ptr<Chip> chip)
        : ISO7816NFCTag4CardService(chip)
    {
    }

    ~DESFireEV1NFCTag4CardService()
    {
    }

    void createNFCApplication(unsigned int aid, std::shared_ptr<DESFireKey> masterPICCKey,
                              unsigned short isoFIDApplication         = 0xe105,
                              unsigned short isoFIDCapabilityContainer = 0xe103,
                              unsigned short isoFIDNDEFFile            = 0xe104,
                              unsigned short NDEFFileSize              = 0xff);

    void deleteNFCApplication(unsigned int aid,
                              std::shared_ptr<DESFireKey> masterPICCKey) const;

    std::shared_ptr<NdefMessage>
    readNDEFFile(unsigned short isoFIDApplication = 0xe105,
                 unsigned short isoFIDNDEFFile    = 0xe104) override;

    void writeNDEF(std::shared_ptr<NdefMessage> records) override;

    void eraseNDEF() override;

  protected:
    std::shared_ptr<DESFireEV1Chip> getDESFireChip() const
    {
        return std::dynamic_pointer_cast<DESFireEV1Chip>(getChip());
    }
};
}

#endif /* LOGICALACCESS_DESFIRENFCTAG4CARDSERVICE_HPP */
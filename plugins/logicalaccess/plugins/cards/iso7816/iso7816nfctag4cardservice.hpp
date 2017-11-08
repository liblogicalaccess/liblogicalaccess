/**
 * \file iso7816nfctag4cardservice.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief NFC Tag 4 card service.
 */

#ifndef LOGICALACCESS_ISO7816NFCTAG4CARDSERVICE_HPP
#define LOGICALACCESS_ISO7816NFCTAG4CARDSERVICE_HPP

#include <logicalaccess/services/nfctag/nfctagcardservice.hpp>
#include <logicalaccess/services/nfctag/ndefmessage.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816chip.hpp>

namespace logicalaccess
{
/**
 * \brief The ISO7816 NFC Tag 4 storage card service base class.
 */
class LIBLOGICALACCESS_API ISO7816NFCTag4CardService : public NFCTagCardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The chip.
     */
    explicit ISO7816NFCTag4CardService(std::shared_ptr<Chip> chip)
        : NFCTagCardService(chip)
    {
    }

    virtual ~ISO7816NFCTag4CardService()
    {
    }

    virtual void writeCapabilityContainer(unsigned short isoFID         = 0xe103,
                                          unsigned short isoFIDNDEFFile = 0xe104,
                                          unsigned short NDEFFileSize   = 0xff);

    virtual void writeNDEFFile(std::shared_ptr<NdefMessage> records,
                               unsigned short isoFIDNDEFFile = 0xe104);

    virtual void writeNDEFFile(ByteVector recordsData,
                               unsigned short isoFIDNDEFFile = 0xe104);

    virtual std::shared_ptr<NdefMessage>
    readNDEFFile(unsigned short isoFIDApplication = 0xe105,
                 unsigned short isoFIDNDEFFile    = 0xe104);

    std::shared_ptr<NdefMessage> readNDEF() override;

  protected:
    std::shared_ptr<ISO7816Chip> getISO7816Chip() const
    {
        return std::dynamic_pointer_cast<ISO7816Chip>(getChip());
    }
};
}

#endif /* LOGICALACCESS_ISO7816NFCTAG4CARDSERVICE_HPP */
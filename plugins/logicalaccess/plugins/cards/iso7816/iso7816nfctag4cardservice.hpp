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
#define NFC_CARDSERVICE_ISO7816_TAG4 "ISO7816NFCTag4"

/**
 * \brief The ISO7816 NFC Tag 4 storage card service base class.
 */
class LLA_CARDS_ISO7816_API ISO7816NFCTag4CardService : public NFCTagCardService
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

    std::string getCSType() override
    {
        return NFC_CARDSERVICE_ISO7816_TAG4;
    }

    virtual void writeCapabilityContainer(unsigned short isoFID         = 0xe103,
                                          unsigned short isoFIDNDEFFile = 0xe104,
                                          unsigned short NDEFFileSize   = 0xff);

    virtual void writeNDEFFile(std::shared_ptr<NdefMessage> records,
                               unsigned short isoFIDNDEFFile = 0xe104);

    virtual void writeNDEFFile(ByteVector recordsData,
                               unsigned short isoFIDNDEFFile = 0xe104);

    virtual std::shared_ptr<NdefMessage>
    readNDEFFile(const ByteVector &appDFName = {0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01},
                 unsigned short isoFIDNDEFFile    = 0xe104);

    std::shared_ptr<NdefMessage> readNDEF() override;

  protected:
    virtual std::shared_ptr<ISO7816Commands> getISO7816Commands() const;
};
}

#endif /* LOGICALACCESS_ISO7816NFCTAG4CARDSERVICE_HPP */
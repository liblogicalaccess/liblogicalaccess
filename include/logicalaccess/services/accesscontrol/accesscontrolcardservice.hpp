/**
 * \file accesscontrolcardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Access Control Card service.
 */

#ifndef LOGICALACCESS_ACCESSCONTROLCARDSERVICE_HPP
#define LOGICALACCESS_ACCESSCONTROLCARDSERVICE_HPP

#include <logicalaccess/services/cardservice.hpp>
#include <logicalaccess/services/accesscontrol/formats/format.hpp>
#include <logicalaccess/cards/location.hpp>
#include <logicalaccess/cards/accessinfo.hpp>
#include <logicalaccess/iks/RemoteCrypto.hpp>

namespace logicalaccess
{
/**
 * \brief A format list.
 */
typedef std::vector<std::shared_ptr<Format>> FormatList;


/**
 * Extraction flags used to configure PACS bits retrieval.
 */
enum class PACS_EXTRACTION_MODE
{
    SECURE_CHANNEL = 0,
    NO_SECURE_CHANNEL,
    KEYBOARD_WEDGE,
};

#define ACCESSCONTROLCARDSERVICE "AccessControl"

/**
 * \brief The base access control card service class for all access control services.
 */
class LLA_CORE_API AccessControlCardService : public CardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The associated chip.
     */
    explicit AccessControlCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    virtual ~AccessControlCardService();

    std::string getCSType() override
    {
        return ACCESSCONTROLCARDSERVICE;
    }

    /**
     * \brief Read format from the card.
     * \param format The format to read.
     * \param location The format location.
     * \param aiToUse The key's informations to use.
     * \return The format read on success, null otherwise.
     */
    virtual std::shared_ptr<Format> readFormat(std::shared_ptr<Format> format,
                                               std::shared_ptr<Location> location,
                                               std::shared_ptr<AccessInfo> aiToUse);

    /**
     * IKS Related call.
     *
     * This is when IKS sign decrypted packet as a proof that they
     * were read using a DESFire Session Key stored on IKS.
     */
    SignatureResult IKS_getPayloadSignature() const
    {
        return last_signature_;
    }

    /**
     * \brief Write format to the card.
     * \param format The format to write.
     * \param location The format location.
     * \param aiToUse The key's informations to use.
     * \param aiToWrite The key's informations to write.
     * \return True on success, false otherwise.
     */
    virtual bool writeFormat(std::shared_ptr<Format> format,
                             std::shared_ptr<Location> location,
                             std::shared_ptr<AccessInfo> aiToUse,
                             std::shared_ptr<AccessInfo> aiToWrite);

    /**
     * \brief Get the supported format list.
     * \return The format list.
     */
    virtual FormatList getSupportedFormatList();

    /**
     * \brief Get the HID Wiegand format list.
     * \return The format list.
     */
    FormatList getHIDWiegandFormatList() const;

    /**
     * Retrieve the Physical Access Control bits.
     * This generally relies on the PCSC reader to provide support.
     */
    virtual ByteVector getPACSBits(PACS_EXTRACTION_MODE)
    {
        throw std::runtime_error("Not implemented");
    }

    /**
     * Write PACSbit onto a card.
     *
     * This method is geared toward SEos, and will write to standard
     * PACS ADF.
     */
    virtual void writePACSBit(const ByteVector &pacsbits)
    {
        throw std::runtime_error("Not implemented");
    }

  protected:
    SignatureResult last_signature_;
};
}

#endif
/**
 * \file twicaccesscontrolcardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic Access Control Card service.
 */

#ifndef LOGICALACCESS_TWICACCESSCONTROLCARDSERVICE_HPP
#define LOGICALACCESS_TWICACCESSCONTROLCARDSERVICE_HPP

#include <logicalaccess/plugins/cards/twic/twicchip.hpp>
#include <logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp>

namespace logicalaccess
{
class TwicCardProvider;

#define TWIC_ACCESSCONTROL_CARDSERVICE "TwicAccessControl"

/**
 * \brief The Twic access control card service class.
 */
class LLA_CARDS_TWIC_API TwicAccessControlCardService : public AccessControlCardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The associated chip.
     */
    explicit TwicAccessControlCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    ~TwicAccessControlCardService();

    std::string getCSType() override
    {
        return TWIC_ACCESSCONTROL_CARDSERVICE;
    }

    /**
     * \brief Read format from the card.
     * \param format The format to read.
     * \param location The format location.
     * \param aiToUse The key's informations to use.
     * \return The format read on success, null otherwise.
     */
    std::shared_ptr<Format> readFormat(std::shared_ptr<Format> format,
                                       std::shared_ptr<Location> location,
                                       std::shared_ptr<AccessInfo> aiToUse) override;

    /**
     * \brief Write format to the card.
     * \param format The format to write.
     * \param location The format location.
     * \param aiToUse The key's informations to use.
     * \param aiToWrite The key's informations to write.
     * \return True on success, false otherwise.
     */
    bool writeFormat(std::shared_ptr<Format> format, std::shared_ptr<Location> location,
                     std::shared_ptr<AccessInfo> aiToUse,
                     std::shared_ptr<AccessInfo> aiToWrite) override;

    /**
    * \brief Get the supported format list.
    * \return The format list.
    */
    FormatList getSupportedFormatList() override;

  protected:
    std::shared_ptr<TwicChip> getTwicChip() const
    {
        return std::dynamic_pointer_cast<TwicChip>(getChip());
    }
};
}

#endif
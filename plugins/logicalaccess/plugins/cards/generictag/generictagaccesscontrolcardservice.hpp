/**
 * \file generictagaccesscontrolcardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic Tag Access Control Card service.
 */

#ifndef LOGICALACCESS_GENERICTAGACCESSCONTROLCARDSERVICE_HPP
#define LOGICALACCESS_GENERICTAGACCESSCONTROLCARDSERVICE_HPP

#include <logicalaccess/plugins/cards/generictag/generictagchip.hpp>
#include <logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp>

namespace logicalaccess
{
class GenericTagCardProvider;

#define GENERICACCESSCONTROLCARDSERVICE "GenericTagAccessControl"

/**
 * \brief The GenericTag access control card service class.
 */
class LLA_CARDS_GENERICTAG_API GenericTagAccessControlCardService
    : public AccessControlCardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The chip.
     */
    explicit GenericTagAccessControlCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    ~GenericTagAccessControlCardService();

    std::string getCSType() override
    {
        return GENERICACCESSCONTROLCARDSERVICE;
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

    std::shared_ptr<GenericTagChip> getGenericTagChip() const
    {
        return std::dynamic_pointer_cast<GenericTagChip>(getChip());
    }
};
}

#endif
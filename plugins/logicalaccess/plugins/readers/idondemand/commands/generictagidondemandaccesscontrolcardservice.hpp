/**
 * \file generictagidondemandaccesscontrolcardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic Tag IdOnDemand Access Control Card service.
 */

#ifndef LOGICALACCESS_GENERICTAGIDONDEMANDACCESSCONTROLCARDSERVICE_HPP
#define LOGICALACCESS_GENERICTAGIDONDEMANDACCESSCONTROLCARDSERVICE_HPP

#include <logicalaccess/plugins/cards/generictag/generictagaccesscontrolcardservice.hpp>

namespace logicalaccess
{
class GenericTagCardProvider;

#define GENERICTAGIDONDEMANDEACCESSCONTROLCARDSERVICE "GenericTagIdOnDemandAccessControl"

/**
 * \brief The GenericTag access control card service class for IdOnDemand reader.
 */
class LIBLOGICALACCESS_API GenericTagIdOnDemandAccessControlCardService
    : public GenericTagAccessControlCardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The chip.
     */
    explicit GenericTagIdOnDemandAccessControlCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    ~GenericTagIdOnDemandAccessControlCardService();

    std::string getCSType() override { return GENERICTAGIDONDEMANDEACCESSCONTROLCARDSERVICE; }

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
};
}

#endif
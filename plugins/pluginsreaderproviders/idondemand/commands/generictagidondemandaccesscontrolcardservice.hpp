/**
 * \file generictagidondemandaccesscontrolcardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic Tag IdOnDemand Access Control Card service.
 */

#ifndef LOGICALACCESS_GENERICTAGIDONDEMANDACCESSCONTROLCARDSERVICE_HPP
#define LOGICALACCESS_GENERICTAGIDONDEMANDACCESSCONTROLCARDSERVICE_HPP

#include "generictag/generictagaccesscontrolcardservice.hpp"

namespace logicalaccess
{
    class GenericTagCardProvider;

    /**
     * \brief The GenericTag access control card service class for IdOnDemand reader.
     */
    class LIBLOGICALACCESS_API GenericTagIdOnDemandAccessControlCardService : public GenericTagAccessControlCardService
    {
    public:

        /**
         * \brief Constructor.
         * \param chip The chip.
         */
        GenericTagIdOnDemandAccessControlCardService(std::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
        ~GenericTagIdOnDemandAccessControlCardService();

        /**
         * \brief Read format from the card.
         * \param format The format to read.
         * \param location The format location.
         * \param aiToUse The key's informations to use.
         * \return The format read on success, null otherwise.
         */
        virtual std::shared_ptr<Format> readFormat(std::shared_ptr<Format> format, std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse);

        /**
         * \brief Write format to the card.
         * \param format The format to write.
         * \param location The format location.
         * \param aiToUse The key's informations to use.
         * \param aiToWrite The key's informations to write.
         * \return True on success, false otherwise.
         */
        virtual bool writeFormat(std::shared_ptr<Format> format, std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite);

    protected:
    };
}

#endif
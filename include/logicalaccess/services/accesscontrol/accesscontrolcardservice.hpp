/**
 * \file accesscontrolcardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Access Control Card service.
 */

#ifndef LOGICALACCESS_ACCESSCONTROLCARDSERVICE_HPP
#define LOGICALACCESS_ACCESSCONTROLCARDSERVICE_HPP

#include "logicalaccess/services/cardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/format.hpp"
#include "logicalaccess/cards/location.hpp"
#include "logicalaccess/cards/accessinfo.hpp"

namespace logicalaccess
{
    /**
     * \brief The base access control card service class for all access control services.
     */
    class LIBLOGICALACCESS_API AccessControlCardService : public CardService
    {
    public:

        /**
         * \brief Constructor.
         * \param cardProvider The associated chip.
         */
        AccessControlCardService(std::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
        ~AccessControlCardService();

        /**
         * \brief Get the card service type.
         * \return The card service type.
         */
        virtual CardServiceType getServiceType() const;

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

        /**
         * \brief Get the PACS bits of the card.
         * \param location Location where is located the PACS bits.
         * \param aiToUse Access information to use for authentication on the PACS bits.
         * \param maxPACSLength The maximum PACS length in bytes.
         * \return The PACS bits.
         */
        virtual std::vector<unsigned char> getPACSBits(
                std::shared_ptr<Location> location = std::shared_ptr<Location>(),
                std::shared_ptr<AccessInfo> aiToUse = std::shared_ptr<AccessInfo>(),
                size_t maxPACSLength = 0) { return std::vector<unsigned char>() ; } ;

    protected:
    };
}

#endif
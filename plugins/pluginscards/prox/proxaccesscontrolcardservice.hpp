/**
 * \file proxaccesscontrolcardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox Access Control Card service.
 */

#ifndef LOGICALACCESS_PROXACCESSCONTROLCARDSERVICE_HPP
#define LOGICALACCESS_PROXACCESSCONTROLCARDSERVICE_HPP

#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"

namespace logicalaccess
{
    /**
     * \brief The Prox access control card service class.
     */
    class LIBLOGICALACCESS_API ProxAccessControlCardService : public AccessControlCardService
    {
    public:

        /**
         * \brief Constructor.
         * \param cardProvider The associated card provider.
         */
        ProxAccessControlCardService(std::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
        ~ProxAccessControlCardService();

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
         * \return The PACS bits.
         */
        std::vector<unsigned char> getPACSBits();

		/**
		* \brief Get the supported format list.
		* \return The format list.
		*/
		virtual FormatList getSupportedFormatList();

    protected:
    };
}

#endif
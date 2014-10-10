/**
 * \file generictagaccesscontrolcardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic Tag Access Control Card service.
 */

#ifndef LOGICALACCESS_GENERICTAGACCESSCONTROLCARDSERVICE_HPP
#define LOGICALACCESS_GENERICTAGACCESSCONTROLCARDSERVICE_HPP

#include "generictagchip.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"

namespace logicalaccess
{
    class GenericTagCardProvider;

    /**
     * \brief The GenericTag access control card service class.
     */
    class LIBLOGICALACCESS_API GenericTagAccessControlCardService : public AccessControlCardService
    {
    public:

        /**
         * \brief Constructor.
         * \param chip The chip.
         */
        GenericTagAccessControlCardService(boost::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
        ~GenericTagAccessControlCardService();

        /**
         * \brief Read format from the card.
         * \param format The format to read.
         * \param location The format location.
         * \param aiToUse The key's informations to use.
         * \return The format read on success, null otherwise.
         */
        virtual boost::shared_ptr<Format> readFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse);

        /**
         * \brief Write format to the card.
         * \param format The format to write.
         * \param location The format location.
         * \param aiToUse The key's informations to use.
         * \param aiToWrite The key's informations to write.
         * \return True on success, false otherwise.
         */
        virtual bool writeFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite);

        boost::shared_ptr<GenericTagChip> getGenericTagChip() { return boost::dynamic_pointer_cast<GenericTagChip>(getChip()); };

    protected:
    };
}

#endif
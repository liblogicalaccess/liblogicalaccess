/**
 * \file topazstoragecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz storage card service.
 */

#ifndef LOGICALACCESS_TOPAZSTORAGECARDSERVICE_HPP
#define LOGICALACCESS_TOPAZSTORAGECARDSERVICE_HPP

#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "topazchip.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The Topaz base profile class.
     */
    class LIBLOGICALACCESS_API TopazStorageCardService : public StorageCardService
    {
    public:

        /**
         * \brief Constructor.
         * \param chip The chip.
         */
        TopazStorageCardService(std::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
        ~TopazStorageCardService();

        /**
         * \brief Erase the card.
         */
        virtual void erase();

        /**
         * \brief Erase a specific location on the card.
         * \param location The data location.
         * \param aiToUse The key's informations to use to delete.
         */
        virtual void erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse);

        /**
         * \brief Write data on a specific Topaz location, using given Topaz keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param aiToWrite The key's informations to change.
         * \param data Data to write.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual void writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite, const std::vector<unsigned char>& data, CardBehavior behaviorFlags);

        /**
         * \brief Read data on a specific Topaz location, using given Topaz keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param length to read.
         * \param behaviorFlags Flags which determines the behavior.
		 * \return Data readed
         */
        virtual std::vector<unsigned char> readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, size_t length, CardBehavior behaviorFlags);

		/**
		* \brief Read data header on a specific location, using given keys.
		* \param location The data location.
		* \param aiToUse The key's informations to use.
		* \return The read data.
		*/
		virtual std::vector<unsigned char> readDataHeader(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse);

    protected:

        /*
         * \brief Get the associated Topaz chip.
         * \return The Topaz chip.
         */
        std::shared_ptr<TopazChip> getTopazChip() { return std::dynamic_pointer_cast<TopazChip>(getChip()); };
    };
}

#endif /* LOGICALACCESS_TOPAZSTORAGECARDSERVICE_HPP */
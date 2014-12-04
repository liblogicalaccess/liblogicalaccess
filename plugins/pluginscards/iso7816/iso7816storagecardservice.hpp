/**
 * \file iso7816storagecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 storage card service.
 */

#ifndef LOGICALACCESS_ISO7816STORAGECARDSERVICE_HPP
#define LOGICALACCESS_ISO7816STORAGECARDSERVICE_HPP

#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "iso7816chip.hpp"

namespace logicalaccess
{
    /**
     * \brief The ISO7816 card provider base class.
     */
    class LIBLOGICALACCESS_API ISO7816StorageCardService : public StorageCardService
    {
    public:

        /**
         * \brief Constructor.
         */
        ISO7816StorageCardService(std::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
        virtual ~ISO7816StorageCardService();

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
         * \brief Write data on a specific Tag-It location, using given Tag-It keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param aiToWrite The key's informations to change.
         * \param data Data to write.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual void writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite, const std::vector<unsigned char>& data, CardBehavior behaviorFlags);

        /**
         * \brief Read data on a specific Tag-It location, using given Tag-It keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param length to read.
         * \param behaviorFlags Flags which determines the behavior.
		 * \return The bytes readed.
         */
        virtual std::vector<unsigned char> readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, size_t Length, CardBehavior behaviorFlags);

        /**
         * \brief Read data header on a specific location, using given keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use.
         * \param data Will contain data after reading.
         * \param dataLength Data's length to read.
         * \return Data header length.
         */
        virtual unsigned int readDataHeader(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength);

    protected:

        /**
         * \brief Get the associated ISO7816 chip.
         * \return The ISO7816 chip.
         */
        std::shared_ptr<ISO7816Chip> getISO7816Chip() { return std::dynamic_pointer_cast<ISO7816Chip>(getChip()); };
    };
}

#endif
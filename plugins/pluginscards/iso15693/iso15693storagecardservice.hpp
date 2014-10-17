/**
 * \file iso15693storagecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO15693 storage card service.
 */

#ifndef LOGICALACCESS_ISO15693STORAGECARDSERVICE_HPP
#define LOGICALACCESS_ISO15693STORAGECARDSERVICE_HPP

#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "iso15693chip.hpp"

namespace logicalaccess
{
    class ISO15693Chip;

    /**
     * \brief The ISO15693 storage card service base class.
     */
    class LIBLOGICALACCESS_API ISO15693StorageCardService : public StorageCardService
    {
    public:

        /**
         * \brief Constructor.
         */
        ISO15693StorageCardService(std::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
        virtual ~ISO15693StorageCardService();

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
         * \param dataLength Data's length to write.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual void writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags);

        /**
         * \brief Read data on a specific Tag-It location, using given Tag-It keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param data Will contain data after reading.
         * \param dataLength Data's length to read.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual void readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags);

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

        std::shared_ptr<ISO15693Chip> getISO15693Chip() { return std::dynamic_pointer_cast<ISO15693Chip>(getChip()); };
    };
}

#endif
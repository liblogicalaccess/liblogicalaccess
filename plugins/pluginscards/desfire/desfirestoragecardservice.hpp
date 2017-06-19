/**
 * \file desfirestoragecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire storage card service.
 */

#ifndef LOGICALACCESS_DESFIRESTORAGECARDSERVICE_HPP
#define LOGICALACCESS_DESFIRESTORAGECARDSERVICE_HPP

#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "desfirechip.hpp"

namespace logicalaccess
{
    class DESFireChip;

    /**
     * \brief The DESFire storage card service bas class.
     */
    class LIBLOGICALACCESS_API DESFireStorageCardService : public StorageCardService
    {
    public:

        /**
         * \brief Constructor.
         * \param chip The chip.
         */
        DESFireStorageCardService(std::shared_ptr<Chip> chip);

        /**
         * \brief Erase the card.
         */
        virtual void erase();

        /**
         * \brief Erase a specific location on the card.
         * \param location The data location.
         * \param aiToUse The key's informations to use to delete.
         * \return true if the card was erased, false otherwise. If false, the card may be partially erased.
         */
        virtual void erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse);

        /**
         * \brief Write data on a specific DESFire location, using given DESFire keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param aiToWrite The key's informations to change.
         * \param data Data to write.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual void writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite, const std::vector<unsigned char>& data, CardBehavior behaviorFlags);

        /**
         * \brief Read data on a specific DESFire location, using given DESFire keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param behaviorFlags Flags which determines the behavior.
		 * \return Data readed
         */
        virtual std::vector<unsigned char> readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, size_t dataLength, CardBehavior behaviorFlags);

        /**
         * \brief Read data header on a specific location, using given keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use.
         * \param data Will contain data after reading.
         * \param dataLength Data's length to read.
         * \return Data header length.
         */
        virtual std::vector<unsigned char> readDataHeader(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse);

    protected:

        std::shared_ptr<DESFireChip> getDESFireChip() { return std::dynamic_pointer_cast<DESFireChip>(getChip()); };
    };
}

#endif /* LOGICALACCESS_DESFIRESTORAGECARDSERVICE_HPP */
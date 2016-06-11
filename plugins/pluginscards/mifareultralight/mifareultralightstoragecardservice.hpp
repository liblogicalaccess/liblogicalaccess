/**
 * \file mifareultralightstoragecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight storage card service.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTSTORAGECARDSERVICE_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTSTORAGECARDSERVICE_HPP

#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "mifareultralightchip.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The Mifare Ultralight base profile class.
     */
    class LIBLOGICALACCESS_API MifareUltralightStorageCardService : public StorageCardService
    {
    public:

        /**
         * \brief Constructor.
         * \param chip The chip.
         */
        MifareUltralightStorageCardService(std::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
        ~MifareUltralightStorageCardService();

        /**
         * \brief Erase a specific location on the card.
         * \param location The data location.
         * \param aiToUse The key's informations to use to delete.
         */
        virtual void erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse);

        /**
         * \brief Write data on a specific Mifare location, using given Mifare keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param aiToWrite The key's informations to change.
         * \param data Data to write.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual void writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite, const std::vector<unsigned char>& data, CardBehavior behaviorFlags);

        /**
         * \brief Read data on a specific Mifare location, using given Mifare keys.
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
         * \param data Will contain data after reading.
         * \param dataLength Data's length to read.
         * \return Data header length.
         */
        virtual unsigned int readDataHeader(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength);

    protected:

        /*
         * \brief Get the associated Mifare Ultralight chip.
         * \return The Mifare Ultralight chip.
         */
        std::shared_ptr<MifareUltralightChip> getMifareUltralightChip() { return std::dynamic_pointer_cast<MifareUltralightChip>(getChip()); };
    };
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTSTORAGECARDSERVICE_HPP */
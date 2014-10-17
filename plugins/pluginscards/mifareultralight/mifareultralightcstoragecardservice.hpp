/**
 * \file mifareultralightcstoragecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C storage card service.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCSTORAGECARDSERVICE_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCSTORAGECARDSERVICE_HPP

#include "mifareultralightstoragecardservice.hpp"
#include "mifareultralightccommands.hpp"

namespace logicalaccess
{
    class MifareUltralightCChip;

    /**
     * \brief The Mifare Ultralight C storage card service base class.
     */
    class LIBLOGICALACCESS_API MifareUltralightCStorageCardService : public MifareUltralightStorageCardService
    {
    public:

        /**
         * \brief Constructor.
         * \param chip The chip.
         */
        MifareUltralightCStorageCardService(std::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
        ~MifareUltralightCStorageCardService();

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
         * \brief Write data on a specific Mifare location, using given Mifare keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param aiToWrite The key's informations to change.
         * \param data Data to write.
         * \param dataLength Data's length to write.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual void writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags);

        /**
         * \brief Read data on a specific Mifare location, using given Mifare keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param data Will contain data after reading.
         * \param dataLength Data's length to read.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual void readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags);

    protected:
        /*
         * \brief Get the associated Mifare Ultralight C chip.
         * \return The Mifare Ultralight C chip.
         */
        std::shared_ptr<MifareUltralightCChip> getMifareUltralightCChip() { return std::dynamic_pointer_cast<MifareUltralightCChip>(getMifareUltralightChip()); };
    };
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCSTORAGECARDSERVICE_HPP */
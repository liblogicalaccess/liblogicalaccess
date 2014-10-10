/**
 * \file mifareplussl1storagecardservice.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL1 storage card service.
 */

#ifndef LOGICALACCESS_MIFAREPLUSSL1STORAGECARDSERVICE_HPP
#define LOGICALACCESS_MIFAREPLUSSL1STORAGECARDSERVICE_HPP

#include "logicalaccess/key.hpp"
#include "mifarepluslocation.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "mifarepluschip.hpp"
#include "mifarepluskey.hpp"
#include "mifareplusaccessinfo.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The MifarePlus base profile class.
     */
    class LIBLOGICALACCESS_API MifarePlusSL1StorageCardService : public StorageCardService
    {
    public:

        /**
         * \brief Constructor.
         * \param chip The chip.
         */
        MifarePlusSL1StorageCardService(boost::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
        virtual ~MifarePlusSL1StorageCardService();

        /**
         * \brief Erase the card.
         * \return true if the card was erased, false otherwise. If false, the card may be partially erased.
         */
        virtual void erase();

        /**
         * \brief Erase a specific location on the card.
         * \param location The data location.
         * \param aiToUse The key's informations to use to delete.
         */
        virtual void erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse);

        /**
         * \brief Write data on a specific Mifare location, using given Mifare keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param aiToWrite The key's informations to change.
         * \param data Data to write.
         * \param dataLength Data's length to write.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual void writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags);

        /**
         * \brief Read data on a specific Mifare location, using given Mifare keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param data Will contain data after reading.
         * \param dataLength Data's length to read.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual void readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags);

        /**
         * \brief Read data header on a specific location, using given keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use.
         * \param data Will contain data after reading.
         * \param dataLength Data's length to read.
         * \return Data header length.
         */
        virtual unsigned int readDataHeader(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength);

    protected:

        /*
         * \brief Get the associated Mifare chip.
         * \return The Mifare chip.
         */
        boost::shared_ptr<MifarePlusChip> getMifarePlusChip() { return boost::dynamic_pointer_cast<MifarePlusChip>(getChip()); };
    };
}

#endif /* LOGICALACCESS_MIFAREPLUSSL1STORAGECARDSERVICE_HPP */
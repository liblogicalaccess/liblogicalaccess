/**
 * \file storagecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Storage Card service.
 */

#ifndef LOGICALACCESS_STORAGECARDSERVICE_HPP
#define LOGICALACCESS_STORAGECARDSERVICE_HPP

#include "logicalaccess/services/cardservice.hpp"
#include "logicalaccess/cards/location.hpp"
#include "logicalaccess/cards/accessinfo.hpp"

namespace logicalaccess
{
    /**
     * \brief The card behaviors.
     */
    typedef enum {
        CB_DEFAULT = 0x0000, /**< Default behavior */
        CB_AUTOSWITCHAREA = 0x0001 /**< Auto switch area when needed (for read/write command) */
    } CardBehavior;

    /**
     * \brief The base storage card service class for all storage services.
     */
    class LIBLOGICALACCESS_API StorageCardService : public CardService
    {
    public:

        /**
         * \brief Constructor.
         * \param chip The associated chip.
         */
        StorageCardService(boost::shared_ptr<Chip> chip) : CardService(chip) {};

        /**
         * \brief Get the card service type.
         * \return The card service type.
         */
        virtual CardServiceType getServiceType() const { return CST_STORAGE; };

        /**
         * \brief Erase the card.
         */
        virtual void erase() = 0;

        /**
         * \brief Erase a specific location on the card.
         * \param location The data location.
         * \param aiToUse The key's informations to use to delete.
         */
        virtual void erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse) = 0;

        /**
         * \brief Write data on a specific location, using given keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param aiToWrite The key's informations to change.
         * \param data Data to write.
         * \param dataLength Data's length to write.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual	void writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags) = 0;

        /**
         * \brief Read data on a specific location, using given keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for read access.
         * \param data Will contain data after reading.
         * \param dataLength Data's length to read.
         * \param behaviorFlags Flags which determines the behavior.
         */
        virtual void readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags) = 0;

        /**
         * \brief Read data header on a specific location, using given keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use.
         * \param data Will contain data after reading.
         * \param dataLength Data's length to read.
         * \return Data header length.
         */
        virtual unsigned int readDataHeader(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength) = 0;
    };
}

#endif
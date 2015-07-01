/**
 * \file felicastoragecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa storage card service.
 */

#ifndef LOGICALACCESS_FELICASTORAGECARDSERVICE_HPP
#define LOGICALACCESS_FELICASTORAGECARDSERVICE_HPP

#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "felicachip.hpp"

namespace logicalaccess
{
	class FeliCaChip;

    /**
     * \brief The FeliCa storage card service base class.
     */
	class LIBLOGICALACCESS_API FeliCaStorageCardService : public StorageCardService
    {
    public:

        /**
         * \brief Constructor.
         */
		FeliCaStorageCardService(std::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
		virtual ~FeliCaStorageCardService();

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
        virtual unsigned int readDataHeader(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength);

    protected:

		std::shared_ptr<FeliCaChip> getFeliCaChip() { return std::dynamic_pointer_cast<FeliCaChip>(getChip()); };
    };
}

#endif
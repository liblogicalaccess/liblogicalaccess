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
	    explicit TopazStorageCardService(std::shared_ptr<Chip> chip);

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
	    void erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse) override;

        /**
         * \brief Write data on a specific Topaz location, using given Topaz keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param aiToWrite The key's informations to change.
         * \param data Data to write.
         * \param behaviorFlags Flags which determines the behavior.
         */
	    void writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite, const ByteVector& data, CardBehavior behaviorFlags) override;

        /**
         * \brief Read data on a specific Topaz location, using given Topaz keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param length to read.
         * \param behaviorFlags Flags which determines the behavior.
		 * \return Data readed
         */
	    ByteVector readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, size_t length, CardBehavior behaviorFlags) override;

        /**
         * \brief Read data header on a specific location, using given keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use.
         * \param data Will contain data after reading.
         * \param dataLength Data's length to read.
         * \return Data header length.
         */
		ByteVector readDataHeader(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse) override;

    protected:

        /*
         * \brief Get the associated Topaz chip.
         * \return The Topaz chip.
         */
        std::shared_ptr<TopazChip> getTopazChip() const { return std::dynamic_pointer_cast<TopazChip>(getChip()); }
    };
}

#endif /* LOGICALACCESS_TOPAZSTORAGECARDSERVICE_HPP */
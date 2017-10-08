/**
 * \file mifareultralightcstoragecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C storage card service.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCSTORAGECARDSERVICE_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCSTORAGECARDSERVICE_HPP

#include "mifareultralightstoragecardservice.hpp"
#include "mifareultralightccommands.hpp"
#include "mifareultralightcchip.hpp"

namespace logicalaccess
{
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
	    explicit MifareUltralightCStorageCardService(std::shared_ptr<Chip> chip);

        /**
         * \brief Destructor.
         */
        ~MifareUltralightCStorageCardService();

        /**
         * \brief Erase a specific location on the card.
         * \param location The data location.
         * \param aiToUse The key's informations to use to delete.
         */
	    void erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse) override;

        /**
         * \brief Write data on a specific Mifare location, using given Mifare keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param aiToWrite The key's informations to change.
         * \param data Data to write.
         * \param behaviorFlags Flags which determines the behavior.
         */
	    void writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite, const ByteVector& data, CardBehavior behaviorFlags) override;

        /**
         * \brief Read data on a specific Mifare location, using given Mifare keys.
         * \param location The data location.
         * \param aiToUse The key's informations to use for write access.
         * \param length to read.
         * \param behaviorFlags Flags which determines the behavior.
		 * \return Data readed
         */
	    ByteVector readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, size_t length, CardBehavior behaviorFlags) override;

    protected:
        /*
         * \brief Get the associated Mifare Ultralight C chip.
         * \return The Mifare Ultralight C chip.
         */
        std::shared_ptr<MifareUltralightCChip> getMifareUltralightCChip() const { return std::dynamic_pointer_cast<MifareUltralightCChip>(getMifareUltralightChip()); }
    };
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCSTORAGECARDSERVICE_HPP */
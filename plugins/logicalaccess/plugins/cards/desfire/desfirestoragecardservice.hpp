/**
 * \file desfirestoragecardservice.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire storage card service.
 */

#ifndef LOGICALACCESS_DESFIRESTORAGECARDSERVICE_HPP
#define LOGICALACCESS_DESFIRESTORAGECARDSERVICE_HPP

#include <logicalaccess/services/storage/storagecardservice.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>

namespace logicalaccess
{
class DESFireChip;

#define STORAGE_CARDSERVICE_DESFIRE "DESFireStorage"

/**
 * \brief The DESFire storage card service bas class.
 */
class LLA_CARDS_DESFIRE_API DESFireStorageCardService : public StorageCardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The chip.
     */
    explicit DESFireStorageCardService(std::shared_ptr<Chip> chip);

    std::string getCSType() override
    {
        return STORAGE_CARDSERVICE_DESFIRE;
    }

    /**
     * \brief Erase the card.
     */
    virtual void erase();

    /**
     * \brief Erase a specific location on the card.
     * \param location The data location.
     * \param aiToUse The key's informations to use to delete.
     * \return true if the card was erased, false otherwise. If false, the card may be
     * partially erased.
     */
    void erase(std::shared_ptr<Location> location,
               std::shared_ptr<AccessInfo> aiToUse) override;

    /**
     * \brief Write data on a specific DESFire location, using given DESFire keys.
     * \param location The data location.
     * \param aiToUse The key's informations to use for write access.
     * \param aiToWrite The key's informations to change.
     * \param data Data to write.
     * \param behaviorFlags Flags which determines the behavior.
     */
    void writeData(std::shared_ptr<Location> location,
                   std::shared_ptr<AccessInfo> aiToUse,
                   std::shared_ptr<AccessInfo> aiToWrite, const ByteVector &data,
                   CardBehavior behaviorFlags) override;

    /**
     * \brief Read data on a specific DESFire location, using given DESFire keys.
     * \param location The data location.
     * \param aiToUse The key's informations to use for write access.
     * \param behaviorFlags Flags which determines the behavior.
             * \return Data readed
     */
    ByteVector readData(std::shared_ptr<Location> location,
                        std::shared_ptr<AccessInfo> aiToUse, size_t dataLength,
                        CardBehavior behaviorFlags) override;

    /**
     * \brief Read data header on a specific location, using given keys.
     * \param location The data location.
     * \param aiToUse The key's informations to use.
     * \param data Will contain data after reading.
     * \param dataLength Data's length to read.
     * \return Data header length.
     */
    ByteVector readDataHeader(std::shared_ptr<Location> location,
                              std::shared_ptr<AccessInfo> aiToUse) override;

  protected:
    std::shared_ptr<DESFireChip> getDESFireChip() const
    {
        return std::dynamic_pointer_cast<DESFireChip>(getChip());
    }
};
}

#endif /* LOGICALACCESS_DESFIRESTORAGECARDSERVICE_HPP */
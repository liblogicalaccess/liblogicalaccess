/**
 * \file mifarestoragecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare storage card service.
 */

#ifndef LOGICALACCESS_MIFARESTORAGECARDSERVICE_HPP
#define LOGICALACCESS_MIFARESTORAGECARDSERVICE_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarelocation.hpp>
#include <logicalaccess/services/storage/storagecardservice.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarechip.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
/**
 * \brief The Mifare storage card service base class.
 */
class LIBLOGICALACCESS_API MifareStorageCardService : public StorageCardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The chip.
     */
    explicit MifareStorageCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    virtual ~MifareStorageCardService();

    /**
     * \brief Erase a specific location on the card.
     * \param location The data location.
     * \param aiToUse The key's informations to use to delete.
     */
    void erase(std::shared_ptr<Location> location,
               std::shared_ptr<AccessInfo> aiToUse) override;

    /**
     * \brief Write data on a specific Mifare location, using given Mifare keys.
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
     * \brief Read data on a specific Mifare location, using given Mifare keys.
     * \param location The data location.
     * \param aiToUse The key's informations to use for write access.
     * \param data Will contain data after reading.
     * \param length to read.
     * \param behaviorFlags Flags which determines the behavior.
             * \return Data readed
     */
    ByteVector readData(std::shared_ptr<Location> location,
                        std::shared_ptr<AccessInfo> aiToUse, size_t length,
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
    /*
     * \brief Get the associated Mifare chip.
     * \return The Mifare chip.
     */
    std::shared_ptr<MifareChip> getMifareChip() const
    {
        return std::dynamic_pointer_cast<MifareChip>(getChip());
    }
};
}

#endif /* LOGICALACCESS_MIFARESTORAGECARDSERVICE_HPP */
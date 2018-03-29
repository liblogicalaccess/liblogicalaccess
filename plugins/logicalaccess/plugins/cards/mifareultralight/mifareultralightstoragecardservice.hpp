/**
 * \file mifareultralightstoragecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight storage card service.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTSTORAGECARDSERVICE_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTSTORAGECARDSERVICE_HPP

#include <logicalaccess/services/storage/storagecardservice.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightchip.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define STORAGECARDSERVICE_MIFARE_ULTRALIGHT "MifareUltralightStorage"

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
    explicit MifareUltralightStorageCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    ~MifareUltralightStorageCardService();

    std::string getCSType() override
    {
        return STORAGECARDSERVICE_MIFARE_ULTRALIGHT;
    }

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
     * \brief Get the associated Mifare Ultralight chip.
     * \return The Mifare Ultralight chip.
     */
    std::shared_ptr<MifareUltralightChip> getMifareUltralightChip() const
    {
        return std::dynamic_pointer_cast<MifareUltralightChip>(getChip());
    }
};
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTSTORAGECARDSERVICE_HPP */
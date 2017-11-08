/**
 * \file iso7816storagecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 storage card service.
 */

#ifndef LOGICALACCESS_ISO7816STORAGECARDSERVICE_HPP
#define LOGICALACCESS_ISO7816STORAGECARDSERVICE_HPP

#include <logicalaccess/services/storage/storagecardservice.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816chip.hpp>

namespace logicalaccess
{
/**
 * \brief The ISO7816 card provider base class.
 */
class LIBLOGICALACCESS_API ISO7816StorageCardService : public StorageCardService
{
  public:
    /**
     * \brief Constructor.
     */
    explicit ISO7816StorageCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    virtual ~ISO7816StorageCardService();

    /**
     * \brief Erase a specific location on the card.
     * \param location The data location.
     * \param aiToUse The key's informations to use to delete.
     */
    void erase(std::shared_ptr<Location> location,
               std::shared_ptr<AccessInfo> aiToUse) override;

    /**
     * \brief Write data on a specific Tag-It location, using given Tag-It keys.
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
     * \brief Read data on a specific Tag-It location, using given Tag-It keys.
     * \param location The data location.
     * \param aiToUse The key's informations to use for write access.
     * \param length to read.
     * \param behaviorFlags Flags which determines the behavior.
             * \return The bytes readed.
     */
    ByteVector readData(std::shared_ptr<Location> location,
                        std::shared_ptr<AccessInfo> aiToUse, size_t Length,
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
    /**
     * \brief Get the associated ISO7816 chip.
     * \return The ISO7816 chip.
     */
    std::shared_ptr<ISO7816Chip> getISO7816Chip() const
    {
        return std::dynamic_pointer_cast<ISO7816Chip>(getChip());
    }
};
}

#endif
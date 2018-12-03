/**
 * \file felicastoragecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa storage card service.
 */

#ifndef LOGICALACCESS_FELICASTORAGECARDSERVICE_HPP
#define LOGICALACCESS_FELICASTORAGECARDSERVICE_HPP

#include <logicalaccess/services/storage/storagecardservice.hpp>
#include <logicalaccess/plugins/cards/felica/felicachip.hpp>

namespace logicalaccess
{
class FeliCaChip;

#define STORAGECARDSERVICE_FELICA "FeliCaStorage"

/**
 * \brief The FeliCa storage card service base class.
 */
class LLA_CARDS_FELICA_API FeliCaStorageCardService : public StorageCardService
{
  public:
    /**
     * \brief Constructor.
     */
    explicit FeliCaStorageCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    virtual ~FeliCaStorageCardService();

    std::string getCSType() override
    {
        return STORAGECARDSERVICE_FELICA;
    }

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
    std::shared_ptr<FeliCaChip> getFeliCaChip() const
    {
        return std::dynamic_pointer_cast<FeliCaChip>(getChip());
    }
};
}

#endif
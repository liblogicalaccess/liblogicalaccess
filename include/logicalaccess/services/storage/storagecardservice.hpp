/**
 * \file storagecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Storage Card service.
 */

#ifndef LOGICALACCESS_STORAGECARDSERVICE_HPP
#define LOGICALACCESS_STORAGECARDSERVICE_HPP

#include <logicalaccess/services/cardservice.hpp>
#include <logicalaccess/cards/location.hpp>
#include <logicalaccess/cards/accessinfo.hpp>

namespace logicalaccess
{
/**
 * \brief The card behaviors.
 */
typedef enum {
    CB_DEFAULT = 0x0000, /**< Default behavior */
    CB_AUTOSWITCHAREA =
        0x0001 /**< Auto switch area when needed (for read/write command) */
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
    explicit StorageCardService(std::shared_ptr<Chip> chip);

    virtual ~StorageCardService();

    /**
     * \brief Erase a specific location on the card.
     * \param location The data location.
     * \param aiToUse The key's informations to use to delete.
     */
    virtual void erase(std::shared_ptr<Location> location,
                       std::shared_ptr<AccessInfo> aiToUse) = 0;

    /**
     * \brief Write data on a specific location, using given keys.
     * \param location The data location.
     * \param aiToUse The key's informations to use for write access.
     * \param aiToWrite The key's informations to change.
     * \param data Data to write.
     * \param behaviorFlags Flags which determines the behavior.
     */
    virtual void writeData(std::shared_ptr<Location> location,
                           std::shared_ptr<AccessInfo> aiToUse,
                           std::shared_ptr<AccessInfo> aiToWrite, const ByteVector &data,
                           CardBehavior behaviorFlags) = 0;

    /**
     * \brief Read data on a specific location, using given keys.
     * \param location The data location.
     * \param aiToUse The key's informations to use for write access.
             * \param length to read.
     * \param behaviorFlags Flags which determines the behavior.
             * \return Data readed
     */
    virtual ByteVector readData(std::shared_ptr<Location> location,
                                std::shared_ptr<AccessInfo> aiToUse, size_t length,
                                CardBehavior behaviorFlags) = 0;

    /**
     * \brief Read data header on a specific location, using given keys.
     * \param location The data location.
     * \param aiToUse The key's informations to use.
     * \param data Will contain data after reading.
     * \param dataLength Data's length to read.
     * \return Data header length.
     */
    virtual ByteVector readDataHeader(std::shared_ptr<Location> location,
                                      std::shared_ptr<AccessInfo> aiToUse) = 0;
};
}

#endif
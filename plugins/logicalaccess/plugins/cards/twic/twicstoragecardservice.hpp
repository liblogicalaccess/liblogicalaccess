/**
 * \file twicstoragecardservice.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Twic card provider.
 */

#ifndef LOGICALACCESS_TWICSTORAGECARDSERVICE_HPP
#define LOGICALACCESS_TWICSTORAGECARDSERVICE_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816storagecardservice.hpp>
#include <logicalaccess/plugins/cards/twic/twiclocation.hpp>
#include <logicalaccess/plugins/cards/twic/twiccommands.hpp>
#include <logicalaccess/plugins/cards/twic/twicchip.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define STORAGECARDSERVICE_TWIC "TwicStorage"

/**
 * \brief The Twic storage card service base class.
 */
class LLA_CARDS_TWIC_API TwicStorageCardService : public ISO7816StorageCardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The chip.
     */
    explicit TwicStorageCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    virtual ~TwicStorageCardService();

    std::string getCSType() override
    {
        return STORAGECARDSERVICE_TWIC;
    }

    /**
* \brief Read data on a specific DESFire location, using given DESFire keys.
* \param location The data location.
* \param aiToUse The key's informations to use for write access.
     * \param length to read.
* \param behaviorFlags Flags which determines the behavior.
     * \return Data readed
*/
    ByteVector readData(std::shared_ptr<Location> location,
                        std::shared_ptr<AccessInfo> aiToUse, size_t length,
                        CardBehavior behaviorFlags) override;

  protected:
    std::shared_ptr<TwicChip> getTwicChip() const
    {
        return std::dynamic_pointer_cast<TwicChip>(getISO7816Chip());
    }
};
}

#endif
/**
 * \file cps3storagecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief CPS3 storage card service.
 */

#ifndef LOGICALACCESS_CPS3STORAGECARDSERVICE_HPP
#define LOGICALACCESS_CPS3STORAGECARDSERVICE_HPP

#include <logicalaccess/plugins/cards/iso7816/iso7816storagecardservice.hpp>
#include <logicalaccess/plugins/cards/cps3/cps3chip.hpp>

namespace logicalaccess
{

#define STORAGE_CARDSERVICE_CPS3 "CPS3Storage"

/**
 * \brief The CPS3 storage card service base class.
 */
class LIBLOGICALACCESS_API CPS3StorageCardService : public ISO7816StorageCardService
{
  public:
    /**
     * \brief Constructor.
     */
    explicit CPS3StorageCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    virtual ~CPS3StorageCardService();

    std::string getCSType() override
    {
        return STORAGE_CARDSERVICE_CPS3;
    }

    /**
     * \brief Read data on a specific Tag-It location, using given Tag-It keys.
     * \param location The data location.
     * \param aiToUse The key's informations to use for write access.
     * \param behaviorFlags Flags which determines the behavior.
             * \return Data readed
     */
    ByteVector readData(std::shared_ptr<Location> location,
                        std::shared_ptr<AccessInfo> aiToUse, size_t dataLength,
                        CardBehavior behaviorFlags) override;

  protected:
    std::shared_ptr<CPS3Chip> getCPS3Chip() const
    {
        return std::dynamic_pointer_cast<CPS3Chip>(getISO7816Chip());
    }
};
}

#endif
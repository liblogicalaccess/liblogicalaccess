#pragma once

#include <logicalaccess/plugins/cards/epass/utils.hpp>
#include <logicalaccess/services/identity/identity_service.hpp>

namespace logicalaccess
{
#define IDENTITY_CARDSERVICE_EPASS "EPassIdentity"

class EPassChip;
class EPassAccessInfo;
class LLA_CARDS_EPASS_API EPassIdentityCardService : public IdentityCardService
{
  public:
    explicit EPassIdentityCardService(const std::shared_ptr<Chip> &chip);

    std::chrono::system_clock::time_point getTime(MetaData what) override;
    std::string getString(MetaData what) override;
    ByteVector getData(MetaData what) override;

    std::string getCSType() override
    {
        return IDENTITY_CARDSERVICE_EPASS;
    }


  protected:
    std::shared_ptr<EPassChip> getEPassChip() const;

    /**
     * Return an EPassAccessInfo or throws.
     */
    std::shared_ptr<EPassAccessInfo> getEPassAccessInfo() const;

    /**
     * Will be copied from cache is available.
     * It's lightweight to copy.
     */
    EPassDG1 getDG1();

    std::string getName();
    ByteVector getPicture();

    std::unique_ptr<EPassDG1> dg1_cache_;
};
}

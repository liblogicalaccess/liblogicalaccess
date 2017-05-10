#pragma once

#include "utils.hpp"
#include <logicalaccess/services/identity/identity_service.hpp>

namespace logicalaccess
{
class EPassChip;
class EPassAccessInfo;
class LIBLOGICALACCESS_API EPassIdentityService : public IdentityCardService
{
  public:
    EPassIdentityService(const std::shared_ptr<Chip> &chip);

	virtual std::chrono::system_clock::time_point getTime(MetaData what) override;
	virtual std::string getString(MetaData what) override;
	virtual ByteVector getData(MetaData what) override;


  protected:
    std::shared_ptr<EPassChip> getEPassChip();

    /**
     * Return an EPassAccessInfo or throws.
     */
    std::shared_ptr<EPassAccessInfo> getEPassAccessInfo();

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

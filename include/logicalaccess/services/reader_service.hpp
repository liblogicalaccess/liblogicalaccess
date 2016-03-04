#pragma once

#include "logicalaccess/lla_fwd.hpp"
#include "logicalaccess/logicalaccess_api.hpp"
#include <memory>

namespace logicalaccess
{

typedef enum {
    /**
     * @see LicenseCheckerService
     */
    RST_LICENSE_CHECKER = 0x0000,
} ReaderServiceType;

/**
 * This is a base class for reader-specific services.
 *
 * It is similar to CardService.
 */
class LIBLOGICALACCESS_API ReaderService
    : public std::enable_shared_from_this<ReaderService>
{
  public:
    ReaderService(ReaderUnitPtr reader_unit)
        : reader_unit_(reader_unit)
    {
    }

    virtual ~ReaderService() = default;

    /**
     * Retrieve the ReaderUnit object associated
     * with the service.
     */
    ReaderUnitPtr getReader()
    {
        return reader_unit_;
    }

    /**
     * Retrieve the service type that is implemented
     * by the current ReaderService instance.
     */
    virtual ReaderServiceType getServiceType() const = 0;

  protected:
    ReaderUnitPtr reader_unit_;
};
}

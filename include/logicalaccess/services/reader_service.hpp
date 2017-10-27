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
    ReaderService(ReaderUnitPtr reader_unit, ReaderServiceType service_type)
        : reader_unit_(reader_unit)
        , service_type_(service_type)
    {
    }

    virtual ~ReaderService() = default;

    /**
     * Retrieve the ReaderUnit object associated
     * with the service.
     */
    ReaderUnitPtr getReader() const
    {
        return reader_unit_;
    }

    /**
     * Retrieve the service type that is implemented
     * by the current ReaderService instance.
     */
    virtual ReaderServiceType getServiceType() const
    {
        return service_type_;
    }

  protected:
    ReaderUnitPtr reader_unit_;

    ReaderServiceType service_type_;
};
}

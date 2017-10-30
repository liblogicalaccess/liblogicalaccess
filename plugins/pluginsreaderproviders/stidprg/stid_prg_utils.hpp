#pragma once

#include "logicalaccess/lla_fwd.hpp"
#include "logicalaccess/logicalaccess_api.hpp"
#include <logicalaccess/services/accesscontrol/formats/wiegand35format.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand26format.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp>

namespace logicalaccess
{
class LIBLOGICALACCESS_API STidPRGUtils
{
  public:
    /**
     * Build the proper bytes vector to write on card to encode it as
     * HID prox.
     *
     * We support Wiegand26Format as well as Wiegand37WithFacilityFormat.
     */
    static ByteVector prox_configuration_bytes(const Format &fmt);
};
}

#pragma once

#include <logicalaccess/lla_core_api.hpp>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{
class LLA_CORE_API ASN1
{
  public:
    
	  static ByteVector encode_oid(const std::string &oid);

	  static ByteVector encode_oid(const std::vector<unsigned int> &oid);
};

} // namespace logicalaccess

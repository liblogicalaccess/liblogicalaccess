#pragma once

#ifdef _WIN32

#include <logicalaccess/lla_core_api.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace logicalaccess
{
/**
 * This class is a static helper to interact with the Windows registry.
 *
 * Currently this helper is usable only for keys in the
 * HKEY_LOCAL_MACHINE namespace.
 */
class LLA_CORE_API WindowsRegistry
{
  public:
    WindowsRegistry();
    ~WindowsRegistry();

    static bool writeBinary(const std::string &keypath, const std::string &keyname,
                            const ByteVector &data);

    static bool readBinary(const std::string &keypath, const std::string &keyname,
                           ByteVector &out);
};
}

#endif

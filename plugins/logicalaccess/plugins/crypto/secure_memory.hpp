#pragma once

#include <logicalaccess/lla_fwd.hpp>

#include <openssl/crypto.h>

#include <initializer_list>
#include <vector>

namespace logicalaccess
{
namespace security
{

inline void secureZeroBuffer(ByteVector &buffer) noexcept
{
    if (!buffer.empty())
        OPENSSL_cleanse(buffer.data(), buffer.size());
}

// RAII helper that securely erases temporary sensitive buffers when leaving scope, including during exception unwinding
class SecureZeroGuard final
{
  public:
    explicit SecureZeroGuard(std::initializer_list<ByteVector *> buffers)
        : d_buffers(buffers.begin(), buffers.end())
    {
    }

    ~SecureZeroGuard() noexcept
    {
        for (auto *buffer : d_buffers)
            if (buffer != nullptr)
                secureZeroBuffer(*buffer);
    }

    SecureZeroGuard(const SecureZeroGuard &)            = delete;
    SecureZeroGuard &operator=(const SecureZeroGuard &) = delete;
    SecureZeroGuard(SecureZeroGuard &&)            = delete;
    SecureZeroGuard &operator=(SecureZeroGuard &&) = delete;
  private:
    std::vector<ByteVector *> d_buffers;
};

} // namespace security
} // namespace logicalaccess
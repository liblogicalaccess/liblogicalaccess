#pragma once

#include <logicalaccess/iks/packet/Base.hpp>
#include <array>
#include <string>

namespace logicalaccess
{
namespace iks
{
class LIBLOGICALACCESS_API AesEncryptCommand : public BaseCommand
{
  public:
    AesEncryptCommand();

    ByteVector serialize() const override;

    size_t binary_size_impl() const override;

    /**
     * If set to true, request decryption instead of encryption.
     * Defaults to requesting encryption.
     */
    bool decrypt_;
    std::string key_name_;
    ByteVector payload_;
    std::array<uint8_t, 16> iv_;
};

class LIBLOGICALACCESS_API AesEncryptResponse : public BaseResponse
{
  public:
    AesEncryptResponse(uint16_t status, const ByteVector &data);

    ByteVector bytes_;
};
}
}

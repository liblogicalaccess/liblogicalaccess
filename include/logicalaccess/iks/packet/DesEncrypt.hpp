#pragma once

#include "logicalaccess/iks/packet/Base.hpp"
#include <array>
#include <string>

namespace logicalaccess
{
namespace iks
{
class LIBLOGICALACCESS_API DesEncryptCommand : public BaseCommand
{
  public:
    DesEncryptCommand();

	ByteVector serialize() const override;

	size_t binary_size_impl() const override;

    /**
     * If set to true, request decryption instead of encryption.
     * Defaults to requesting encryption.
     */
    bool decrypt_;
    uint8_t flags_;
    std::string key_name_;
    ByteVector payload_;
    std::array<uint8_t, 8> iv_;

#define COMMAND_DES_ENCRYPT_FLAG_ECB 1
#define COMMAND_DES_ENCRYPT_FLAG_CBC 2
};

class LIBLOGICALACCESS_API DesEncryptResponse : public BaseResponse
{
  public:
    DesEncryptResponse(uint16_t status, const ByteVector &data);

    ByteVector bytes_;
};
}
}

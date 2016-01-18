#pragma once

#include <string>
#include <array>
#include "logicalaccess/iks/packet/Base.hpp"

namespace logicalaccess
{
namespace iks
{
class LIBLOGICALACCESS_API DesEncryptCommand : public BaseCommand
{
  public:
    DesEncryptCommand();

    virtual std::vector<uint8_t> serialize() const override;

    virtual size_t binary_size_impl() const;

    /**
     * If set to true, request decryption instead of encryption.
     * Defaults to requesting encryption.
     */
    bool decrypt_;
    uint8_t flags_;
    std::string key_name_;
    std::vector<uint8_t> payload_;
    std::array<uint8_t, 8> iv_;

#define COMMAND_DES_ENCRYPT_FLAG_ECB 1
#define COMMAND_DES_ENCRYPT_FLAG_CBC 2
};

class LIBLOGICALACCESS_API DesEncryptResponse : public BaseResponse
{
  public:
    DesEncryptResponse(uint16_t status, const std::vector<uint8_t> &data);

    std::vector<uint8_t> bytes_;
};
}
}

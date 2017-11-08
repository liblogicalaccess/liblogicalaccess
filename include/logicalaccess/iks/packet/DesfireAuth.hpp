#pragma once

#include <logicalaccess/iks/packet/Base.hpp>
#include <array>
#include <string>

namespace logicalaccess
{
namespace iks
{

enum desfire_auth_algo_e
{
    DESFIRE_AUTH_ALGO_AES,
    DESFIRE_AUTH_ALGO_DES
};

class LIBLOGICALACCESS_API DesfireAuthCommand : public BaseCommand
{
  public:
    DesfireAuthCommand();
    virtual ~DesfireAuthCommand() = default;

    ByteVector serialize() const override;

    size_t binary_size_impl() const override;

    std::string key_idt_;

    /**
     * Is this step 1 or 2?
     */
    uint8_t step_;

    /**
     * What algorithm are we using (AES, DES3, DES2?)
     */
    uint8_t algo_;

    /**
     * Can be up to 32 bytes. (AES mode. Random from reader + from client)
     */
    std::array<uint8_t, 32> data_;

    KeyDivInfo div_info_;
};

class LIBLOGICALACCESS_API DesfireAuthResponse : public BaseResponse
{
  public:
    DesfireAuthResponse(uint16_t status, const ByteVector &data);

    std::array<uint8_t, 32> data_;

    std::array<uint8_t, 16> random2_;

    uint8_t success_;
};
}
}

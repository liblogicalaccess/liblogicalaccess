#pragma once

#include "logicalaccess/iks/packet/Base.hpp"

namespace logicalaccess
{
namespace iks
{
class GenRandomCommand : public BaseCommand
{
  public:
    GenRandomCommand();
    virtual std::vector<uint8_t> serialize() const override;

    virtual size_t binary_size_impl() const;

    uint16_t nb_bytes_;
};

class GenRandomResponse : public BaseResponse
{
  public:
    GenRandomResponse(uint16_t status, const std::vector<uint8_t> &data);

    std::vector<uint8_t> bytes_;
};
}
}

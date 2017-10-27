#pragma once

#include "logicalaccess/iks/packet/Base.hpp"

namespace logicalaccess
{
namespace iks
{
class LIBLOGICALACCESS_API GenRandomCommand : public BaseCommand
{
  public:
    GenRandomCommand();
    ByteVector serialize() const override;

    size_t binary_size_impl() const override;

    uint16_t nb_bytes_;
};

class LIBLOGICALACCESS_API GenRandomResponse : public BaseResponse
{
  public:
    GenRandomResponse(uint16_t status, const ByteVector &data);

    ByteVector bytes_;
};
}
}

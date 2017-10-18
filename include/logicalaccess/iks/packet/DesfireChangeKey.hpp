#pragma once

#include "logicalaccess/iks/packet/Base.hpp"
#include <array>
#include <string>

namespace logicalaccess
{
namespace iks
{
class LIBLOGICALACCESS_API DesfireChangeKeyCommand : public BaseCommand
{
  public:
    DesfireChangeKeyCommand();
    virtual ~DesfireChangeKeyCommand() = default;

	ByteVector serialize() const override;

	size_t binary_size_impl() const override;

#define IKS_COMMAND_DESFIRE_CHANGEKEY_SAME_KEY 1
#define IKS_COMMAND_DESFIRE_CHANGEKEY_OTHER_KEY 2

    /**
     * Some flags
     */
    uint8_t flag_;

    std::string oldkey_idt_;
    std::string newkey_idt_;

    /**
     * The current session key in use.
     */
    ByteVector session_key_;

    ByteVector iv_;

    uint8_t keyno_;

    KeyDivInfo oldkey_divinfo_;
    KeyDivInfo newkey_divinfo_;
};

class LIBLOGICALACCESS_API DesfireChangeKeyResponse : public BaseResponse
{
  public:
    DesfireChangeKeyResponse(uint16_t status, const ByteVector &data);

    ByteVector bytes_;
};
}
}

#pragma once

#include <string>
#include <array>
#include "logicalaccess/iks/packet/Base.hpp"

namespace logicalaccess
{
namespace iks
{
class DesfireChangeKeyCommand : public BaseCommand
{
  public:
    DesfireChangeKeyCommand();
    virtual ~DesfireChangeKeyCommand() = default;

    virtual std::vector<uint8_t> serialize() const override;

    virtual size_t binary_size_impl() const;

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
    std::vector<uint8_t> session_key_;

    std::vector<uint8_t> iv_;

    uint8_t keyno_;

    KeyDivInfo oldkey_divinfo_;
    KeyDivInfo newkey_divinfo_;
};

class DesfireChangeKeyResponse : public BaseResponse
{
  public:
    DesfireChangeKeyResponse(uint16_t status, const std::vector<uint8_t> &data);

    std::vector<uint8_t> bytes_;
};
}
}

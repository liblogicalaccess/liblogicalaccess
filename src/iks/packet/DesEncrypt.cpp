#include <cstring>
#include <arpa/inet.h>
#include "logicalaccess/iks/packet/DesEncrypt.hpp"
#include "logicalaccess/logs.hpp"


using namespace logicalaccess;
using namespace logicalaccess::iks;

DesEncryptCommand::DesEncryptCommand()
    : decrypt_(false)
    , flags_(0)
{
    opcode_ = CMSG_OP_DES_ENCRYPT;
}

std::vector<uint8_t> DesEncryptCommand::serialize() const
{
    auto header       = BaseCommand::serialize();
    auto payload_size = htons(static_cast<uint16_t>(payload_.size()));
    auto needle       = header.size();

    header.resize(binary_size());
    uint8_t decrypt = decrypt_ ? 1 : 0;
    memcpy(&header[needle], &decrypt, 1);
    needle++;

    memcpy(&header[needle], &flags_, 1);
    needle++;

    memcpy(&header[needle], key_name_.c_str(), key_name_.size() + 1);
    needle += key_name_.size() + 1;

    memcpy(&header[needle], &payload_size, sizeof(payload_size));
    needle += sizeof(payload_size);

    memcpy(&header[needle], &payload_[0], payload_.size());
    needle += payload_.size();

    memcpy(&header[needle], &iv_[0], iv_.size());
    needle += iv_.size();
    return header;
}

size_t DesEncryptCommand::binary_size_impl() const
{
    // 2 byte payload size + iv + payload + key name + '\0' for key name + flags
    return sizeof(uint16_t) + 8 + payload_.size() + key_name_.size() + 1 + 1 + 1;
}

DesEncryptResponse::DesEncryptResponse(uint16_t status,
                                       const std::vector<uint8_t> &data)
    : BaseResponse(SMSG_OP_DES_ENCRYPT, status)
    , bytes_(data)
{
}

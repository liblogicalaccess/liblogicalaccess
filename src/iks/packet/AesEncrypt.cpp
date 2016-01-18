#include <cstring>
#include <boost/asio.hpp>
#include "logicalaccess/iks/packet/AesEncrypt.hpp"
#include "logicalaccess/logs.hpp"

using namespace logicalaccess;
using namespace logicalaccess::iks;

AesEncryptCommand::AesEncryptCommand()
    : decrypt_(false)
{
    opcode_ = CMSG_OP_AES_ENCRYPT;
}

std::vector<uint8_t> AesEncryptCommand::serialize() const
{
    auto header       = BaseCommand::serialize();
    auto payload_size = htons(static_cast<uint16_t>(payload_.size()));
    auto needle       = header.size();

    header.resize(binary_size());
    uint8_t decrypt = decrypt_ ? 1 : 0;
    memcpy(&header[needle], &decrypt, 1);
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

size_t AesEncryptCommand::binary_size_impl() const
{
    // 2 byte payload size + iv + payload + key name + '\0' for key name
    return sizeof(uint16_t) + 16 + payload_.size() + key_name_.size() + 1 + 1;
}

AesEncryptResponse::AesEncryptResponse(uint16_t status,
                                       const std::vector<uint8_t> &data)
    : BaseResponse(SMSG_OP_AES_ENCRYPT, status)
    , bytes_(data)
{
}

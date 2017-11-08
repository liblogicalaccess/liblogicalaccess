#include <logicalaccess/iks/packet/DesfireChangeKey.hpp>
#include <cstring>

using namespace logicalaccess;
using namespace logicalaccess::iks;

DesfireChangeKeyCommand::DesfireChangeKeyCommand()
    : flag_(0)
    , keyno_(0)
{
    opcode_ = CMSG_OP_DESFIRE_CHANGEKEY;
}

ByteVector DesfireChangeKeyCommand::serialize() const
{
    auto header = BaseCommand::serialize();
    auto needle = header.size();

    header.resize(binary_size());
    memcpy(&header[needle], &flag_, 1);
    needle++;

    memcpy(&header[needle], oldkey_idt_.c_str(), oldkey_idt_.size() + 1);
    needle += oldkey_idt_.size() + 1;

    memcpy(&header[needle], newkey_idt_.c_str(), newkey_idt_.size() + 1);
    needle += newkey_idt_.size() + 1;

    uint8_t key_size = static_cast<uint8_t>(session_key_.size());
    memcpy(&header[needle], &key_size, sizeof(key_size));
    needle++;
    memcpy(&header[needle], &session_key_[0], key_size);
    needle += session_key_.size();

    uint8_t iv_size = static_cast<uint8_t>(iv_.size());
    memcpy(&header[needle], &iv_size, sizeof(iv_size));
    needle++;
    memcpy(&header[needle], &iv_[0], iv_size);
    needle += iv_.size();

    memcpy(&header[needle], &keyno_, sizeof(keyno_));
    needle++;

    auto tmp = oldkey_divinfo_.serialize();
    memcpy(&header[needle], &tmp[0], tmp.size());
    needle += tmp.size();

    tmp = newkey_divinfo_.serialize();
    memcpy(&header[needle], &tmp[0], tmp.size());
    // needle += tmp.size();

    return header;
}

size_t DesfireChangeKeyCommand::binary_size_impl() const
{
    return 1 + oldkey_idt_.size() + 1 + newkey_idt_.size() + 1 + session_key_.size() + 1 +
           1 + iv_.size() + 1 + oldkey_divinfo_.binary_size() +
           newkey_divinfo_.binary_size();
}

DesfireChangeKeyResponse::DesfireChangeKeyResponse(uint16_t status,
                                                   const ByteVector &data)
    : BaseResponse(SMSG_OP_DESFIRE_CHANGEKEY, status)
    , bytes_(data)
{
}

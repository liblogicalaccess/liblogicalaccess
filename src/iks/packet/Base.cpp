#include <logicalaccess/iks/packet/Base.hpp>

#include <logicalaccess/key.hpp>
#include <assert.h>
#include <cstring>
#include <logicalaccess/cards/keydiversification.hpp>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/utils.hpp>

using namespace logicalaccess;
using namespace logicalaccess::iks;

ByteVector BaseCommand::serialize() const
{
    ByteVector ret(6);
    uint32_t full_size = lla_htonl(static_cast<uint32_t>(binary_size()));
    uint16_t op        = lla_htons(opcode_);

    memcpy(&ret[0], &full_size, sizeof(full_size));
    memcpy(&ret[sizeof(full_size)], &op, sizeof(op));
    return ret;
}

size_t BaseCommand::binary_size() const
{
    return binary_size_impl() + sizeof(uint32_t) + sizeof(opcode_);
}

BaseResponse::BaseResponse(uint16_t opcode, uint16_t status)
    : opcode_(opcode)
    , status_(status)
{
    if (status_ != SMSG_STATUS_SUCCESS)
    {
        THROW_EXCEPTION_WITH_LOG(IKSException,
                                 "Invalid response from server: " + strstatus(status_));
    }
}

std::string iks::strstatus(enum response_status_e st)
{
    return strstatus(static_cast<uint16_t>(st));
}

std::string iks::strstatus(uint16_t st)
{
    enum response_status_e ste = static_cast<enum response_status_e>(st);
    switch (ste)
    {
    case SMSG_STATUS_SUCCESS: return "SUCCESS";
    case SMSG_STATUS_FAILURE: return "FAILURE";
    case SMSG_STATUS_INVALID_PAYLOAD_SIZE: return "INVALID_PAYLOAD_SIZE";
    case SMSG_STATUS_TOO_MANY_BYTES: return "TOO_MANY_BYTES";
    case SMSG_STATUS_CANNOT_LOAD_KEY: return "CANNOT_LOAD_KEY";
    case SMSG_STATUS_INVALID_FLAGS: return "INVALID_FLAGS";
    default: return "UNHANDLED_STATUS_CODE";
    }
}

size_t KeyDivInfo::binary_size()
{
    return 1 + 1 + 64;
}

ByteVector KeyDivInfo::serialize() const
{
    ByteVector ret(binary_size());
    assert(div_input_.size() <= 64);

    uint8_t div_info_size = static_cast<uint8_t>(div_input_.size());
    div_input_.resize(64);

    ret[0] = flag_;
    ret[1] = div_info_size;
    memcpy(&ret[2], &div_input_[0], 64);
    return ret;
}

KeyDivInfo::KeyDivInfo()
    : flag_(KEYDIV_ALGO_NONE)
{
}

KeyDivInfo KeyDivInfo::build(std::shared_ptr<Key> key, const ByteVector &divinput)
{
    if (!key->getKeyDiversification())
        return KeyDivInfo();

    auto kd       = KeyDivInfo();
    kd.div_input_ = divinput;

    if (key->getKeyDiversification()->getType() == "NXPAV1")
    {
        kd.flag_ = KEYDIV_ALGO_NXP_AV1;
    }
    if (key->getKeyDiversification()->getType() == "NXPAV2")
    {
        kd.flag_ = KEYDIV_ALGO_NXP_AV2;
    }
    return kd;
}

KeyDivInfo KeyDivInfo::build(std::shared_ptr<Key> key, ByteVector identifier, int AID,
                             unsigned char keyno)
{
    if (!key->getKeyDiversification())
        return KeyDivInfo();

    ByteVector div_input;
    key->getKeyDiversification()->initDiversification(identifier, AID, key, keyno,
                                                      div_input);
    return build(key, div_input);
}

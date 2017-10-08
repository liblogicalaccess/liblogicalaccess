#include "logicalaccess/iks/packet/DesfireAuth.hpp"
#include "logicalaccess/logs.hpp"
#include <cstring>
#include <logicalaccess/myexception.hpp>


using namespace logicalaccess;
using namespace iks;

DesfireAuthCommand::DesfireAuthCommand(): step_(0), algo_(0)
{
	opcode_ = CMSG_OP_DESFIRE_AUTH;
}

std::vector<uint8_t> DesfireAuthCommand::serialize() const
{
    auto header = BaseCommand::serialize();
    auto needle = header.size();

    header.resize(binary_size());
    memcpy(&header[needle], key_idt_.c_str(), key_idt_.size() + 1);
    needle += key_idt_.size() + 1;

    memcpy(&header[needle], &step_, sizeof(step_));
    needle += sizeof(step_);

    memcpy(&header[needle], &algo_, sizeof(algo_));
    needle += sizeof(algo_);

    memcpy(&header[needle], &data_[0], data_.size());
    needle += data_.size();

    auto tmp = div_info_.serialize();
    memcpy(&header[needle], &tmp[0], tmp.size());
    //needle += tmp.size();

    return header;
}

size_t DesfireAuthCommand::binary_size_impl() const
{
    // keyname + \0 + algo (1) + step (1) + data (32)
    return key_idt_.size() + 1 + 1 + 1 + 32 + div_info_.binary_size();
}

DesfireAuthResponse::DesfireAuthResponse(uint16_t status,
                                         const std::vector<uint8_t> &data)
    : BaseResponse(SMSG_OP_DESFIRE_AUTH, status)
{
    if (data.size() != 32 + 16 + 1)
        THROW_EXCEPTION_WITH_LOG(IKSException, "Invalid response size.");

    copy(data.begin(), data.begin() + 32, data_.begin());
    copy(data.begin() + 32, data.begin() + 32 + 16, random2_.begin());
    success_ = *(data.end() - 1);
}

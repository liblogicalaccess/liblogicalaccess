#include "logicalaccess/iks/packet/GenRandom.hpp"
#include <cstring>

using namespace logicalaccess;
using namespace logicalaccess::iks;

GenRandomCommand::GenRandomCommand()
    : nb_bytes_(0)
{
    opcode_ = CMSG_OP_GENRANDOM;
}

std::vector<uint8_t> GenRandomCommand::serialize() const
{
    auto header      = BaseCommand::serialize();
    auto req_bytes   = lla_htons(nb_bytes_);
    auto buffer_size = header.size();

    header.resize(buffer_size + sizeof(req_bytes));
    memcpy(&header[buffer_size], &req_bytes, sizeof(req_bytes));

    return header;
}

size_t GenRandomCommand::binary_size_impl() const
{
    return sizeof(nb_bytes_);
}

GenRandomResponse::GenRandomResponse(uint16_t status,
                                     const std::vector<uint8_t> &data)
    : BaseResponse(SMSG_OP_GENRANDOM, status)
    , bytes_(data)
{
}

//
// Created by xaqq on 3/20/15.
//

#include <cassert>
#include <stdint.h>
#include <logicalaccess/TLV.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <iostream>


namespace logicalaccess
{

TLV::TLV(uint8_t t)
    : tag_(t)
{
}

uint8_t TLV::tag() const
{
    return tag_;
}

void TLV::tag(uint8_t t)
{
    tag_ = t;
}

ByteVector TLV::value() const
{
    if (subTLV_)
        return subTLV_->compute();
    return value_;
}

void TLV::value(const ByteVector &v)
{
    subTLV_ = nullptr;
    value_  = v;
}

void TLV::value(TLVPtr tlv)
{
    assert(tlv);
    subTLV_ = tlv;
    value_.clear();
}

ByteVector TLV::compute() const
{
    auto ret = ByteVector();
    auto v   = value();
    assert(v.size() < 256);

    ret.push_back(tag_);
    ret.push_back(static_cast<uint8_t>(v.size()));
    ret.insert(ret.end(), v.begin(), v.end());

    return ret;
}

std::vector<TLVPtr> parse_tlvs(const ByteVector &bytes, size_t &bytes_consumed)
{
    size_t idx = 0;
    std::vector<TLVPtr> tlvs;

    while (idx < bytes.size())
    {
        TLVPtr tlv = std::make_shared<TLV>(bytes[idx]);

        // Go to size byte.
        idx++;
        // We need at least 1 bytes indicating the size.
        if (idx >= bytes.size())
            break;
        size_t current_tlv_size = bytes[idx];

        // Go to first data byte.
        idx++;

        // We need at least N bytes for the value
        if (idx + current_tlv_size > bytes.size())
            break;
        auto data =
            ByteVector(bytes.begin() + idx, bytes.begin() + idx + current_tlv_size);
        idx += data.size();
        tlv->value(data);
        tlvs.push_back(tlv);
    }
    bytes_consumed = idx;
    return tlvs;
}

std::vector<TLVPtr> parse_tlvs(const ByteVector &bytes)
{
    size_t consumed;
    return parse_tlvs(bytes, consumed);
}
}

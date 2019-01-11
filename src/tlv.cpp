//
// Created by xaqq on 3/20/15.
//

#include <cassert>
#include <stdint.h>
#include <logicalaccess/tlv.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <iostream>
#include <logicalaccess/plugins/llacommon/settings.hpp>


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
    if (subTLVs_.size() > 0)
    {
        ByteVector v;
        for (auto it = subTLVs_.cbegin(); it != subTLVs_.end(); ++it)
        {
            auto subv = (*it)->compute();
            v.insert(v.end(), subv.begin(), subv.end());
        }
        return v;
    }
    return value_;
}

void TLV::value(bool v)
{
    value(static_cast<unsigned char>(v ? 0x01 : 0x00));
}

void TLV::value(unsigned char v)
{
    value(ByteVector{v});
}

void TLV::value(const ByteVector &v)
{
    subTLVs_.clear();
    value_ = v;
}

void TLV::value(TLVPtr tlv)
{
    assert(tlv);
    subTLVs_.clear();
    subTLVs_.push_back(tlv);
    value_.clear();
}

void TLV::value(std::vector<TLVPtr> tlv)
{
    subTLVs_ = tlv;
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

std::vector<TLVPtr> TLV::parse_tlvs(const ByteVector &bytes, size_t &bytes_consumed)
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

std::vector<TLVPtr> TLV::parse_tlvs(const ByteVector &bytes, bool strict)
{
    size_t consumed = 0;
    auto tlv        = parse_tlvs(bytes, consumed);

    if (strict && consumed != bytes.size())
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "TLV parsing didn't reached the end of the buffer.");
    }

    return tlv;
}
}

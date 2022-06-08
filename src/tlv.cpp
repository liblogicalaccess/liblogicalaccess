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
    : tag_(t), sizeTag_(0)
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

uint8_t TLV::value_u1() const
{
    if (value_.size() != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "TLV value must be exactly one-byte long.");
    }
    return value_.at(0);
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
    ++idx;
    size_t current_tlv_size = 0;
    if (bytes[idx] > 0x80)
    {
      tlv->setSizeTag(bytes[idx]);
      size_t lenghtSize = bytes[idx] & 0x7F;
      ++idx;
      if (idx + lenghtSize > bytes.size()) {
          break;
      }
      ByteVector Vsize(bytes.begin() + idx, bytes.begin() + idx + lenghtSize);
      size_t multiplicator = 0x01;
      idx += lenghtSize;
      tlv->setSizeVector(Vsize);
      while (lenghtSize > 0)
      {
        --lenghtSize;
        current_tlv_size += Vsize[lenghtSize] * multiplicator;;
        multiplicator *= 0x100;
      }
    }
    else
    {
      ByteVector t;
      t.push_back(bytes[idx]);
      tlv->setSizeVector(t);
      current_tlv_size = bytes[idx];
      ++idx;
    }
    if (idx + current_tlv_size > bytes.size()) {
        break;
    }
    ByteVector data = ByteVector(bytes.begin() + idx, bytes.begin() + idx + current_tlv_size);
    idx += data.size();
    tlv->value(data);
    tlvs.push_back(tlv);
  }
  bytes_consumed = idx;
  return tlvs;
}

uint8_t TLV::getSizeTag() const
{
  return sizeTag_;
}

void TLV::setSizeTag(uint8_t tag)
{
  sizeTag_ = tag;
}

ByteVector TLV::getSizeVector() const
{
  return sizeVector_;
}

void TLV::setSizeVector(ByteVector size)
{
  if (size.size() == 1)
    sizeTag_ = size[0];
  sizeVector_ = size;
}

ByteVector TLV::getCompletTLV() const
{
  ByteVector stock;
  stock.push_back(tag_);
  stock.push_back(sizeTag_);
  if (sizeVector_.size() > 1)
    stock.insert(stock.end(), sizeVector_.begin(), sizeVector_.end());
  stock.insert(stock.end(), value_.begin(), value_.end());
  return stock;
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

ByteVector TLV::value_tlvs(std::vector<TLVPtr> tlvs)
{
    ByteVector res;
    for (auto it = tlvs.cbegin(); it != tlvs.cend(); ++it)
    {
        auto v = (*it)->compute();
        res.insert(res.end(), v.begin(), v.end());
    }
    return res;
}

TLVPtr TLV::get_child(uint8_t tag) const
{
    auto tlvs = subTLVs_;
	if (tlvs.size() < 1)
	{
        tlvs = TLV::parse_tlvs(value_);
	}

	return get_child(tlvs, tag);
}

std::vector<TLVPtr> TLV::get_childs() const
{
    return subTLVs_;
}

TLVPtr TLV::get_child(std::vector<TLVPtr> tlvs, uint8_t tag, bool required)
{
    for (auto it = tlvs.cbegin(); it != tlvs.cend(); ++it)
    {
        if ((*it)->tag() == tag)
            return *it;
    }

    if (required)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                "Cannot found expected child TLV.");
    }
    
    return nullptr;
}
}

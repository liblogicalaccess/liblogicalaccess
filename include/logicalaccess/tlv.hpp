//
// Created by xaqq on 3/20/15.
//
#pragma once

#include <logicalaccess/lla_core_api.hpp>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{
/**
 * This is a helper that represents TLV (tag, length, value) datatype.
 *
 * A few specifications:
 *       + The tag is one byte.
 *       + The length is one byte.
 *       + The value size is variable.
 *
 * A TLV's value can be an other TLV. This scheme is supported.
 */
class LLA_CORE_API TLV
{
  public:
    /**
     * Construct a TLV and specify the tag.
     */
    explicit TLV(uint8_t t);

    virtual ~TLV() = default;

    TLV(const TLV &) = delete;
    TLV(TLV &&)      = delete;
    TLV &operator=(const TLV &) = delete;
    TLV &operator=(TLV &&) = delete;


    uint8_t tag() const;
    void tag(uint8_t t);

    /**
     * Return a copy of the "value" of this TLV.
     * If this TLV wraps an other TLV, returns the `compute()` version of the sub TLV.
     */
    ByteVector value() const;

	/**
     * Set the value.
     */
    void value(bool v);

	/**
     * Set the value.
     */
    void value(unsigned char v);

    /**
     * Set the value.
     */
    void value(const ByteVector &v);

    /**
     * Set an other TLV as the value of this TLV.
     * @param tlv the other TLV. Must not be NULL. Will not be copied.
     */
    void value(TLVPtr tlv);

	/**
     * Set the value.
     */
    void value(std::vector<TLVPtr> tlv);

    /**
     * Return a raw representation of this TLV.
     */
    ByteVector compute() const;

	static std::vector<TLVPtr> parse_tlvs(const ByteVector &bytes,
                                                size_t &bytes_consumed);
    static std::vector<TLVPtr> parse_tlvs(const ByteVector &bytes, bool strict = false);

  private:
    uint8_t tag_;
    ByteVector value_; // Either one of those
    std::vector<TLVPtr> subTLVs_;    // will be used, but not both.
};

} // namespace logicalaccess

/**
 * \file memorycontroltlv.hpp
 * \author Remi W
 * \brief Memory control tlv.
 */

#ifndef LOGICALACCESS_MEMORYCONTROLTLV_HPP
#define LOGICALACCESS_MEMORYCONTROLTLV_HPP

#include <logicalaccess/services/nfctag/nfcdata.hpp>
#include <cmath>

namespace logicalaccess
{
  #define RFU 0x00

  class LLA_CORE_API MemoryControlTlv : public NfcData
  {
  public:
    MemoryControlTlv();
    MemoryControlTlv(ByteVector);
    MemoryControlTlv(int, unsigned char);
    ~MemoryControlTlv();
    int getByteAddr() const;
    void setByteAddr(int);
    unsigned char getPageAddr() const;
    void setPageAddr(unsigned char);
    unsigned char getOffset() const;
    void setOffset(unsigned char);
    unsigned char getSize() const;
    void setSize(unsigned char);
    unsigned char getBytesPerPage() const;
    void setBytesPerPage(unsigned char);


    void serialize(boost::property_tree::ptree &parentNode) override;
    void unSerialize(boost::property_tree::ptree &node) override;
    std::string getDefaultXmlNodeName() const override;

    static std::shared_ptr<MemoryControlTlv> tlvToMemoryControl(ByteVector tlv);

    static ByteVector memoryControlToTLV(std::shared_ptr<MemoryControlTlv>);

  private:
    void calculateReservedPosition();
    int _byteAddr;
    unsigned char _pageAddr;
    unsigned char _offset;
    unsigned char _size;
    unsigned char _bytesPerPage;
  };

}

#endif

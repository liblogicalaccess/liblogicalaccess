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
  #define TLV_RFU 0x00

  class LLA_CORE_API MemoryControlTlv : public NfcData
  {
  public:
    MemoryControlTlv();
    MemoryControlTlv(ByteVector data);
    MemoryControlTlv(int byteAddr, unsigned char size);
    ~MemoryControlTlv();
    int getByteAddr() const;
    void setByteAddr(int addr);
    unsigned char getPageAddr() const;
    void setPageAddr(unsigned char pageAddr);
    unsigned char getOffset() const;
    void setOffset(unsigned char offset);
    unsigned char getSize() const;
    void setSize(unsigned char size);
    unsigned char getBytesPerPage() const;
    void setBytesPerPage(unsigned char bytesPerPage);


    void serialize(boost::property_tree::ptree &parentNode) override;
    void unSerialize(boost::property_tree::ptree &node) override;
    std::string getDefaultXmlNodeName() const override;

    static std::shared_ptr<MemoryControlTlv> tlvToMemoryControl(ByteVector tlv);

    static ByteVector memoryControlToTLV(std::shared_ptr<MemoryControlTlv> data);

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

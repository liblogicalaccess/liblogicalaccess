/**
 * \file lockcontroltlv.hpp
 * \author Remi W
 * \brief Lock control tlv.
 */

#ifndef LOGICALACCESS_LOCKCONTROLTLV_HPP
#define LOGICALACCESS_LOCKCONTROLTLV_HPP

#include <logicalaccess/services/nfctag/nfcdata.hpp>
#include <cmath>

namespace logicalaccess
{
  class LLA_CORE_API LockControlTlv : public NfcData
  {
  public:
    LockControlTlv();
    LockControlTlv(ByteVector);
    LockControlTlv(int, unsigned char  = 0x10, unsigned char  = 0x04);
    ~LockControlTlv();
    int getAdditionalDataSize() const;
    void setAdditionalDataSize(int);
    unsigned char  getNumberOfDynamicLockBits() const;
    void setNumberOfDynamicLockBits(unsigned char );
    unsigned char  getBytesLockedPerLockBit() const;
    void setBytesLockedPerLockBit(unsigned char );
    unsigned char  getBytesPerPage() const;
    void setBytesPerPage(unsigned char );
    unsigned char  getOffset() const;
    void setOffest(unsigned char );
    unsigned char  getPageAddr() const;
    void setPageAddr(unsigned char );
    unsigned char  getDynamicLockBytesPosition() const;
    void setDynamicLockBytesPosition(unsigned char );

    void serialize(boost::property_tree::ptree &parentNode) override;
    void unSerialize(boost::property_tree::ptree &node) override;
    std::string getDefaultXmlNodeName() const override;

    static std::shared_ptr<LockControlTlv> tlvToLockControl(ByteVector tlv);

    static ByteVector lockControlToTLV(std::shared_ptr<LockControlTlv>);
  private:
    void calculateLockPosition();
    int  _additionalDataSize;
    unsigned char  _numberOfDynamicLockBits;
    unsigned char  _bytesLockedPerLockBit;
    unsigned char  _bytesPerPage;
    unsigned char  _offset;
    unsigned char  _pageAddr;
    unsigned char  _dynamicLockBytesPosition;

  };
}

#endif

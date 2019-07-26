/**
 * \file lockcontroltlv.cpp
 * \author Remi W
 * \brief Lock control tlv.
 */

 #include <logicalaccess/services/nfctag/lockcontroltlv.hpp>

namespace logicalaccess
{
  LockControlTlv::LockControlTlv() :
    NfcData(0x01), _additionalDataSize(0), _numberOfDynamicLockBits(0x10), _bytesLockedPerLockBit(0x04), _bytesPerPage(4), _offset(0), _pageAddr(0), _dynamicLockBytesPosition(0)
  {

  }

  LockControlTlv::LockControlTlv(ByteVector data) :
    NfcData(0x01), _additionalDataSize(0), _numberOfDynamicLockBits(0x10), _bytesLockedPerLockBit(0x04), _bytesPerPage(4), _offset(0), _pageAddr(0), _dynamicLockBytesPosition(0)
  {
    if (data.size() == 5)
    {
      unsigned char  blplb = data[4] << 4;
      blplb = blplb >> 4;
      _dynamicLockBytesPosition = data[2];
      _additionalDataSize = data[2] - 64;
      _numberOfDynamicLockBits = data[3];
      _bytesLockedPerLockBit = blplb;
      this->calculateLockPosition();
    }
  }

  LockControlTlv::LockControlTlv(int ad, unsigned char  nodlb, unsigned char  blplb) :
    NfcData(0x01), _additionalDataSize(ad), _numberOfDynamicLockBits(nodlb), _bytesLockedPerLockBit(blplb), _bytesPerPage(4), _offset(0), _pageAddr(0), _dynamicLockBytesPosition(0)
  {
    this->calculateLockPosition();
  }

  LockControlTlv::~LockControlTlv()
  {

  }

  std::shared_ptr<LockControlTlv> LockControlTlv::tlvToLockControl(ByteVector tlv)
  {
    std::shared_ptr<LockControlTlv>  lockControl = nullptr;

    if (tlv.size() == 5)
    {
      lockControl->setDynamicLockBytesPosition(tlv[2]);
      unsigned char  blplb = tlv[4] << 4;
      blplb = blplb >> 4;
      lockControl = std::make_shared<LockControlTlv>(tlv[2] - 64, tlv[3], blplb);
    }
    return lockControl;
  }

  ByteVector LockControlTlv::lockControlToTLV(std::shared_ptr<LockControlTlv> lockControl)
  {
    ByteVector tlv = {0x01, 0x03};
    unsigned char  byteTmp;

    lockControl->calculateLockPosition();
    byteTmp =  (lockControl->getPageAddr() << 4) + lockControl->getOffset();
    tlv.push_back(byteTmp);
    tlv.push_back(lockControl->getNumberOfDynamicLockBits());
    byteTmp = (lockControl->getBytesPerPage() << 4) + lockControl->getBytesLockedPerLockBit();
    tlv.push_back(byteTmp);
    return tlv;
  }

  void LockControlTlv::calculateLockPosition()
  {
    float a;
    float b;
    int bytesPerPage = 0;
    int offset = 0;
    int byteAddr = _additionalDataSize + 64;

    for (int i = 15; i > 0; i--)
    {
      a = byteAddr / i;
      a = ceil(a);
      b = log2(a);
      bytesPerPage = floor(b);
      offset = byteAddr - i * pow(2, bytesPerPage);
      if (offset >= 0 && offset <= 15)
      {
        _pageAddr = (unsigned char)i;
        _bytesPerPage = (unsigned char)bytesPerPage;
        _offset = (unsigned char)offset;
        return ;
      }
    }
    return ;
  }

  void LockControlTlv::serialize(boost::property_tree::ptree &parentNode)
  {
    /* TO DO */
  }

  void LockControlTlv::unSerialize(boost::property_tree::ptree &node)
  {
    /* TO DO */
  }

  std::string LockControlTlv::getDefaultXmlNodeName() const
  {
      return "LockControlTlv";
  }

  int LockControlTlv::getAdditionalDataSize() const
  {
    return _additionalDataSize;
  }

  void LockControlTlv::setAdditionalDataSize(int i)
  {
    _additionalDataSize = i;
  }

  unsigned char  LockControlTlv::getNumberOfDynamicLockBits() const
  {
    return _numberOfDynamicLockBits;
  }

  void LockControlTlv::setNumberOfDynamicLockBits(unsigned char  c)
  {
    _numberOfDynamicLockBits = c;
  }

  unsigned char  LockControlTlv::getBytesLockedPerLockBit() const
  {
    return _bytesLockedPerLockBit;
  }

  void LockControlTlv::setBytesLockedPerLockBit(unsigned char  c)
  {
    _bytesLockedPerLockBit = c;
  }

  unsigned char  LockControlTlv::getBytesPerPage() const
  {
    return _bytesPerPage;
  }

  void LockControlTlv::setBytesPerPage(unsigned char  c)
  {
    _bytesPerPage = c;
  }

  unsigned char  LockControlTlv::getOffset() const
  {
    return _offset;
  }

  void LockControlTlv::setOffest(unsigned char  c)
  {
    _offset = c;
  }

  unsigned char  LockControlTlv::getPageAddr() const
  {
    return _pageAddr;
  }

  void LockControlTlv::setPageAddr(unsigned char  c)
  {
    _pageAddr = c;
  }

  unsigned char  LockControlTlv::getDynamicLockBytesPosition() const
  {
    return _dynamicLockBytesPosition;
  }

  void LockControlTlv::setDynamicLockBytesPosition(unsigned char  c)
  {
    _dynamicLockBytesPosition = c;
  }
}

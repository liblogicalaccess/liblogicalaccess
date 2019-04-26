/**
 * \file memorycontroltlv.cpp
 * \author Remi W
 * \brief Memory control tlv.
 */

 #include <logicalaccess/services/nfctag/memorycontroltlv.hpp>

namespace logicalaccess
{
  MemoryControlTlv::MemoryControlTlv() :
    NfcData(0x02), _byteAddr(0), _pageAddr(0), _offset(0), _size(0), _bytesPerPage(0)
  {

  }

  MemoryControlTlv::MemoryControlTlv(int byteAddr, unsigned char size) :
    NfcData(0x02), _byteAddr(byteAddr), _pageAddr(0), _offset(0), _size(size), _bytesPerPage(0)
  {
    this->calculateReservedPosition();
  }

  MemoryControlTlv::MemoryControlTlv(ByteVector data) :
    NfcData(0x02), _byteAddr(0), _pageAddr(0), _offset(0), _size(0), _bytesPerPage(0)
  {
    if (data.size() == 5)
    {
      _byteAddr = data[2];
      _size = data[3];
      this->calculateReservedPosition();
    }
  }

  MemoryControlTlv::~MemoryControlTlv()
  {

  }

  std::shared_ptr<MemoryControlTlv> MemoryControlTlv::tlvToMemoryControl(ByteVector tlv)
  {
    std::shared_ptr<MemoryControlTlv> mem = nullptr;

    if (tlv.size() == 5)
    {
      mem = std::make_shared<MemoryControlTlv>(tlv[2], tlv[3]);
    }
    return mem;
  }

  ByteVector MemoryControlTlv::memoryControlToTLV(std::shared_ptr<MemoryControlTlv> data)
  {
    ByteVector tlv {0x02, 0x03};

    data->calculateReservedPosition();
    tlv.push_back((data->getPageAddr() << 4) + data->getOffset());
    tlv.push_back(data->getSize());
    tlv.push_back((data->getBytesPerPage() << 4) + RFU);
    return tlv;
  }

  void MemoryControlTlv::calculateReservedPosition()
  {
    float a;
    float b;
    float bytesPerPage;
    float offset;

    if (_byteAddr <= 64)
    {
      _bytesPerPage = 0x04;
      _pageAddr = _byteAddr >> 4;
      _offset = _byteAddr << 4;
      _offset = _offset >> 4;
      return ;
    }
    for (int i = 15; i >= 0; i--)
    {
      a = ceil(_byteAddr / i);
      b = log2(a);
      bytesPerPage = floor(b);
      offset = _byteAddr - i * pow(2, bytesPerPage);
      if (offset >= 0 && offset <= 15)
      {
        _bytesPerPage = (unsigned char)bytesPerPage;
        _offset = (unsigned char)offset;
        _pageAddr = (unsigned char)i;
        return ;
      }
    }
  }

  void MemoryControlTlv::serialize(boost::property_tree::ptree &parentNode)
  {
    /* TO DO */
  }

  void MemoryControlTlv::unSerialize(boost::property_tree::ptree &node)
  {
    /* TO DO */
  }

  std::string MemoryControlTlv::getDefaultXmlNodeName() const
  {
      return "MemoryControlTlv";
  }

  int MemoryControlTlv::getByteAddr() const
  {
    return _byteAddr;
  }

  void MemoryControlTlv::setByteAddr(int i)
  {
    _byteAddr = i;
  }

  unsigned char MemoryControlTlv::getPageAddr() const
  {
    return _pageAddr;
  }

  void MemoryControlTlv::setPageAddr(unsigned char c)
  {
    _pageAddr = c;
  }

  unsigned char MemoryControlTlv::getOffset() const
  {
    return _offset;
  }

  void MemoryControlTlv::setOffset(unsigned char c)
  {
    _offset = c;
  }

  unsigned char MemoryControlTlv::getSize() const
  {
    return _size;
  }

  void MemoryControlTlv::setSize(unsigned char c)
  {
    _size = c;
  }

  unsigned char MemoryControlTlv::getBytesPerPage() const
  {
    return _bytesPerPage;
  }

  void MemoryControlTlv::setBytesPerPage(unsigned char c)
  {
    _bytesPerPage = c;
  }
}

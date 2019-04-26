/**
 * \file ndefmessage.hpp
 * \author Remi W
 * \brief NFC data.
 */

#ifndef LOGICALACCESS_NFCDATA_HPP
#define LOGICALACCESS_NFCDATA_HPP

#include <logicalaccess/xmlserializable.hpp>
#include <logicalaccess/bufferhelper.hpp>

namespace logicalaccess
{
  class LLA_CORE_API NfcData : public XmlSerializable
  {
  public:
    NfcData();
    NfcData(char type);
    ~NfcData();
    static ByteVector dataToTLV(std::shared_ptr<NfcData> data);
    static std::vector<std::shared_ptr<NfcData> > tlvToData(ByteVector tlv);
    void serialize(boost::property_tree::ptree &parentNode) override;
    void unSerialize(boost::property_tree::ptree &node) override;
    std::string getDefaultXmlNodeName() const override;
    char getType() const;
    void setType(char type);
  protected:
    char m_type;
  };
}

#endif

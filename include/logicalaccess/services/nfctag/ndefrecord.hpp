/**
 * \file nfcrecord.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief NFC Record.
 */

#ifndef LOGICALACCESS_NDEFRECORD_HPP
#define LOGICALACCESS_NDEFRECORD_HPP

#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/xmlserializable.hpp>

namespace logicalaccess
{
enum TNF
{
    TNF_EMPTY         = 0x00,
    TNF_WELL_KNOWN    = 0x01,
    TNF_MIME_MEDIA    = 0x02,
    TNF_ABSOLUTE_URI  = 0x03,
    TNF_EXTERNAL_TYPE = 0x04,
    TNF_UNKNOWN       = 0x05,
    TNF_UNCHANGED     = 0x06,
    TNF_RESERVED      = 0x07
};

class LLA_CORE_API NdefRecord : public XmlSerializable
{
  public:
    NdefRecord()
        : m_tnf(TNF::TNF_EMPTY)
    {
    }
    virtual ~NdefRecord()
    {
    }

    size_t getEncodedSize() const;
    ByteVector encode(bool firstRecord, bool lastRecord);

    void setTnf(TNF tnf)
    {
        m_tnf = tnf;
    }
    TNF getTnf() const
    {
        return m_tnf;
    }

    void setType(ByteVector type)
    {
        m_type = type;
    }
    ByteVector &getType()
    {
        return m_type;
    }

    void setPayload(ByteVector payload)
    {
        m_payload = payload;
    }
    ByteVector &getPayload()
    {
        return m_payload;
    }

    void setId(ByteVector id)
    {
        m_id = id;
    }
    ByteVector &getId()
    {
        return m_id;
    }

    void serialize(boost::property_tree::ptree &parentNode) override;
    void unSerialize(boost::property_tree::ptree &node) override;
    std::string getDefaultXmlNodeName() const override
    {
        return "NdefRecord";
    }

  private:
    unsigned char getTnfByte(bool firstRecord, bool lastRecord) const;

    TNF m_tnf; // 3 bit
    ByteVector m_type;
    ByteVector m_payload;
    ByteVector m_id;
};
}

#endif
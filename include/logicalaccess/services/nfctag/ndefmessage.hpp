/**
 * \file ndefmessage.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief NDEF Message.
 */

#ifndef LOGICALACCESS_NDEFMESSAGE_HPP
#define LOGICALACCESS_NDEFMESSAGE_HPP

#include <logicalaccess/xmlserializable.hpp>
#include <logicalaccess/services/nfctag/ndefrecord.hpp>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{
enum NdefType
{
    Text = 0x54,
    Uri  = 0x55
};

enum UriType
{
    NO_PREFIX = 0x00, // no prefix
    HTTP_WWW  = 0x01, // http://www.
    HTTPS_WWW = 0x02, // https://www.
    HTTP      = 0x03, // http://
    HTTPS     = 0x04, // https://
    TEL       = 0x05, // tel:
    MAIL_TO   = 0x06, // mailto:
    URI_FILE  = 0x1D  // file://
};

class LLA_CORE_API NdefMessage : public XmlSerializable
{
  public:
    NdefMessage()
    {
    }
    explicit NdefMessage(const ByteVector &data);
    virtual ~NdefMessage()
    {
    }

    ByteVector encode();

    void addRecord(std::shared_ptr<NdefRecord> record)
    {
        m_records.push_back(record);
    }
    void addMimeMediaRecord(std::string mimeType, ByteVector payload);
    void addTextRecord(std::string text);
    void addTextRecord(ByteVector text, std::string encoding = "us-ascii");
    void addUriRecord(std::string uri, UriType uritype);
    void addEmptyRecord();

    size_t getRecordCount() const
    {
        return m_records.size();
    }
    std::vector<std::shared_ptr<NdefRecord>> &getRecords()
    {
        return m_records;
    }

    void serialize(boost::property_tree::ptree &parentNode) override;
    void unSerialize(boost::property_tree::ptree &node) override;
    std::string getDefaultXmlNodeName() const override;

    static std::shared_ptr<NdefMessage> TLVToNdefMessage(ByteVector tlv);

    static ByteVector NdefMessageToTLV(std::shared_ptr<NdefMessage> record);

  private:
    std::vector<std::shared_ptr<NdefRecord>> m_records;
};
}

#endif
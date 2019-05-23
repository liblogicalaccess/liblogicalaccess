/**
 * \file ndefmessage.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief NDEF Message.
 */

#ifndef LOGICALACCESS_NDEFMESSAGE_HPP
#define LOGICALACCESS_NDEFMESSAGE_HPP

#include <logicalaccess/services/nfctag/textrecord.hpp>
#include <logicalaccess/services/nfctag/nfcdata.hpp>
#include <logicalaccess/services/nfctag/urirecord.hpp>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{
  #define NDEF_MESSAGE 0x03
enum NdefType
{
    Text = 0x54,
    Uri  = 0x55
};

class LLA_CORE_API NdefMessage : public NfcData
{
  public:
    NdefMessage();
    explicit NdefMessage(const ByteVector &data);
    virtual ~NdefMessage()
    {
    }

    ByteVector encode();

    void addRecord(std::shared_ptr<NdefRecord> record)
    {
        m_records.push_back(record);
    }
    void removeRecord(unsigned int i);
    void addTextRecord(std::string text, std::string language, UTF utf = UTF_8);
    void addEmptyRecord();
    void addUriRecord(std::string uri, UriType uritype);
    void addMimeMediaRecord(std::string mimeType, ByteVector payload);

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

/**
 * \file ndefmessage.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief NDEF Message.
 */

#include <logicalaccess/services/nfctag/ndefmessage.hpp>
#include <logicalaccess/services/nfctag/textrecord.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
  NdefMessage::NdefMessage() : NfcData(NDEF_MESSAGE )
 {
 }

NdefMessage::NdefMessage(const ByteVector &data) : NfcData(NDEF_MESSAGE )
{
    size_t index = 0;

    while (index < data.size())
    {
        EXCEPTION_ASSERT((index + 3) < data.size(), std::invalid_argument,
                         "The buffer size is too small (1).");
        std::shared_ptr<NdefRecord> record(new NdefRecord());

        unsigned char tnf_tmp = data[index];
        //bool mb = (tnf_tmp & 0x80) != 0;
        bool me = (tnf_tmp & 0x40) != 0;
        // bool cf = (tnf_tmp & 0x20) != 0; //We dont manage chunked payload
        bool sr           = (tnf_tmp & 0x10) != 0;
        bool il           = (tnf_tmp & 0x8) != 0;
        unsigned char tnf = (tnf_tmp & 0x7);

        TNF recordTnf = static_cast<TNF>(tnf);
        ByteVector recordType;
        ByteVector recordId;
        ByteVector recordPayload;
        ++index;

        int typeLength = data[index];

        int payloadLength = 0;
        ++index;

        if (sr)
            payloadLength = data[index];
        else
            payloadLength = BufferHelper::getInt32(data, index);
        int idLength = 0;
        if (il)
        {
            index++;
            EXCEPTION_ASSERT(index < data.size(), std::invalid_argument,
                             "The buffer size is too small (2).");
            idLength = data[index];
        }

        ++index;
        EXCEPTION_ASSERT((index + typeLength) <= data.size(), std::invalid_argument,
                         "The buffer size is too small (3).");
        recordType = ByteVector(data.begin() + index, data.begin() + index + typeLength);
        index += typeLength;
        if (il)
        {
            EXCEPTION_ASSERT((index + idLength) <= data.size(), std::invalid_argument,
                             "The buffer size is too small (4).");
            recordId =  ByteVector(data.begin() + index, data.begin() + index + idLength);
            index += idLength;
        }
        EXCEPTION_ASSERT((index + payloadLength) <= data.size(), std::invalid_argument,
                         "The buffer size is too small (5).");
        recordPayload = ByteVector(data.begin() + index, data.begin() + index + payloadLength);
        index += payloadLength;
        if (recordType.size() != 0 && recordType == ByteVector({0x54}))
        {
          std::shared_ptr<TextRecord> tr = std::make_shared<TextRecord>();
          tr->init(recordTnf, recordType, recordId, recordPayload);
          record = tr;
        }
        else if (recordType.size() != 0 && recordType == ByteVector({0x55}))
        {
          std::shared_ptr<UriRecord> ur = std::make_shared<UriRecord>();
          ur->init(recordTnf, recordType, recordId, recordPayload);
          record = ur;
        }
        else
          record->init(recordTnf, recordType, recordId, recordPayload);
        m_records.push_back(record);

        if (me)
            break; // last message
    }
}


void NdefMessage::addMimeMediaRecord(std::string mimeType, ByteVector payload)
{
    std::shared_ptr<NdefRecord> ndefr(new NdefRecord());
    ndefr->setTnf(TNF_MIME_MEDIA);

    ByteVector mimeTypeVec(mimeType.begin(), mimeType.end());

    ndefr->setType(mimeTypeVec);
    ndefr->setPayload(payload);

    m_records.push_back(ndefr);
}

void NdefMessage::removeRecord(unsigned int i)
{
  m_records.erase(m_records.begin() + i);
}

void NdefMessage::addTextRecord(std::string text, std::string language, UTF utf)
{
  std::shared_ptr<TextRecord> ndefr = std::make_shared<TextRecord>();
  unsigned char firstByte = static_cast<int>(utf);
  ByteVector payload;

  if (firstByte != 0)
    firstByte = firstByte << 7;
  firstByte += static_cast<unsigned char>(language.size());
  payload.push_back(firstByte);
  payload.insert(payload.end(), language.begin(), language.end());
  payload.insert(payload.end(), text.begin(), text.end());
  ndefr->init(TNF_WELL_KNOWN, ByteVector({0x54}), ByteVector(), payload);;
  m_records.push_back(ndefr);
}

void NdefMessage::addUriRecord(std::string uri, UriType uritype)
{
    std::shared_ptr<UriRecord> ndefr = std::make_shared<UriRecord>();
    ByteVector payload;

    payload.push_back(static_cast<unsigned char>(uritype));
    payload.insert(payload.end(), uri.begin(), uri.end());

    ndefr->init(TNF_WELL_KNOWN, ByteVector({0x55}), ByteVector(), payload);
    m_records.push_back(ndefr);
}


void NdefMessage::addEmptyRecord()
{
    std::shared_ptr<NdefRecord> ndefr = std::make_shared<NdefRecord>();
    ndefr->setTnf(TNF_EMPTY);
    m_records.push_back(ndefr);
}
ByteVector NdefMessage::encode()
{
    ByteVector data;

    for (std::vector<std::shared_ptr<NdefRecord>>::iterator it = m_records.begin();
         it != m_records.end(); ++it)
    {
        ByteVector record =
            (*it)->encode((it == m_records.begin()), (next(it) == m_records.end()));
        data.insert(data.end(), record.begin(), record.end());
    }
    return data;
}

void NdefMessage::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    boost::property_tree::ptree fnode;
    for (std::vector<std::shared_ptr<NdefRecord>>::const_iterator i = m_records.cbegin();
         i != m_records.cend(); ++i)
    {
        (*i)->serialize(fnode);
    }
    node.add_child("Fields", fnode);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void NdefMessage::unSerialize(boost::property_tree::ptree &node)
{
    m_records.clear();
    BOOST_FOREACH (boost::property_tree::ptree::value_type const &v,
                   node.get_child("Fields"))
    {
        std::shared_ptr<NdefRecord> record(new NdefRecord());
        if (record)
        {
            boost::property_tree::ptree f = v.second;
            record->unSerialize(f);
            m_records.push_back(record);
        }
    }
}

std::string NdefMessage::getDefaultXmlNodeName() const
{
    return "NdefMessage";
}

std::shared_ptr<NdefMessage> NdefMessage::TLVToNdefMessage(ByteVector tlv)
{
    std::shared_ptr<NdefMessage> ndef;
    unsigned short i = 0;
    while (i + 1u < tlv.size())
    {
        switch (tlv[i++])
        {
        case 0x00: // Null
            break;
        case 0x01: // Lock
        case 0x02: // Memory control
        case 0xFD: // Proprietary
            i += tlv[i];
            break;
        case 0x03: // Ndef message
            if (tlv.size() >= i + 1u + tlv[i])
            {
                ByteVector msgdata(tlv.begin() + i + 1, tlv.begin() + i + 1 + tlv[i]);
                ndef.reset(new NdefMessage(msgdata));
                i += tlv[i];
            }

            // Ndef found, leave
            i = static_cast<unsigned short>(tlv.size());
            break;
        case 0xFE: // Terminator
            // Just leave
            i = static_cast<unsigned short>(tlv.size());
            break;
        default:;
        }

        i += 1;
    }
    return ndef;
}

ByteVector NdefMessage::NdefMessageToTLV(std::shared_ptr<NdefMessage> message)
{
    ByteVector data;
    data.push_back(0x03); // T = NDEF
    ByteVector recordsData = message->encode();
    data.push_back(static_cast<unsigned char>(recordsData.size()));
    data.insert(data.end(), recordsData.begin(), recordsData.end());
    data.push_back(0xFE); // T = Terminator
    return data;
}
}

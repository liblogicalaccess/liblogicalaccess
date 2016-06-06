/**
 * \file ndefmessage.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief NDEF Message.
 */

#include "logicalaccess/services/nfctag/ndefmessage.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/logs.hpp>

namespace logicalaccess
{
    NdefMessage::NdefMessage(const std::vector<unsigned char>& data)
    {
        size_t index = 0;

        while (index < data.size())
        {
            EXCEPTION_ASSERT((index + 3) < data.size(), std::invalid_argument, "The buffer size is too small.");
            std::shared_ptr<NdefRecord> record(new NdefRecord());

            unsigned char tnf_tmp = data[index];
            //bool mb = (tnf_tmp & 0x80) != 0;
            bool me = (tnf_tmp & 0x40) != 0;
            //bool cf = (tnf_tmp & 0x20) != 0; //We dont manage chunked payload
            bool sr = (tnf_tmp & 0x10) != 0;
            bool il = (tnf_tmp & 0x8) != 0;
            unsigned char tnf = (tnf_tmp & 0x7);
            ++index;

            record->setTnf(static_cast<TNF>(tnf));

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
                EXCEPTION_ASSERT(index < data.size(), std::invalid_argument, "The buffer size is too small.");
                idLength = data[index];
            }

            ++index;
            EXCEPTION_ASSERT((index + typeLength) <= data.size(), std::invalid_argument, "The buffer size is too small.");
            record->setType(std::vector<unsigned char>(data.begin() + index, data.begin() + index + typeLength));
            index += typeLength;

            if (il)
            {
                EXCEPTION_ASSERT((index + idLength) <= data.size(), std::invalid_argument, "The buffer size is too small.");
                record->setId(std::vector<unsigned char>(data.begin() + index, data.begin() + index + idLength));
                index += idLength;
            }

            EXCEPTION_ASSERT((index + payloadLength) <= data.size(), std::invalid_argument, "The buffer size is too small.");
            record->setPayload(std::vector<unsigned char>(data.begin() + index, data.begin() + index + payloadLength));
            index += payloadLength;

            m_records.push_back(record);

            if (me)
                break; // last message
        }
    }

    void NdefMessage::addMimeMediaRecord(std::string mimeType, std::vector<unsigned char> payload)
    {
        std::shared_ptr<NdefRecord> ndefr(new NdefRecord());
        ndefr->setTnf(TNF_MIME_MEDIA);

        std::vector<unsigned char> mimeTypeVec(mimeType.begin(), mimeType.end());

        ndefr->setType(mimeTypeVec);
		ndefr->setPayload(payload);

        m_records.push_back(ndefr);
    }

	void NdefMessage::addTextRecord(std::string text)
	{
		addTextRecord(std::vector<unsigned char>(text.begin(), text.end()));
	}

    void NdefMessage::addTextRecord(std::vector<unsigned char> text, std::string encoding)
    {
        std::shared_ptr<NdefRecord> ndefr(new NdefRecord());
        ndefr->setTnf(TNF_WELL_KNOWN);
        ndefr->setType(std::vector<unsigned char>(1, NdefType::Text));

        std::vector<unsigned char> payload;
        payload.push_back(static_cast<unsigned char>(encoding.length()));
        payload.insert(payload.end(), encoding.begin(), encoding.end());
        payload.insert(payload.end(), text.begin(), text.end());

        ndefr->setPayload(payload);

        m_records.push_back(ndefr);
    }

    void NdefMessage::addUriRecord(std::string uri, UriType uritype)
    {
        std::shared_ptr<NdefRecord> ndefr(new NdefRecord());
        ndefr->setTnf(TNF_WELL_KNOWN);
        ndefr->setType(std::vector<unsigned char>(1, NdefType::Uri));

        std::vector<unsigned char> payload;
        payload.push_back(static_cast<unsigned char>(uritype));
        payload.insert(payload.end(), uri.begin(), uri.end());

        ndefr->setPayload(payload);

        m_records.push_back(ndefr);
    }

    void NdefMessage::addEmptyRecord()
    {
        std::shared_ptr<NdefRecord> ndefr(new NdefRecord());
        ndefr->setTnf(TNF_EMPTY);
        m_records.push_back(ndefr);
    }

    std::vector<unsigned char> NdefMessage::encode()
    {
        std::vector<unsigned char> data;

        for (std::vector<std::shared_ptr<NdefRecord> >::iterator it = m_records.begin(); it != m_records.end(); ++it)
        {
            std::vector<unsigned char> record = (*it)->encode((it == m_records.begin()), (std::next(it) == m_records.end()));
            data.insert(data.end(), record.begin(), record.end());
        }
        return data;
    }

    void NdefMessage::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        boost::property_tree::ptree fnode;
        for (std::vector<std::shared_ptr<NdefRecord> >::const_iterator i = m_records.cbegin(); i != m_records.cend(); ++i)
        {
            (*i)->serialize(fnode);
        }
        node.add_child("Fields", fnode);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void NdefMessage::unSerialize(boost::property_tree::ptree& node)
    {
        m_records.clear();
        BOOST_FOREACH(boost::property_tree::ptree::value_type const& v, node.get_child("Fields"))
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
}
/**
 * \file ndefmessage.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief NDEF Message.
 */

#include "logicalaccess/services/nfctag/ndefmessage.hpp"
#include "logicalaccess/bufferhelper.hpp"


namespace logicalaccess
{
	NdefMessage::NdefMessage(std::vector<unsigned char>& data)
	{
		unsigned int index = 0;

		while (index < data.size())
		{
			NdefRecord record;

			unsigned char tnf_tmp = data[index];
			bool mb = (tnf_tmp & 0x80) != 0;
			bool me = (tnf_tmp & 0x40) != 0;
			bool cf = (tnf_tmp & 0x20) != 0; //We dont manage chunked payload
			bool sr = (tnf_tmp & 0x10) != 0;
			bool il = (tnf_tmp & 0x8) != 0;
			unsigned char tnf = (tnf_tmp & 0x7);
			++index;

			record.setTnf(tnf);

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
				idLength = data[index];
			}

			++index;
			record.setType(std::vector<unsigned char>(data.begin() + index, data.begin() + index + typeLength));
			index += typeLength;

			if (il)
			{
				record.setId(std::vector<unsigned char>(data.begin() + index, data.begin() + index + idLength));
				index += idLength;
			}

			record.setPayload(std::vector<unsigned char>(data.begin() + index, data.begin() + index + payloadLength));
			index += payloadLength;

			m_records.push_back(record);

			if (me)
				break; // last message
		}
	}

	void NdefMessage::addMimeMediaRecord(std::string mimeType, std::string payload)
	{
		NdefRecord ndefr = NdefRecord();
		ndefr.setTnf(TNF_MIME_MEDIA);

		std::vector<unsigned char> mimeTypeVec(mimeType.begin(), mimeType.end()); 
		std::vector<unsigned char> payloadVec(payload.begin(), payload.end());

		ndefr.setType(mimeTypeVec);
		ndefr.setPayload(payloadVec);

		m_records.push_back(ndefr);
	}

	void NdefMessage::addTextRecord(std::string text)
	{
		addTextRecord(text, "en");
	}

	void NdefMessage::addTextRecord(std::string text, std::string encoding)
	{
		NdefRecord ndefr;
		ndefr.setTnf(TNF_WELL_KNOWN);
		ndefr.setType(std::vector<unsigned char>(1, 0x54)); //Text Constant

		std::vector<unsigned char> payload;
		payload.push_back(static_cast<unsigned char>(encoding.length()));
		payload.insert(payload.begin(), encoding.begin(), encoding.end());
		payload.insert(payload.begin(), text.begin(), text.end());

		ndefr.setPayload(payload);

		m_records.push_back(ndefr);
	}

	void NdefMessage::addUriRecord(std::string uri)
	{
		NdefRecord ndefr;
		ndefr.setTnf(TNF_WELL_KNOWN);
		ndefr.setType(std::vector<unsigned char>(1, 0x54)); //URI Constant

		std::vector<unsigned char> payload;
		payload.push_back(0x00);
		payload.insert(payload.begin(), uri.begin(), uri.end());

		ndefr.setPayload(payload);

		m_records.push_back(ndefr);
	}

	void NdefMessage::addEmptyRecord()
	{
		NdefRecord ndefr;
		ndefr.setTnf(TNF_EMPTY);
		m_records.push_back(ndefr);
	}
}


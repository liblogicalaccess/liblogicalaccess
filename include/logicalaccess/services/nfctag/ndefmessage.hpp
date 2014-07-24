/**
 * \file ndefmessage.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief NDEF Message.
 */

#ifndef LOGICALACCESS_NDEFMESSAGE_HPP
#define LOGICALACCESS_NDEFMESSAGE_HPP

#include "logicalaccess/services/nfctag/ndefrecord.hpp"

namespace logicalaccess
{
	enum TNF {
		TNF_EMPTY = 0x00,
		TNF_WELL_KNOWN = 0x01,
		TNF_MIME_MEDIA = 0x02,
		TNF_ABSOLUTE_URI = 0x03,
		TNF_EXTERNAL_TYPE = 0x04,
		TNF_UNKNOWN = 0x05,
		TNF_UNCHANGED = 0x06,
		TNF_RESERVED = 0x07
	};

	class LIBLOGICALACCESS_API NdefMessage
	{
	public:
		NdefMessage() {};
		NdefMessage(std::vector<unsigned char>& data);
		~NdefMessage() {};

        unsigned int getEncodedSize(); // need so we can pass array to encode
        std::vector<unsigned char> encode();

		void addRecord(NdefRecord& record) { m_records.push_back(record); };
        void addMimeMediaRecord(std::string mimeType, std::string payload);
        void addTextRecord(std::string text);
        void addTextRecord(std::string text, std::string encoding);
        void addUriRecord(std::string uri);
        void addEmptyRecord();

		size_t getRecordCount() { return m_records.size(); };

    private:
        std::vector<NdefRecord> m_records;
	};
}

#endif
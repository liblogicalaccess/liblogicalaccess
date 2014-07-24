/**
 * \file nfcrecord.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief NFC Record.
 */

#ifndef LOGICALACCESS_NDEFRECORD_HPP
#define LOGICALACCESS_NDEFRECORD_HPP

#include "logicalaccess/services/cardservice.hpp"

namespace logicalaccess
{
	class LIBLOGICALACCESS_API NdefRecord
	{
	public:
		NdefRecord() {};
		~NdefRecord() {};

        unsigned int getEncodedSize();
        std::vector<unsigned char> encode(bool firstRecord, bool lastRecord);

		void setTnf(unsigned char tnf) { m_tnf = tnf; };
		unsigned char getTnf() { return m_tnf; };

        void setType(std::vector<unsigned char> type) { m_type = type; };
		std::vector<unsigned char> getType() { return m_type; };

        void setPayload(std::vector<unsigned char> payload) { m_payload = payload; };
		std::vector<unsigned char> getPayload() { return m_payload; };

        void setId(std::vector<unsigned char> id) { m_id = id; };
		std::vector<unsigned char> getId() { return m_id; };

    private:
        unsigned char getTnfByte(bool firstRecord, bool lastRecord);
        unsigned char m_tnf; // 3 bit
        std::vector<unsigned char> m_type;
        std::vector<unsigned char> m_payload;
        std::vector<unsigned char> m_id;
	};
}

#endif
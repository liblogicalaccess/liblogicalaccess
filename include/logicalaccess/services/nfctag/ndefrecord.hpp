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

	class LIBLOGICALACCESS_API NdefRecord : public XmlSerializable
	{
	public:
		NdefRecord() : m_tnf(TNF::TNF_EMPTY) {};
		~NdefRecord() {};

        size_t getEncodedSize() const;
        std::vector<unsigned char> encode(bool firstRecord, bool lastRecord);

		void setTnf(TNF tnf) { m_tnf = tnf; };
		TNF getTnf() const { return m_tnf; };

        void setType(std::vector<unsigned char> type) { m_type = type; };
		std::vector<unsigned char>& getType() { return m_type; };

        void setPayload(std::vector<unsigned char> payload) { m_payload = payload; };
		std::vector<unsigned char>& getPayload() { return m_payload; };

        void setId(std::vector<unsigned char> id) { m_id = id; };
		std::vector<unsigned char>& getId() { return m_id; };


		virtual void serialize(boost::property_tree::ptree& parentNode);
		virtual void unSerialize(boost::property_tree::ptree& node);
		virtual std::string getDefaultXmlNodeName() const { return "NdefRecord"; };

    private:
        unsigned char getTnfByte(bool firstRecord, bool lastRecord) const;

        TNF m_tnf; // 3 bit
        std::vector<unsigned char> m_type;
        std::vector<unsigned char> m_payload;
        std::vector<unsigned char> m_id;
	};
}

#endif
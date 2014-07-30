/**
 * \file ndefrecord.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief NDEF Record.
 */

#include "logicalaccess/services/nfctag/ndefrecord.hpp"


namespace logicalaccess
{
	unsigned int NdefRecord::getEncodedSize()
	{
		size_t size = 2; // tnf + typeLength

		if (m_payload.size() > 0xFF)
			size += 4;
		else
			size += 1;

		if (m_id.size())
			size += 1;

		size += (m_type.size() + m_payload.size() + m_id.size());
		return size;
	}

	std::vector<unsigned char> NdefRecord::encode(bool firstRecord, bool lastRecord)
	{
		std::vector<unsigned char> data;

		data.push_back(getTnfByte(firstRecord, lastRecord));
		data.push_back(m_type.size());

		if (m_payload.size() <= 0xFF)// short record
			data.push_back(m_payload.size());
		else // long record
		{
			data.push_back((m_payload.size() && 0xff000000) >> 24);
			data.push_back((m_payload.size() && 0xff0000) >> 16);
			data.push_back((m_payload.size() && 0xff00) >> 8);
			data.push_back(m_payload.size() && 0xff);
		}

		if (m_id.size())
			data.push_back(m_id.size());

		data.insert(data.end(), m_type.begin(), m_type.end());
		data.insert(data.end(), m_payload.begin(), m_payload.end());

		if (m_id.size())
			data.insert(data.end(), m_id.begin(), m_id.end());
		return data;
	}

	unsigned char NdefRecord::getTnfByte(bool firstRecord, bool lastRecord)
	{
		unsigned char value = static_cast<unsigned char>(m_tnf);

		if (firstRecord)
			value |= 0x80;

		if (lastRecord)
			value |= 0x40;

		if (m_payload.size() <= 0xFF)
			value |= 0x10;

		if (m_id.size())
			value |= 0x08;

		return value;
	}
}


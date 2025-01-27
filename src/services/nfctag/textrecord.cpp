/**
 * \file ndefrecord.cpp
 * \author Remi W
 * \brief text Record.
 */

#include <logicalaccess/services/nfctag/textrecord.hpp>

namespace logicalaccess
{
  void TextRecord::init(TNF tnf, ByteVector type, ByteVector id, ByteVector payload)
  {
    unsigned int size;

    m_tnf = tnf;
    m_type = type;
    m_payload = payload;
    m_id = id;
    m_utf = static_cast<UTF>(payload[0] & 0x80);
    size = static_cast<UTF>(payload[0] & 0x3F);
    m_language = std::string(payload.begin() + 1, payload.begin() + 1 + size);
    m_cleanPayload = std::string(payload.begin() + 1 + size, payload.end());
  }

  void TextRecord::updatePayload()
  {
    unsigned char firstByte = static_cast<int>(m_utf);

    firstByte = firstByte << 7;
    firstByte += static_cast<unsigned char>(m_language.size());
    m_payload.clear();
    m_payload.push_back(firstByte);
    m_payload.insert(m_payload.end(), m_language.begin(), m_language.end());
    m_payload.insert(m_payload.end(), m_cleanPayload.begin(), m_cleanPayload.end());
  }

  void TextRecord::setLanguage(std::string language, UTF utf)
  {
    m_language = language;
    m_utf = utf;
    updatePayload();
  }

  std::string TextRecord::getLanguage() const
  {
    return m_language;
  }

  void TextRecord::setCleanPayload(std::string cleanPayload)
  {
    m_cleanPayload = cleanPayload;
    updatePayload();
  }

  std::string TextRecord::getCleanPayload() const
  {
    return m_cleanPayload;
  }

  void TextRecord::setUTF(UTF utf)
  {
    m_utf = utf;
    updatePayload();
  }

  UTF TextRecord::getUTF() const
  {
    return m_utf;
  }
}

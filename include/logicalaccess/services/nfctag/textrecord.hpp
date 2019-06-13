/**
 * \file textrecord.hpp
 * \author Remi W
 * \brief Text Record.
 */

#ifndef LOGICALACCESS_TEXTRECORD_HPP
#define LOGICALACCESS_TEXTRECORD_HPP

#include <logicalaccess/services/nfctag/ndefrecord.hpp>

namespace logicalaccess
{
  enum UTF
  {
      UTF_8        = 0x00,
      UTF_16       = 0x01
  };

class LLA_CORE_API TextRecord : public NdefRecord
{
  public:
    TextRecord()
        : NdefRecord()
    {
    }
    virtual ~TextRecord()
    {
    }

    void init(TNF tnf, ByteVector type, ByteVector id, ByteVector payload) override;
    void setLanguage(std::string language, UTF utf = UTF_8);
    std::string getLanguage() const;
    void setCleanPayload(std::string cleanPayload);
    std::string getCleanPayload() const;
    void setUTF(UTF utf);
    UTF getUTF() const;
    void updatePayload();
  private:
    std::string m_language;
    std::string m_cleanPayload;
    UTF m_utf;
};
}

#endif

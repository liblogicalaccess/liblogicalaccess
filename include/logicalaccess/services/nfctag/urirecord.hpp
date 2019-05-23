/**
 * \file urirecord.hpp
 * \author Remi W
 * \brief URI Record.
 */

#ifndef LOGICALACCESS_URIRECORD_HPP
#define LOGICALACCESS_URIRECORD_HPP

#include <logicalaccess/services/nfctag/ndefrecord.hpp>
#include <unordered_map>

namespace logicalaccess
{
  enum UriType
  {
      NO_PREFIX = 0x00, // no prefix
      HTTP_WWW  = 0x01, // http://www.
      HTTPS_WWW = 0x02, // https://www.
      HTTP      = 0x03, // http://
      HTTPS     = 0x04, // https://
      TEL       = 0x05, // tel:
      MAIL_TO   = 0x06, // mailto:
      URI_FILE  = 0x1D  // file://
  };
class LLA_CORE_API UriRecord : public NdefRecord
{
  public:
    UriRecord()
        : NdefRecord()
    {
      m_prefixeMap[NO_PREFIX] = "";
      m_prefixeMap[HTTP_WWW] = "http://www.";
      m_prefixeMap[HTTPS_WWW] = "https://www.";
      m_prefixeMap[HTTP] = "http://";
      m_prefixeMap[HTTPS] = "https://";
      m_prefixeMap[TEL] = "tel:";
      m_prefixeMap[MAIL_TO] = "mailto:";
      m_prefixeMap[URI_FILE] = "file://";
    }

    virtual ~UriRecord()
    {
    }

    void init(TNF tnf, ByteVector type, ByteVector id, ByteVector payload) override;
    void updatePayload();
    void setUri(std::string uri);
    std::string getUri();
    void setPrefixe(UriType prefixe);
    UriType getPrefixe() const;
  private:
    std::string m_uri;
    UriType m_prefixe;
    std::unordered_map<UriType, std::string> m_prefixeMap;
};
}

#endif

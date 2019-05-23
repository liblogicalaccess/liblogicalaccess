/**
 * \file urirecord.cpp
 * \author Remi W
 * \brief URI Record.
 */

 #include <logicalaccess/services/nfctag/urirecord.hpp>

 namespace logicalaccess
 {
   void UriRecord::init(TNF tnf, ByteVector type, ByteVector id, ByteVector payload)
   {
       m_tnf = tnf;
       m_type = type;
       m_payload = payload;
       m_id = id;
       m_prefixe = static_cast<UriType>(payload[0]);
       m_uri = std::string(payload.begin() + 1, payload.end());
   }

   void UriRecord::setUri(std::string uri)
   {
     m_uri = uri;
    updatePayload();
   }

   std::string UriRecord::getUri()
   {
     std::string completeUri = m_prefixeMap[m_prefixe] + m_uri;
     return completeUri;
   }

   void UriRecord::setPrefixe(UriType prefixe)
   {
     m_prefixe = prefixe;
     updatePayload();
   }

   UriType UriRecord::getPrefixe() const
   {
     return m_prefixe;
   }

   void UriRecord::updatePayload()
   {
     m_payload.clear();
     m_payload.push_back(m_prefixe);
     m_payload.insert(m_payload.end(), m_uri.begin(), m_uri.end());
   }
 }

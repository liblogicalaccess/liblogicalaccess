#pragma once
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <logicalaccess/plugins/crypto/openssl.hpp>
#include <logicalaccess/plugins/crypto/public_key.hpp>
#include <iostream>
#include <memory>

namespace logicalaccess
{
  class LLA_CRYPTO_API X509Certificate
  {
  public:
    X509Certificate();
    X509Certificate(std::string data);
    ~X509Certificate();
    void setData(std::string data);
    std::string getRawData() const;
    std::shared_ptr<PublicKey> getKey();
    int verify(std::shared_ptr<PublicKey> key);
  private:
    void freeAllData();
    X509* _certificate;
    std::string _rawData;
    BIO *_bio;
    std::shared_ptr<PublicKey> _key;
  };
}

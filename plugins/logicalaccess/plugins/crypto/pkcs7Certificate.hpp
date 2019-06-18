#pragma once
#include <openssl/pkcs7.h>
#include <logicalaccess/plugins/crypto/openssl.hpp>
#include <logicalaccess/plugins/crypto/public_key.hpp>
#include <openssl/x509.h>
#include <iostream>
#include <memory>

namespace logicalaccess
{
  class LLA_CRYPTO_API Pkcs7Certificate
  {
  public:
    Pkcs7Certificate();
    Pkcs7Certificate(std::string data);
    ~Pkcs7Certificate();
    void setData(std::string data);
    std::string getRawData() const;
    int verify(std::shared_ptr<PublicKey> key);
    std::string getExpiryDate();
  private:
    PKCS7* _certificate;
    std::string _rawData;
  };
}

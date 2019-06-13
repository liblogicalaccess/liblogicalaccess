#pragma once
#include <logicalaccess/plugins/crypto/openssl.hpp>
#include <openssl/x509.h>
#include <iostream>

namespace logicalaccess
{
  class LLA_CRYPTO_API PublicKey
  {
  public:
    PublicKey();
    PublicKey(EVP_PKEY *key);
    ~PublicKey();
    void setPublicKey(EVP_PKEY *key);
    EVP_PKEY *getPublicKey() const;
  private:
    EVP_PKEY *_key;
  };
}

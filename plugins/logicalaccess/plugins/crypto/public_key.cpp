#include <logicalaccess/plugins/crypto/public_key.hpp>

using namespace logicalaccess;

PublicKey::PublicKey()
{
  _key = nullptr;
}

PublicKey::PublicKey(EVP_PKEY *key)
{
  _key = key;
}


PublicKey::~PublicKey()
{
  if (_key != nullptr)
    EVP_PKEY_free(_key);
}

void PublicKey::setPublicKey(EVP_PKEY *key)
{
  _key = key;
}

EVP_PKEY *PublicKey::getPublicKey() const
{
  return _key;
}

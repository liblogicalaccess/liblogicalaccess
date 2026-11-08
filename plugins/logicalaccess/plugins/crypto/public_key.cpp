#include <logicalaccess/plugins/crypto/public_key.hpp>

using namespace logicalaccess;

PublicKey::PublicKey()
    : _key(nullptr, EVP_PKEY_free)
{
}

PublicKey::PublicKey(EVP_PKEY *key)
    : _key(key, EVP_PKEY_free)
{
}

void PublicKey::setPublicKey(EVP_PKEY *key)
{
    _key.reset(key);
}

EVP_PKEY *PublicKey::getPublicKey() const noexcept
{
    return _key.get();
}

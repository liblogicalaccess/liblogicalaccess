#pragma once

#include <memory>

#include <logicalaccess/plugins/crypto/openssl.hpp>

#include <openssl/evp.h>

namespace logicalaccess
{

  class LLA_CRYPTO_API PublicKey
  {
  public:
    PublicKey();
    /**
     * \brief Takes ownership of the supplied EVP_PKEY
     *
     * The caller must not free key after transferring ownership.
     * The supplied pointer may be nullptr
     */
    explicit PublicKey(EVP_PKEY *key);

    PublicKey(const PublicKey &)            = delete;
    PublicKey &operator=(const PublicKey &) = delete;

    PublicKey(PublicKey &&) noexcept            = default;
    PublicKey &operator=(PublicKey &&) noexcept = default;

    ~PublicKey() = default;

    /**
     * \brief Replace the owned OpenSSL public key
     *
     * Ownership of key is transferred to this object.
     * The supplied pointer may be nullptr
     */
    void setPublicKey(EVP_PKEY *key);

    /**
     * \brief Return the underlying OpenSSL public key
     *
     * The returned pointer remains owned by this object and mustn't be freed by the caller
     */
    EVP_PKEY *getPublicKey() const noexcept;
  private:
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> _key{nullptr, EVP_PKEY_free};
  };

} // namespace logicalaccess

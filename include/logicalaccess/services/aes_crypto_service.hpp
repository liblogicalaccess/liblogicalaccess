#pragma once

#include <logicalaccess/lla_core_api.hpp>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{
/**
 * Stateless service to perform AES cryptography against Key.
 *
 * The goal of this service is to perform an operation that is key-storage
 * agnostic. The service will delegate to either in memory crypto, PKCS, IKS, SAM
 * depending on the KeyStorage defined on the key.
 *
 * Note sure how to handle derivation yet.
 */
class LLA_CORE_API AESCryptoService
{
  public:
    ByteVector aes_encrypt(const ByteVector &data, const ByteVector &iv,
                           std::shared_ptr<Key> key);

    ByteVector aes_decrypt(const ByteVector &data, const ByteVector &iv,
                           std::shared_ptr<Key> key);

  private:
    // Adjust IV. If `iv` is empty vector, return an full zero iv.
    ByteVector adjust_iv(const ByteVector &iv);

    ByteVector perform_operation(const ByteVector &data, const ByteVector &iv,
                                 std::shared_ptr<Key> key, bool encrypt);

    ByteVector in_memory(const ByteVector &data, const ByteVector &iv,
                         std::shared_ptr<Key> key, bool encrypt);

    ByteVector with_pkcs(const ByteVector &data, const ByteVector &iv,
                         std::shared_ptr<Key> key, bool encrypt);
};

/**
 * Interface that should be implemented by plugin that provide cryptographic
 * service.
 *
 * Instance of this class are loaded the AESCryptoService and
 * the plugin manager in order to use cryptographic backend that match
 * the KeyStorage of a given key.
 */
class LLA_CORE_API IAESCryptoService
{
  public:
    virtual ByteVector aes_encrypt(const ByteVector &data, const ByteVector &iv,
                                   std::shared_ptr<Key> key) = 0;

    virtual ByteVector aes_decrypt(const ByteVector &data, const ByteVector &iv,
                                   std::shared_ptr<Key> key) = 0;
};
}

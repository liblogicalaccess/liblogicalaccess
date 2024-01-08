/**
 * \file des_cipher.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DES cipher class.
 */

#ifndef DES_CIPHER_HPP
#define DES_CIPHER_HPP

#include <logicalaccess/plugins/crypto/openssl_symmetric_cipher.hpp>

namespace logicalaccess
{
namespace openssl
{
/**
 * \brief A DES cipher.
 *
 * DESCipher is a symmetric cipher that relies on OpenSSL's implementation of the DES
 * algorithm. Take a look at SymmetricCipher for usage.
 */
class LLA_CRYPTO_API DESCipher : public OpenSSLSymmetricCipher
{
  public:
    /**
     * \brief Constructor.
     * \param _mode The encryption mode. Default is CBC.
     */
    explicit DESCipher(EncMode _mode = ENC_MODE_CBC)
        : OpenSSLSymmetricCipher(_mode)
    {
    }

	unsigned char getBlockSize() const override;

  protected:
    /**
     * \brief Get the openssl EVP cipher.
     * \param key The key.
     * \return The EVP cipher.
     */
    const EVP_CIPHER *getEVPCipher(const SymmetricKey &key) const override;
};
}
}

#endif /* AES_CIPHER_HPP */
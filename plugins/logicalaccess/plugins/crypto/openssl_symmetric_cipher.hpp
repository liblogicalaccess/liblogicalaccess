/**
 * \file openssl_symmetric_cipher.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief OpenSSL symmetric cipher base class.
 */

#ifndef OPENSSL_SYMETRIC_CIPHER_HPP
#define OPENSSL_SYMETRIC_CIPHER_HPP

#include <logicalaccess/plugins/crypto/symmetric_cipher.hpp>
#include <openssl/evp.h>

namespace logicalaccess
{
namespace openssl
{
class OpenSSLSymmetricCipherContext;

/**
 * \brief A OpenSSL symmetric cipher base class.
 *
 * This class serves as a base for all OpenSSL based symmetric cipher classes.
 *
 * To cipher or decipher a buffer, you can use the start() method to get a context, use
 * update() to fill-in a data block and stop() to get the ciphered/deciphered resulting
 * data.
 *
 * OpenSSLSymmetricCipher also provides helper methods cipher() and decipher() to quickly
 * cipher or decipher a buffer.
 */
class LIBLOGICALACCESS_API OpenSSLSymmetricCipher : public SymmetricCipher
{
  public:
    /**
     * \brief Encryption mode.
     */
    enum EncMode
    {
        ENC_MODE_CBC,  /**< \brief CBC. */
        ENC_MODE_CFB,  /**< \brief CFB. */
        ENC_MODE_CFB1, /**< \brief CFB1. */
        ENC_MODE_CFB8, /**< \brief CFB8. */
        ENC_MODE_ECB,  /**< \brief ECB. */
        ENC_MODE_OFB   /**< \brief OFB. */
    };

    /**
     * \brief A method.
     */
    enum Method
    {
        M_ENCRYPT,
        M_DECRYPT
    };

    /**
     * \brief Constructor.
     * \param mode The encryption mode.
     */
    explicit OpenSSLSymmetricCipher(EncMode mode);

    virtual ~OpenSSLSymmetricCipher();

    /**
     * \brief Initialize a new encryption/decryption session.
     * \param method The method.
     * \param key The key to use.
     * \param iv The initialisation vector.
     * \param padding Whether to use padding.
     * \return A context that must be passed to stop() when cipherement is done.
     */
    OpenSSLSymmetricCipherContext start(Method method, const SymmetricKey &key,
                                        const InitializationVector &iv,
                                        bool padding) const;

    /**
     * \brief Add data to encrypt/decrypt.
     * \param context The context.
     * \param src The data to add to the encrypt/decrypt session.
     */
    static void update(OpenSSLSymmetricCipherContext &context, const ByteVector &src);

    /**
     * \brief Finalize an encryption/decryption session.
     * \param context The context.
     * \return The encrypted/decrypted data.
     * \warning The value context is undefined after the call. Passing it to start(),
     * update() or stop() will lead to undefined behavior.
     */
    static ByteVector stop(OpenSSLSymmetricCipherContext &context);

    /**
     * \brief Cipher a buffer.
     * \param src The buffer to cipher.
     * \param dest The ciphered buffer.
     * \param key The key to use.
     * \param iv The initialisation vector.
     * \param padding Whether to use padding.
     * In case of a failure, the call
     an InvalidCallException.
     */
    void cipher(const ByteVector &src, ByteVector &dest, const SymmetricKey &key,
                const InitializationVector &iv, bool padding) override;

    /**
     * \brief Decipher a buffer.
     * \param src The buffer to decipher.
     * \param dest The deciphered buffer.
     * \param key The key to use.
     * \param iv The initialisation vector.
     * \param padding Whether to use padding.
     * In case of a failure, the call
     a InvalidCallException.
     */
    void decipher(const ByteVector &src, ByteVector &dest, const SymmetricKey &key,
                  const InitializationVector &iv, bool padding) override;

  protected:
    /**
     * \brief Get the openssl EVP cipher.
     * \param key The key.
     * \return The EVP cipher.
     */
    virtual const EVP_CIPHER *getEVPCipher(const SymmetricKey &key) const = 0;

    /**
     * \brief Get the encryption mode.
     * \return The encryption mode.
     */
    EncMode mode() const
    {
        return d_mode;
    }

  private:
    /**
     * \brief The encryption mode.
     */
    EncMode d_mode;
};
}
}

#endif /* OPENSSL_SYMETRIC_CIPHER_HPP */
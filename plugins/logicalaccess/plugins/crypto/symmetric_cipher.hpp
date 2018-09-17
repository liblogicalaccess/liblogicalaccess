/**
 * \file symmetric_cipher.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Symmetric cipher base class.
 */

#ifndef SYMETRICCIPHER_HPP
#define SYMETRICCIPHER_HPP

#include <logicalaccess/plugins/crypto/initialization_vector.hpp>

namespace logicalaccess
{
class InitializationVector;

namespace openssl
{
class SymmetricKey;

/**
 * \brief A symmetric cipher.
 */
class LIBLOGICALACCESS_API SymmetricCipher
{
  public:
    /**
     * \brief Destructor.
     *
     * This is required for the compiler to properly generate RTTI information.
     * Do not remove, and do not inline.
     */
    virtual ~SymmetricCipher();

    /**
     * \brief Cipher a buffer.
     * \param src The buffer to cipher.
     * \param dest The ciphered buffer.
     * \param key The key to use.
     * \param iv The initialisation vector.
     * \param padding Whether to use padding.
     * In case of a failure, the call throw an InvalidCallException.
     */
    virtual void cipher(const ByteVector &src, ByteVector &dest, const SymmetricKey &key,
                        const InitializationVector &iv, bool padding) = 0;

    /**
     * \brief Decipher a buffer.
     * \param src The buffer to decipher.
     * \param dest The deciphered buffer.
     * \param key The key to use.
     * \param iv The initialisation vector.
     * \param padding Whether to use padding.
     * In case of a failure, the call throw a InvalidCallException.
     */
    virtual void decipher(const ByteVector &src, ByteVector &dest,
                          const SymmetricKey &key, const InitializationVector &iv,
                          bool padding) = 0;
};
}
}

#endif /* SYMETRICCIPHER_HPP */
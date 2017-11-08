/**
 * \file asymmetric_cipher.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief The base class for all asymmetric ciphers.
 */

#ifndef ASYMETRICCIPHER_HPP
#define ASYMETRICCIPHER_HPP

#include <logicalaccess/plugins/crypto/asymmetric_key.hpp>

#include <vector>
#include <iostream>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{
namespace openssl
{
/**
 * \brief The asymmetric cipher base class.
 *
 * AsymmetricCipher is the base class for all asymmetric ciphers. It provides two methods,
 * cipher() and decipher() which respectively cipher and decipher a specified buffer and
 * which must be implementend in every subclass.
 */
class AsymmetricCipher
{
  public:
    /**
     * \brief Destructor.
     */
    virtual ~AsymmetricCipher()
    {
    }

    /**
     * \brief A list of possible encryption keys compounds to use.
     */
    enum KeyCompound
    {
        KC_PUBLIC, /**< \brief The public key compound. */
        KC_PRIVATE /**< \brief The private key compound. */
    };

    /**
     * \brief Cipher a buffer.
     * \param src The buffer to cipher.
     * \param dest The ciphered buffer.
     * \param key The key to use.
     * \param key_compound The key compound to use.
     * In case of a failure, the call throw an InvalidCallException.
     */
    virtual void cipher(const ByteVector &src, ByteVector &dest, const AsymmetricKey &key,
                        KeyCompound key_compound) = 0;

    /**
     * \brief Decipher a buffer.
     * \param src The buffer to decipher.
     * \param dest The deciphered buffer.
     * \param key The key to use.
     * \param key_compound The key compound to use.
     * In case of a failure, the call throw a InvalidCallException.
     */
    virtual void decipher(const ByteVector &src, ByteVector &dest,
                          const AsymmetricKey &key, KeyCompound key_compound) = 0;
};
}
}

#endif /* ASYMETRICCIPHER_HPP */
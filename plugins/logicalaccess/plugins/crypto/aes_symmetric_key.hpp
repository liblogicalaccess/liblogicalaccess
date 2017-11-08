/**
 * \file aes_symmetric_key.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief AES symmetric key class.
 */

#ifndef AES_SYMETRICKEY_HPP
#define AES_SYMETRICKEY_HPP

#include <logicalaccess/plugins/crypto/symmetric_key.hpp>

#include <string>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{
namespace openssl
{
/**
 * \brief An AES symmetric key.
 *
 * AESSymmetricKey represents a 16, 24 or 32 bytes long AES key to use with an instance
 * AESCipher.
 *
 * You can create an AESSymmetricKey from random values, using createRandom() or from
 * specific data, using createFromData(). You can also generate an AESSymmetricKey from a
 * passphrase, using createFromPassphrase().
 */
class AESSymmetricKey : public SymmetricKey
{
  public:
    /**
     * \brief Create a random symmetric key of a given size.
     * \param size The size of the key. Must be 16, 24 or 32.
     * \return The symmetric key.
     */
    static AESSymmetricKey createRandom(size_t size);

    /**
     * \brief Create a symmetric key from the specified data.
     * \param data The data.
     * \return The symmetric key.
     * \warning If data is not 16, 24 or 32 bytes long, the behavior is undefined.
     */
    static AESSymmetricKey createFromData(const ByteVector &data);

    /**
     * \brief Create a 32 bytes long symmetric key from the specified passphrase.
     * \param passphrase The passphrase.
     * \return The symmetric key.
     */
    static AESSymmetricKey createFromPassphrase(const std::string &passphrase);

  protected:
    /**
     * \brief Create a new random AESSymmetricKey.
     * \param size The size of the key. Must be 16, 24 or 32.
     */
    explicit AESSymmetricKey(size_t size);

    /**
     * \brief Create an AESInitializationVector from existing data.
     * \param data The data.
     * \warning If data is not 16, 24 or 32 bytes long, the behavior is undefined.
     */
    explicit AESSymmetricKey(const ByteVector &data);
};
}
}

#endif /* AES_SYMETRICKEY_HPP */
/**
 * \file aes_initialization_vector.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AES initialization vector class.
 */

#ifndef AES_INITIALIZATION_VECTOR_HPP
#define AES_INITIALIZATION_VECTOR_HPP

#include <logicalaccess/plugins/crypto/initialization_vector.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include "logicalaccess/plugins/crypto/lla_crypto_api.hpp"

namespace logicalaccess
{
namespace openssl
{
/**
 * \brief An AES initialization vector.
 *
 * AESInitializationVector represents an AES initialization vector (IV). Every
 * AESInitializationVector is 16 bytes long.
 *
 * It is meant to be used with an instance of AESCipher.
 */
class LLA_CRYPTO_API AESInitializationVector : public InitializationVector
{
  public:
    /**
     * \brief Create a null IV.
     */
    static AESInitializationVector createNull();

    /**
     * \brief Create a random IV.
     */
    static AESInitializationVector createRandom();

    /**
     * \brief Create an IV from data.
     * \param data The data. Must be 16 exactly bytes long.
     * \return IV from the data.
     * \warning If data is not exactly 16 bytes long, the behavior is undefined.
     */
    static AESInitializationVector createFromData(const ByteVector &data);

  protected:
    /**
     * \brief Create a new AESInitializationVector.
     * \param random true if the IV must be randomized, false otherwise.
     */
    explicit AESInitializationVector(bool random);

    /**
     * \brief Create an AESInitializationVector from existing data.
     * \param data The data.
     * \warning If data is not 16 bytes long, the behavior is undefined.
     */
    explicit AESInitializationVector(const ByteVector &data);

  private:
    /**
     * \brief The AES initialization vector size.
     */
    static const size_t DEFAULT_SIZE = 16;
};
}
}

#endif /* AES_INITIALIZATION_VECTOR_HPP */
/**
 * \file des_initialization_vector.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief DES initialization vector class.
 */

#ifndef DES_INITIALIZATION_VECTOR_HPP
#define DES_INITIALIZATION_VECTOR_HPP

#include <logicalaccess/plugins/crypto/initialization_vector.hpp>

namespace logicalaccess
{
namespace openssl
{
/**
 * \brief An DES initialization vector.
 *
 * DESInitializationVector represents an DES initialization vector (IV). Every
 * DESInitializationVector is 8 bytes long.
 *
 * It is meant to be used with an instance of DESCipher.
 */
class LIBLOGICALACCESS_API DESInitializationVector : public InitializationVector
{
  public:
    /**
     * \brief Create a null IV.
     */
    static DESInitializationVector createNull();

    /**
     * \brief Create a random IV.
     */
    static DESInitializationVector createRandom();

    /**
     * \brief Create an IV from data.
     * \param data The data. Must be 8 exactly bytes long.
     * \return IV from the data.
     * \warning If data is not exactly 8 bytes long, the behavior is undefined.
     */
    static DESInitializationVector createFromData(const ByteVector &data);

  protected:
    /**
     * \brief Create a new DESInitializationVector.
     * \param random true if the IV must be randomized, false otherwise.
     */
    explicit DESInitializationVector(bool random);

    /**
     * \brief Create an DESInitializationVector from existing data.
     * \param data The data.
     * \warning If data is not 8 bytes long, the behavior is undefined.
     */
    explicit DESInitializationVector(const ByteVector &data);

  private:
    /**
     * \brief The DES initialization vector size.
     */
    static const size_t DEFAULT_SIZE = 8;
};
}
}

#endif /* DES_INITIALIZATION_VECTOR_HPP */
/**
 * \file initialization_vector.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Initialization vector base class.
 */

#ifndef INITIALIZATION_VECTOR_HPP
#define INITIALIZATION_VECTOR_HPP

#include <vector>
#include <iostream>
#include <logicalaccess/lla_fwd.hpp>
#include "logicalaccess/plugins/crypto/lla_crypto_api.hpp"

namespace logicalaccess
{
namespace openssl
{
/**
 * \brief An initialisation vector.
 *
 * InitializationVector serves as a base class for AESInitializationVector and DESInitializationVector.
 */
class LLA_CRYPTO_API InitializationVector
{
  public:
    /**
     * \brief Create a new InitializationVector.
     * \param size The IV size.
     * \param random true if the IV must be randomized, false otherwise.
     */
    InitializationVector(size_t size, bool random);

    /**
     * \brief Create an InitializationVector from existing data.
     * \param data The data.
     */
    explicit InitializationVector(const ByteVector &data);

    /**
     * \brief Get the IV data.
     * \return The IV data.
     */
    const ByteVector &data() const
    {
        return d_data;
    }

  protected:

    /**
     * \brief Zero the IV.
     */
    void zero();

    /**
     * \brief Randomize the IV.
     */
    void randomize();

  private:
    /**
     * \brief The IV data.
     */
    ByteVector d_data;
};
}
}

#endif /* INITIALIZATION_VECTOR_HPP */
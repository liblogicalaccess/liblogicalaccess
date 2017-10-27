/**
 * \file samcrypto.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMDESfireCrypto header.
 */

#ifndef SAMCRYPTO_HPP
#define SAMCRYPTO_HPP

#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "desfire/desfirecrypto.hpp"

#ifndef UNIX
#include "logicalaccess/msliblogicalaccess.h"
#endif

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
/**
 * \brief SAM cryptographic functions.
 */
class LIBLOGICALACCESS_API SAMDESfireCrypto : public DESFireCrypto
{
  public:
    /**
     * \brief Constructor
     */
    SAMDESfireCrypto();

    /**
     * \brief Destructor
     */
    virtual ~SAMDESfireCrypto();

    ByteVector authenticateHostP1(std::shared_ptr<DESFireKey> key, ByteVector encRndB,
                                  unsigned char keyno);

    void authenticateHostP2(unsigned char keyno, ByteVector encRndA1,
                            std::shared_ptr<DESFireKey> key);

    ByteVector sam_crc_encrypt(ByteVector d_sessionKey, ByteVector vectordata,
                               std::shared_ptr<DESFireKey> key) const;
};
}

#endif
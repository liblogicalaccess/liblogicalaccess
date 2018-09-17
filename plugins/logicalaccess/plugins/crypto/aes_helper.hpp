//
// Created by xaqq on 7/2/15.
//

#ifndef LIBLOGICALACCESS_AES_HELPER_H
#define LIBLOGICALACCESS_AES_HELPER_H

#include <vector>
#include <cstdint>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{
/**
 * Some static AES helper method with a very easy to use API.
 */
class LIBLOGICALACCESS_API AESHelper
{
  public:
    /**
     * Encrypt `data` using key `key` and `iv_data`.
     *
     * If `iv_data` is empty, use a null IV.
     *
     * Returns the encrypted buffer.
     */
    static ByteVector AESEncrypt(const ByteVector &data, const ByteVector &key,
                                 const ByteVector &iv_data);

    /**
     * Decrypt `data` using key `key` and `iv_data`.
     *
     * If `iv_data` is empty, use a null IV.
     *
     * Returns the decrypted buffer.
     */
    static ByteVector AESDecrypt(const ByteVector &data, const ByteVector &key,
                                 ByteVector const &iv_data);

  private:
    static ByteVector AESRun(const ByteVector &data, const ByteVector &key,
                             const ByteVector &iv_data, bool crypt);
};
}

#endif // LIBLOGICALACCESS_AES_HELPER_H

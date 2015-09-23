//
// Created by xaqq on 7/2/15.
//

#ifndef LIBLOGICALACCESS_AES_HELPER_H
#define LIBLOGICALACCESS_AES_HELPER_H

#include <vector>
#include <cstdint>

namespace logicalaccess
{
    /**
     * Some static AES helper method with a very easy to use API.
     */
    class AESHelper
    {
    public:

        /**
         * Encrypt `data` using key `key` and `iv_data`.
         *
         * If `iv_data` is empty, use a null IV.
         *
         * Returns the encrypted buffer.
         */
        static std::vector<uint8_t> AESEncrypt(const std::vector<uint8_t> &data,
                                               const std::vector<uint8_t> &key,
                                               const std::vector<uint8_t> &iv_data);

        /**
         * Decrypt `data` using key `key` and `iv_data`.
         *
         * If `iv_data` is empty, use a null IV.
         *
         * Returns the decrypted buffer.
         */
        static std::vector<uint8_t> AESDecrypt(const std::vector<uint8_t> &data,
                                               const std::vector<uint8_t> &key,
                                               std::vector<uint8_t> const &iv_data);
        
    private:
        static std::vector<uint8_t>  AESRun(const std::vector<uint8_t> &data, const std::vector<uint8_t> &key,
                                            const std::vector<uint8_t> &iv_data,
                                            bool crypt);

    };
}

#endif //LIBLOGICALACCESS_AES_HELPER_H

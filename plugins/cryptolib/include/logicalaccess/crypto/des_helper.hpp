#pragma once

#include <vector>
#include <cstdint>

namespace logicalaccess
{
/**
 * Some static DES helper method with a very easy to use API.
 */
class DESHelper {
      public:

        /**
         * Encrypt `data` using key `key` and `iv_data`.
         *
         * If `iv_data` is empty, use a null IV.
         *
         * Returns the encrypted buffer.
         */
        static std::vector <uint8_t> DESEncrypt(const std::vector <uint8_t> &data,
                                                const std::vector <uint8_t> &key,
                                                const std::vector <uint8_t> &iv_data);

        /**
         * Decrypt `data` using key `key` and `iv_data`.
         *
         * If `iv_data` is empty, use a null IV.
         *
         * Returns the decrypted buffer.
         */
        static std::vector <uint8_t> DESDecrypt(const std::vector <uint8_t> &data,
                                                const std::vector <uint8_t> &key,
                                                std::vector <uint8_t> const &iv_data);

      private:
        static std::vector <uint8_t> DESRun(const std::vector <uint8_t> &data,
                                            const std::vector <uint8_t> &key,
                                            const std::vector <uint8_t> &iv_data,
                                            bool crypt);
};
}
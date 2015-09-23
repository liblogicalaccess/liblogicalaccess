//
// Created by xaqq on 7/2/15.
//

#ifndef LIBLOGICALACCESS_LLA_RANDOM_HPP
#define LIBLOGICALACCESS_LLA_RANDOM_HPP

#include <random>
#include <mutex>

namespace logicalaccess
{
    /**
     * An helper to generate random number.
     *
     * This is a wrapper around OpenSSL cryptographically secure random
     * number generator.
     */
    class RandomHelper
    {
    public:
        /**
         * Generate 1 random byte.
         */
        static uint8_t byte();

        /**
         * Generate `size` random bytes.
         */
        static std::vector<uint8_t> bytes(size_t size);

    private:
        /**
         * OpenSSL's random number generation is not thread-safe.
         */
        static std::mutex mutex_;
    };
}

#endif //LIBLOGICALACCESS_LLA_RANDOM_HPP

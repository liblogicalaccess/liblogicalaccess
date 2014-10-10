/**
 * \file sha.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief SHA hash functions.
 */

#ifndef SHA_HPP
#define SHA_HPP

#include <vector>

#include <iostream>

namespace logicalaccess
{
    namespace openssl
    {
        /**
         * \brief Get a SHA-256 hash.
         * \param buffer The buffer to get the hash from.
         * \return The hash.
         */
        std::vector<unsigned char> SHA256Hash(const std::vector<unsigned char>& buffer);

        /**
         * \brief Get a SHA-256 hash.
         * \param str The string to get the hash from.
         * \return The hash.
         */
        std::vector<unsigned char> SHA256Hash(const std::string& str);
    }
}

#endif /* SHA_HPP */
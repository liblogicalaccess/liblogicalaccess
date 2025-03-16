//
// Created by xaqq on 4/4/18.
//

#ifndef LIBLOGICALACCESS_SIGNATURE_HELPER_HPP
#define LIBLOGICALACCESS_SIGNATURE_HELPER_HPP

#include "logicalaccess/plugins/crypto/lla_crypto_api.hpp"
#include <string>
#include <vector>

namespace logicalaccess
{

/**
 * Some static pubkey based signature utils.
 */
class LLA_CRYPTO_API SignatureHelper
{
  public:
    /**
     * Verify that the signature of `data` matches `signature`.
     *
     * pem_pubkey is a text PEM encoded public to use for signature verification.
     *
     * RSA / SHA512 is used.
     *
     * @param data
     * @param signature
     * @param pem_pubkey
     * @return true if signature is valid, false otherwise.
     */
    static bool verify_sha512(const std::string &data, const std::string &signature,
                              const std::string &pem_pubkey);

    static bool verify_ecdsa_secp224r1(const std::vector<unsigned char> &data,
                                       const std::vector<unsigned char> &signature,
                                       const std::vector<unsigned char> &pem_pubkey);
};
}

#endif // LIBLOGICALACCESS_SIGNATURE_HELPER_HPP

/**
 * \file des_cipher.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DES cipher classes.
 */

#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/symmetric_key.hpp"

#include <cassert>

namespace logicalaccess
{
    namespace openssl
    {
        const EVP_CIPHER* DESCipher::getEVPCipher(const SymmetricKey& key) const
        {
            const EVP_CIPHER* evpCipher = nullptr;

            size_t desbytes = key.data().size();

            switch (desbytes)
            {
            case 8:
            {
                switch (mode())
                {
                case ENC_MODE_CBC:
                    evpCipher = EVP_des_cbc();
                    break;

                case ENC_MODE_CFB:
                    evpCipher = EVP_des_cfb();
                    break;

                case ENC_MODE_CFB1:
                    evpCipher = EVP_des_cfb1();
                    break;

                case ENC_MODE_CFB8:
                    evpCipher = EVP_des_cfb8();
                    break;

                case ENC_MODE_ECB:
                    evpCipher = EVP_des_ecb();
                    break;

                case ENC_MODE_OFB:
                    evpCipher = EVP_des_ofb();
                    break;
                }

                break;
            }
            case 16:
            {
                switch (mode())
                {
                case ENC_MODE_CBC:
                    evpCipher = EVP_des_ede_cbc();
                    break;

                case ENC_MODE_CFB:
                    evpCipher = EVP_des_ede_cfb();
                    break;

                case ENC_MODE_ECB:
                    evpCipher = EVP_des_ede_ecb();
                    break;

                case ENC_MODE_OFB:
                    evpCipher = EVP_des_ede_ofb();
                    break;

                case ENC_MODE_CFB1:
                    evpCipher = nullptr;
                    break;

                case ENC_MODE_CFB8:
                    evpCipher = nullptr;
                    break;
                }

                break;
            }
            case 24:
            {
                switch (mode())
                {
                case ENC_MODE_CBC:
                    evpCipher = EVP_des_ede3_cbc();
                    break;

                case ENC_MODE_CFB:
                    evpCipher = EVP_des_ede3_cfb();
                    break;

                case ENC_MODE_CFB1:
                    evpCipher = EVP_des_ede3_cfb1();
                    break;

                case ENC_MODE_CFB8:
                    evpCipher = EVP_des_ede3_cfb8();
                    break;

                case ENC_MODE_ECB:
                    evpCipher = EVP_des_ede3_ecb();
                    break;

                case ENC_MODE_OFB:
                    evpCipher = EVP_des_ede3_ofb();
                    break;
                }

                break;
            }
            default:
            {
                assert(false);
            }
            }

            return evpCipher;
        }
    }
}
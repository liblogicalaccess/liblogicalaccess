/**
 * \file evp_pkey.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An EVP PKEY class.
 */

#include "logicalaccess/crypto/evp_pkey.hpp"
#include "logicalaccess/crypto/rsa_key.hpp"
#include "logicalaccess/logs.hpp"
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/crypto/openssl.hpp"

#include <cassert>
#include <cstring>

namespace logicalaccess
{
    namespace openssl
    {
        EVPPKey EVPPKey::createFromRSAKey(const RSAKey& key)
        {
            std::shared_ptr<EVP_PKEY> _pkey(EVP_PKEY_new(), EVP_PKEY_free);

            EXCEPTION_ASSERT(_pkey, std::runtime_error, "Unable to create a EVP_PKEY structure");
            EVP_PKEY_set1_RSA(_pkey.get(), key.d_rsa.get());

            return EVPPKey(_pkey, key.hasPrivateCompound());
        }

        int EVPPKey::type() const
        {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
            return EVP_PKEY_type(d_pkey.get()->type);
#else
            return EVP_PKEY_base_id(d_pkey.get());
#endif
        }

        EVPPKey EVPPKey::discardPrivateCompound() const
        {
            if (hasPrivateCompound())
            {
                if (type() == EVP_PKEY_RSA)
                {
                    return createFromRSAKey(rsaKey().discardPrivateCompound());
                }

                THROW_EXCEPTION_WITH_LOG(std::runtime_error, "Unsupported key type");
            }
            else
            {
                return *this;
            }
        }

        RSAKey EVPPKey::rsaKey() const
        {
            if (type() == EVP_PKEY_RSA)
            {
                std::shared_ptr<RSA> rsa(EVP_PKEY_get1_RSA(d_pkey.get()), RSA_free);

                return RSAKey(rsa, d_has_private_compound);
            }

            THROW_EXCEPTION_WITH_LOG(std::runtime_error, "Key type is not RSA");
        }

        EVPPKey::EVPPKey(std::shared_ptr<EVP_PKEY> pkey, bool has_private_compound) :
            d_pkey(pkey),
            d_has_private_compound(has_private_compound)
        {
            OpenSSLInitializer::GetInstance();
            assert(d_pkey);
        }
    }
}

/**
 * \file evp_pkey.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief An EVP PKEY class.
 */

#ifndef EVP_PKEY_HPP
#define EVP_PKEY_HPP

#include "logicalaccess/crypto/pem.h"

#include <string>
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <openssl/evp.h>

namespace LOGICALACCESS
{
	namespace openssl
	{
		class RSAKey;

		/**
		 * \brief An EVP PKEY.
		 *
		 * EVPPKey holds an EVP public or private key. It mainly serves as a wrapper for OpenSSL routines that require a EVP_PKEY.
		 *
		 * You can create a new EVPPKey from a RSAKey using createFromRSAKey() or do the reverse operation, using rsaKey().
		 */
		class EVPPKey
		{
			public:

				/**
				 * \brief Create an EVPPKey instance from an RSAKey.
				 * \param key The key.
				 * \return The EVPPKey instance.
				 */
				static EVPPKey createFromRSAKey(const RSAKey& key);

				/**
				 * \brief Get the type of this EVP PKEY.
				 * \return The type.
				 */
				int type() const;

				/**
				 * \brief Check if the EVP PKEY has a private compound.
				 * \return true if the EVP PKEY has a private compound, false otherwise.
				 */
				inline bool hasPrivateCompound() const
				{
					return d_has_private_compound;
				}

				/**
				 * \brief Get a copy of the EVP PKEY without any private compound.
				 * \return A copy of the EVP PKEY, without the private compound, if any.
				 */
				EVPPKey discardPrivateCompound() const;

				/**
				 * \brief Get the RSAKey.
				 * \return The RSAKey. If the EVPPKey does not contain any RSAKey, an invalid call exception is thrown.
				 */
				RSAKey rsaKey() const;

			protected:

				/**
				 * \brief Create an EVPPKey from a EVP_PKEY structure pointer.
				 * \param pkey The EVP_PKEY structure pointer. If pkey is NULL, the behavior is undefined.
				 * \param has_private_compound true if the EVP_PKEY has a private compound, false otherwise.
				 */
				EVPPKey(boost::shared_ptr<EVP_PKEY> pkey, bool has_private_compound);

			private:

				/**
				 * \brief The internal EVP_PKEY structure pointer.
				 */
				boost::shared_ptr<EVP_PKEY> d_pkey;

				/**
				 * \brief The private compound flag.
				 */
				bool d_has_private_compound;

				friend class X509Certificate;
				friend class X509CertificateRequest;
		};
	}
}

#endif /* EVP_PKEY_HPP */


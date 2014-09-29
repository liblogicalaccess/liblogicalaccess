/**
 * \file rsa_key.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief RSA key base class.
 */

#ifndef RSA_KEY_HPP
#define RSA_KEY_HPP

#include "logicalaccess/crypto/asymmetric_key.hpp"
#include "logicalaccess/crypto/pem.hpp"

#include <vector>
#include <chrono>

#include <boost/shared_ptr.hpp>

#include <openssl/rsa.h>
#include <openssl/err.h>

namespace logicalaccess
{
	namespace openssl
	{
		/**
		 * \brief A RSA key.
		 *
		 * RSAKey represents a RSA key pair.
		 *
		 * A RSAKey is nullable. Before using one of its method, you should ensure that the instance is not null.
		 *
		 * If the RSAKey is not null, the private key compound can be null, but the public key compound is always defined.
		 */
		class RSAKey : public AsymmetricKey
		{
			public:

				/**
				 * \brief Create a random RSAKey.
				 * \return The RSAKey.
				 */
				static RSAKey createRandom();

				/**
				 * \brief Create a RSAKey from a public key compound.
				 * \param data The key data.
				 * \return The RSAKey.
				 * \warning If no public compound can be read from data, an InvalidCallException is thrown.
				 */
				static RSAKey createFromPublicCompound(const std::vector<unsigned char>& data);

				/**
				 * \brief Create a RSAKey from a private key compound.
				 * \param data The key data.
				 * \return The RSAKey.
				 * \warning If no private compound can be read from data, an InvalidCallException is thrown.
				 */
				static RSAKey createFromPrivateCompound(const std::vector<unsigned char>& data);

				/**
				 * \brief Create a RSAKey from a PEM public key.
				 * \param data The PEM key data.
				 * \param callback A passphrase callback function.
				 * \param userdata Some user data to pass to the callback function.
				 * \return The RSAKey.
				 */
				static RSAKey createFromPEMPublicKey(const std::vector<unsigned char>& data, PEMPassphraseCallback callback = NULL, void* userdata = NULL);

				/**
				 * \brief Create a RSAKey from a PEM private key.
				 * \param data The PEM key data.
				 * \param callback A passphrase callback function.
				 * \param userdata Some user data to pass to the callback function.
				 * \return The RSAKey.
				 */
				static RSAKey createFromPEMPrivateKey(const std::vector<unsigned char>& data, PEMPassphraseCallback callback = NULL, void* userdata = NULL);

				/**
				 * \brief Create a RSAKey from a PEM public key file.
				 * \param filename The filename.
				 * \param callback A passphrase callback function.
				 * \param userdata Some user data to pass to the callback function.
				 * \return The RSAKey.
				 */
				static RSAKey createFromPEMPublicKeyFile(const std::string& filename, PEMPassphraseCallback callback = NULL, void* userdata = NULL);

				/**
				 * \brief Create a RSAKey from a PEM private key file.
				 * \param filename The filename.
				 * \param callback A passphrase callback function.
				 * \param userdata Some user data to pass to the callback function.
				 * \return The RSAKey.
				 */
				static RSAKey createFromPEMPrivateKeyFile(const std::string& filename, PEMPassphraseCallback callback = NULL, void* userdata = NULL);

				/**
				 * \brief Constructor.
				 * Create an empty RSA key.
				 */
				RSAKey();

				/**
				 * \brief Get the raw pointer.
				 * \return The raw pointer.
				 */
				inline boost::shared_ptr<RSA> getRaw() const
				{
					return d_rsa;
				}

				/**
				 * \brief Get the RSA key in PEM format.
				 * \param discard_private_compound If true, only the public key part is extracted. Default is false.
				 * \param callback A passphrase callback function. Default is NULL.
				 * \param userdata Some user data to pass to the callback function. Default is NULL.
				 * \return The RSA key in PEM format.
				 */
				std::vector<unsigned char> getPEM(bool discard_private_compound = false, PEMPassphraseCallback callback = NULL, void* userdata = NULL) const;

				/**
				 * \brief Get the public compound.
				 * \return The public compound.
				 */
				const std::vector<unsigned char>& publicCompound() const;

				/**
				 * \brief Get the private compound.
				 * \return The private compound.
				 */
				const std::vector<unsigned char>& privateCompound() const;

				/**
				 * \brief Get a copy of the RSA key without any private compound.
				 * \return A copy of the RSA key, without the private compound, if any.
				 */
				RSAKey discardPrivateCompound() const;

				/**
				 * \brief Check if the RSA key has a private compound.
				 * \return true if the RSA key has a private compound, false otherwise.
				 */
				inline bool hasPrivateCompound() const
				{
					return d_has_private_key;
				}

				/**
				 * \brief Save the RSAKey to a PEM file.
				 * \param filename The filename.
				 * \param callback A passphrase callback function.
				 * \param userdata Some user data to pass to the callback function.
				 */
				void writeToPEMKeyFile(const std::string& filename, PEMPassphraseCallback callback = NULL, void* userdata = NULL) const;

				/**
				 * \brief Clear the RSA key so it becomes null.
				 */
				inline void clear()
				{
					d_rsa.reset();
					d_has_private_key = false;
					d_public_key.clear();
					d_private_key.clear();
				}

			private:

				/**
				 * \brief Create a RSAKey by copying a raw RSA structure.
				 * \param raw A RSA structure pointer. raw's ownership is *NOT* taken !
				 * \param has_private_key Wether the specified RSA pointer contains a private compound or not.
				 * \return A RSAKey instance whose data is copied from raw.
				 */
				static RSAKey createFromRaw(RSA* raw, bool has_private_key);

				/**
				 * \brief Create a RSAKey from a RSA pointer.
				 * \param rsa The RSA pointer. If rsa is NULL, the behavior is undefined.
				 * \param has_private_key Wether the specified RSA pointer contains a private compound or not.
				 */
				RSAKey(boost::shared_ptr<RSA> rsa, bool has_private_key);

				/**
				 * \brief The boolean test.
				 * \return true if d_rsa is set.
				 */
				inline bool boolean_test() const
				{
					return bool(d_rsa);
				}

				/**
				 * \brief Update the cache.
				 */
				void cache();

				/**
				 * \brief The RSA keys pair.
				 */
				boost::shared_ptr<RSA> d_rsa;

				/**
				 * \brief The private key flag.
				 */
				bool d_has_private_key;

				/**
				 * \brief The public key part.
				 */
				std::vector<unsigned char> d_public_key;

				/**
				 * \brief The private key part.
				 */
				std::vector<unsigned char> d_private_key;

				friend bool operator==(const RSAKey& lhs, const RSAKey& rhs);
				friend class RSACipher;
				friend class X509Certificate;
				friend class X509CertificateRequest;
				friend class EVPPKey;
		};

		/**
		 * \brief Comparaison operator.
		 * \param lhs The left parameter.
		 * \param rhs The right parameter.
		 * \return true if the two pair of keys have the same values.
		 */
		bool operator==(const RSAKey& lhs, const RSAKey& rhs);

		/**
		 * \brief Comparaison operator.
		 * \param lhs The left parameter.
		 * \param rhs The right parameter.
		 * \return true if the two pair of keys have the same values.
		 */
		inline bool operator!=(const RSAKey& lhs, const RSAKey& rhs)
		{
			return !(lhs == rhs);
		}
	}
}

#endif /* RSA_KEY_HPP */


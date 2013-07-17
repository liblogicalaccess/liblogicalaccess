/**
 * \file x509_certificate.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief A X509 certificate class.
 */

#ifndef X509_CERTIFICATE_HPP
#define X509_CERTIFICATE_HPP

#include "logicalaccess/crypto/pem.hpp"

#include <string>

#include <openssl/x509.h>

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace logicalaccess
{
	namespace openssl
	{
		class EVPPKey;
		class X509Name;
		class X509CertificateRequest;

		/**
		 * \brief A X509 certificate.
		 *
		 * X509Certificate represents a X509 certificate. X509Certificate can be loaded from a file (createFromFile()), from memory (createFromPEM()) or created from scratch (createSelfSignedFromScratch()).
		 *
		 * You can also get a X509Certificate from a X509CertificateRequest (see createCASignedFromX509CertificateRequest()) or get a X509CertificateRequest from a X509Certificate (see toSignedX509CertificateRequest()).
		 *
		 * X509Certificate is nullable. Before using one of its method you must ensure that the instance is not null.
		 */
		class X509Certificate
		{
			public:

				/**
				 * \brief Create a null X509Certificate.
				 */
				inline X509Certificate() {}

				/**
				 * \brief Create a X509 certificate by loading it from a file.
				 * \param filename The filename.
				 * \param callback A passphrase callback function.
				 * \param userdata Some user data to pass to the callback function.
				 * \return The X509Certificate.
				 */
				static X509Certificate createFromFile(const std::string& filename, PEMPassphraseCallback callback = NULL, void* userdata = NULL);

				/**
				 * \brief Create a X509 certificate from a PEM buffer.
				 * \param data The buffer.
				 * \param callback A passphrase callback function.
				 * \param userdata Some user data to pass to the callback function.
				 * \return The X509Certificate.
				 */
				static X509Certificate createFromPEM(const std::vector<unsigned char>& data, PEMPassphraseCallback callback = NULL, void* userdata = NULL);

				/**
				 * \brief Create a self-signed X509 certificate.
				 * \param pkey The private key to use. If pkey is not a RSAKey or if it has no private compound, the behavior is undefined.
				 * \param name The X509Name structure.
				 * \param notBefore The "not before" date.
				 * \param notAfter The "not after" date.
				 * \param digest_nid The digest nid. Default is NID_sha1WithRSAEncryption.
				 * \return self-signed X509 certificate.
				 */
				static X509Certificate createSelfSignedFromScratch(const EVPPKey& pkey, const X509Name& name, const boost::posix_time::ptime& notBefore, const boost::posix_time::ptime& notAfter, int digest_nid = NID_sha1WithRSAEncryption);

				/**
				 * \brief Create a CA-signed X509 certificate from a request.
				 * \param req The X509CertificateRequest. If req is null, the behavior is undefined.
				 * \param ca_cert The CA certificate. If ca_cert is null, the behavior is undefined.
				 * \param ca_pkey The CA private key. If ca_pkey is null or has no private compound, the behavior is undefined.
				 * \param duration The duration, in days. Default is 365 days.
				 * \param digest_nid The digest nid. Default is NID_sha1WithRSAEncryption.
				 * \return CA-signed X509 certificate.
				 */
				static X509Certificate createCASignedFromX509CertificateRequest(
				    const X509CertificateRequest& req,
				    const X509Certificate& ca_cert,
				    const EVPPKey& ca_pkey,
				    int duration = 365,
				    int digest_nid = NID_sha1WithRSAEncryption
				);

				/**
				 * \brief Get the raw pointer.
				 * \return The raw pointer.
				 */
				inline boost::shared_ptr<X509> getRaw() const
				{
					return d_cert;
				}

				/**
				 * \brief Get the X509 certificate in PEM format.
				 * \return The X509 certificate in PEM format.
				 */
				std::vector<unsigned char> getPEM() const;

				/**
				 * \brief Get the associated public key.
				 * \return The associated public key.
				 */
				EVPPKey publicKey() const;

				/**
				 * \brief Get the subject name of the certificate.
				 * \return The subject name.
				 */
				X509Name subjectName() const;

				/**
				 * \brief Get the issuer name of the certificate.
				 * \return The issuer name.
				 */
				X509Name issuerName() const;

				/**
				 * \brief Verify the certificate.
				 * \param pkey The public key. If key is NULL or has a private compound, the behavior is undefined.
				 * \return true on success, false otherwise.
				 */
				bool verify(const EVPPKey& pkey) const;

				/**
				 * \brief Check if the certificate matches the supplied private key.
				 * \param pkey The private key. If key is NULL or has no private compound, the behavior is undefined.
				 * \return true on success, false otherwise.
				 */
				bool checkPrivateKey(const EVPPKey& pkey) const;

				/**
				 * \brief Create a X509CertificateRequest from the X509Certificate.
				 * \param pkey The private key. If key is NULL or has no private compound, the behavior is undefined.
				 * \param digest_nid The digest nid. Default is NID_sha1WithRSAEncryption.
				 * \return A signed X509CertificateRequest object.
				 */
				X509CertificateRequest toSignedX509CertificateRequest(const EVPPKey& pkey, int digest_nid = NID_sha1WithRSAEncryption) const;

				/**
				 * \brief Write the certificate to a file.
				 * \param filename The filename.
				 *
				 * Use writeToFile() to write a X509Certificate to a file, in PEM format.
				 */
				void writeToFile(const std::string& filename) const;

				/**
				 * \brief Clear the certificate so it becomes null.
				 */
				inline void clear()
				{
					d_cert.reset();
				}

				/**
				 * \brief Perform a deep copy.
				 */
				void deepCopy();

			private:

				/**
				 * \brief Set the serial number for a given certificate.
				 * \param cert The X509 certificate. If cert is NULL, the behavior is undefined.
				 */
				static void setSerialNumber(boost::shared_ptr<X509> cert);

				/**
				 * \brief Add a X509v3 extension to a given certificate.
				 * \param cert The X509 certificate. If cert is NULL, the behavior is undefined.
				 * \param ctx The X509v3 context. If ctx is NULL, the behavior is undefined.
				 * \param key The key.
				 * \param value The value.
				 */
				static void addExtension(boost::shared_ptr<X509> cert, boost::shared_ptr<X509V3_CTX> ctx, const std::string& key, const std::string& value);

				/**
				 * \brief Sign a certificate with a given key.
				 * \param cert The X509 certificate. If cert is NULL, the behavior is undefined.
				 * \param pkey The private key. If key is NULL or has no private compound, the behavior is undefined.
				 * \param digest_nid The digest nid. Default is NID_sha1WithRSAEncryption.
				 */
				static void signCertificateWithKey(boost::shared_ptr<X509> cert, const EVPPKey& pkey, int digest_nid = NID_sha1WithRSAEncryption);

				/**
				 * \brief Check if a specified NID must can be copied from CA certificate to a client certificate.
				 * \param nid The NID.
				 * \return true if the copy is ok, false otherwise.
				 */
				static bool CheckNidForCopy(int nid);

				/**
				 * \brief Create a certificate from a X509 structure pointer.
				 * \param cert The X509 structure pointer. If cert is NULL, the behavior is undefined.
				 */
				X509Certificate(boost::shared_ptr<X509> cert);

				/**
				 * \brief The boolean test.
				 * \return true if d_cert is set.
				 */
				inline bool boolean_test() const
				{
					return bool(d_cert);
				}

				/**
				 * \brief The internal X509 structure pointer.
				 */
				boost::shared_ptr<X509> d_cert;

				friend bool operator==(const X509Certificate& lhs, const X509Certificate& rhs);
				friend bool operator!=(const X509Certificate& lhs, const X509Certificate& rhs);
				friend class SecureEndPoint;
				friend class SecureSocket;
				friend class X509CertificateStoreContext;
		};

		/**
		 * \brief Comparison operator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificates are equal.
		 */
		bool operator==(const X509Certificate& lhs, const X509Certificate& rhs);

		/**
		 * \brief Comparison operator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificates are different.
		 */
		bool operator!=(const X509Certificate& lhs, const X509Certificate& rhs);
	}
}

#endif /* X509_CERTIFICATE_HPP */


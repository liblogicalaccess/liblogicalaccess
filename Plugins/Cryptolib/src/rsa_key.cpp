/**
 * \file rsa_key.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief RSA key class.
 */

#include "logicalaccess/crypto/rsa_key.h"
#include "logicalaccess/logs.h"
#include "logicalaccess/crypto/openssl_exception.h"
#include "logicalaccess/MyException.h"

#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>

namespace LOGICALACCESS
{
	namespace openssl
	{
		RSAKey RSAKey::createRandom()
		{
			RSAKey rsa(boost::shared_ptr<RSA>(RSA_generate_key(1024, rand() | 1, NULL, NULL), RSA_free), true);

			EXCEPTION_ASSERT_WITH_LOG(rsa.d_rsa, OpenSSLException, "Cannot generate RSA key pair");

			rsa.cache();

			return rsa;
		}

		RSAKey RSAKey::createFromPublicCompound(const std::vector<unsigned char>& data)
		{
			const unsigned char* buf = reinterpret_cast<const unsigned char*>(&data[0]);

			RSA* prsa = NULL;

			if (!d2i_RSAPublicKey(&prsa, &buf, static_cast<long>(data.size())))
			{
				THROW_EXCEPTION_WITH_LOG(OpenSSLException, "Unable to read public key compound from the specified buffer.");
			}
			else
			{
				boost::shared_ptr<RSA> sprsa(prsa, RSA_free);

				return RSAKey(sprsa, false);
			}
		}

		RSAKey RSAKey::createFromPrivateCompound(const std::vector<unsigned char>& data)
		{
			const unsigned char* buf = reinterpret_cast<const unsigned char*>(&data[0]);

			RSA* prsa = NULL;

			if (!d2i_RSAPrivateKey(&prsa, &buf, static_cast<long>(data.size())))
			{
				THROW_EXCEPTION_WITH_LOG(OpenSSLException, "Unable to read private key compound from the specified buffer.");
			}
			else
			{
				boost::shared_ptr<RSA> sprsa(prsa, RSA_free);

				return RSAKey(sprsa, true);
			}
		}

		RSAKey RSAKey::createFromPEMPublicKey(const std::vector<unsigned char>& data, PEMPassphraseCallback callback, void* userdata)
		{
			std::vector<unsigned char> datacopy = data;

			boost::shared_ptr<BIO> pbio(BIO_new_mem_buf(datacopy.data(), (int)datacopy.size()), BIO_free);

			RSA* prsa = PEM_read_bio_RSA_PUBKEY(pbio.get(), NULL, callback, userdata);

			EXCEPTION_ASSERT_WITH_LOG(prsa, OpenSSLException, "Unable to parse the RSA public key PEM data");

			boost::shared_ptr<RSA> sprsa(prsa, RSA_free);

			return RSAKey(sprsa, false);
		}

		RSAKey RSAKey::createFromPEMPrivateKey(const std::vector<unsigned char>& data, PEMPassphraseCallback callback, void* userdata)
		{
			std::vector<unsigned char> datacopy = data;

			boost::shared_ptr<BIO> pbio(BIO_new_mem_buf(datacopy.data(), (int)datacopy.size()), BIO_free);

			RSA* prsa = PEM_read_bio_RSAPrivateKey(pbio.get(), NULL, callback, userdata);

			EXCEPTION_ASSERT_WITH_LOG(prsa, OpenSSLException, "Unable to parse the RSA public key PEM data");

			boost::shared_ptr<RSA> sprsa(prsa, RSA_free);

			return RSAKey(sprsa, true);
		}

		RSAKey RSAKey::createFromPEMPublicKeyFile(const std::string& filename, PEMPassphraseCallback callback, void* userdata)
		{
			FILE* fp = fopen(filename.c_str(), "r");

			if (fp == NULL)
			{
			  THROW_EXCEPTION_WITH_LOG(Exception::exception, "Cannot open the file.");
			}

			RSA* prsa = PEM_read_RSAPublicKey(fp, NULL, callback, userdata);

			fclose(fp);

			EXCEPTION_ASSERT_WITH_LOG(prsa, OpenSSLException, "Unable to parse the RSA public key PEM file");

			boost::shared_ptr<RSA> sprsa(prsa, RSA_free);

			return RSAKey(sprsa, false);
		}

		RSAKey RSAKey::createFromPEMPrivateKeyFile(const std::string& filename, PEMPassphraseCallback callback, void* userdata)
		{
			FILE* fp = fopen(filename.c_str(), "r");

			if (fp == NULL)
			{
				THROW_EXCEPTION_WITH_LOG(Exception::exception, "Cannot open the file.");
			}

			RSA* prsa = PEM_read_RSAPrivateKey(fp, NULL, callback, userdata);

			fclose(fp);

			EXCEPTION_ASSERT_WITH_LOG(prsa, OpenSSLException, "Unable to parse the RSA private key PEM file");

			boost::shared_ptr<RSA> sprsa(prsa, RSA_free);

			return RSAKey(sprsa, true);
		}

		RSAKey::RSAKey() :
			d_has_private_key(false)
		{
		}

		std::vector<unsigned char> RSAKey::getPEM(bool discard_private_compound, PEMPassphraseCallback callback, void* userdata) const
		{
			boost::shared_ptr<BIO> pbio(BIO_new(BIO_s_mem()), BIO_free);

			if (hasPrivateCompound() && (!discard_private_compound))
			{
				EXCEPTION_ASSERT_WITH_LOG(PEM_write_bio_RSAPrivateKey(pbio.get(), d_rsa.get(), NULL, NULL, 0, callback, userdata), OpenSSLException, "Cannot write PEM data");
			}
			else
			{
				EXCEPTION_ASSERT_WITH_LOG(PEM_write_bio_RSA_PUBKEY(pbio.get(), d_rsa.get()), OpenSSLException, "Cannot write PEM data");
			}

			int len = BIO_pending(pbio.get());

			std::vector<unsigned char> buffer(len);

			BIO_read(pbio.get(), buffer.data(), len);
			buffer.resize(len);

			return buffer;
		}

		const std::vector<unsigned char>& RSAKey::publicCompound() const
		{
			return d_public_key;
		}

		const std::vector<unsigned char>& RSAKey::privateCompound() const
		{
			return d_private_key;
		}

		RSAKey RSAKey::discardPrivateCompound() const
		{
			if (hasPrivateCompound())
			{
				return createFromPEMPublicKey(getPEM(true));
			}
			else
			{
				return *this;
			}
		}

		void RSAKey::writeToPEMKeyFile(const std::string& filename, PEMPassphraseCallback callback, void* userdata) const
		{
			FILE* fp = fopen(filename.c_str(), "w+");

			if (fp == NULL)
			{
				THROW_EXCEPTION_WITH_LOG(Exception::exception, "Cannot open the file.");
			}

			int result = 0;

			if (hasPrivateCompound())
			{
				result = PEM_write_RSAPrivateKey(fp, d_rsa.get(), NULL, NULL, 0, callback, userdata);
			}
			else
			{
				result = PEM_write_RSAPublicKey(fp, d_rsa.get());
			}

			fclose(fp);

			EXCEPTION_ASSERT_WITH_LOG(result != 0, OpenSSLException, "Cannot write PEM file");
		}

		RSAKey RSAKey::createFromRaw(RSA* raw, bool has_private_key)
		{
			boost::shared_ptr<RSA> sprsa;

			if (has_private_key)
			{
				sprsa.reset(RSAPrivateKey_dup(raw), RSA_free);
			}
			else
			{
				sprsa.reset(RSAPublicKey_dup(raw), RSA_free);
			}

			return RSAKey(sprsa, has_private_key);
		}

		RSAKey::RSAKey(boost::shared_ptr<RSA> rsa, bool has_private_key) :
			d_rsa(rsa),
			d_has_private_key(has_private_key)
		{
			assert(rsa);
			cache();
		}

		void RSAKey::cache()
		{
			unsigned char buffer[8192];
			unsigned char* buf = buffer;

			int len = i2d_RSAPublicKey(d_rsa.get(), &buf);

			EXCEPTION_ASSERT_WITH_LOG(len >= 0, OpenSSLException, "Invalid RSA context: cannot retrieve public key");

			d_public_key.clear();
			d_public_key.insert(d_public_key.end(), buffer, buffer + len);

			if (hasPrivateCompound())
			{
				buf = buffer;
				len = i2d_RSAPrivateKey(d_rsa.get(), &buf);

				EXCEPTION_ASSERT_WITH_LOG(len >= 0, OpenSSLException, "Invalid RSA context: cannot retrieve private key");

				d_private_key.clear();
				d_private_key.insert(d_private_key.end(), buffer, buffer + len);
			}
			else
			{
				d_private_key.clear();
			}
		}

		bool operator==(const RSAKey& lhs, const RSAKey& rhs)
		{
			if (lhs.hasPrivateCompound())
			{
				if (rhs.hasPrivateCompound())
				{
					return lhs.d_private_key == rhs.d_private_key;
				}
			}
			else
			{
				if (!rhs.hasPrivateCompound())
				{
					return lhs.d_public_key == rhs.d_public_key;
				}
			}

			return false;
		}
	}
}


/**
 * \file aes_cipher.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief AES cipher class.
 */

#include "logicalaccess/crypto/aes_cipher.h"
#include "logicalaccess/crypto/symmetric_key.h"

#include <cassert>

namespace LOGICALACCESS
{
	namespace openssl
	{
		const EVP_CIPHER*  AESCipher::getEVPCipher(const SymmetricKey& key) const
		{
			const EVP_CIPHER* evpCipher = NULL;

			size_t aeslength = (key.data().size() * 8);

			switch (aeslength)
			{
				case 128:
					{
						switch (mode())
						{
							case ENC_MODE_CBC:
								evpCipher = EVP_aes_128_cbc();
								break;

							case ENC_MODE_CFB:
								evpCipher = EVP_aes_128_cfb();
								break;

							case ENC_MODE_CFB1:
								evpCipher = EVP_aes_128_cfb1();
								break;

							case ENC_MODE_CFB8:
								evpCipher = EVP_aes_128_cfb8();
								break;

							case ENC_MODE_ECB:
								evpCipher = EVP_aes_128_ecb();
								break;

							case ENC_MODE_OFB:
								evpCipher = EVP_aes_128_ofb();
								break;
						}

						break;
					}
				case 192:
					{
						switch (mode())
						{
							case ENC_MODE_CBC:
								evpCipher = EVP_aes_192_cbc();
								break;

							case ENC_MODE_CFB:
								evpCipher = EVP_aes_192_cfb();
								break;

							case ENC_MODE_CFB1:
								evpCipher = EVP_aes_192_cfb1();
								break;

							case ENC_MODE_CFB8:
								evpCipher = EVP_aes_192_cfb8();
								break;

							case ENC_MODE_ECB:
								evpCipher = EVP_aes_192_ecb();
								break;

							case ENC_MODE_OFB:
								evpCipher = EVP_aes_192_ofb();
								break;
						}

						break;
					}
				case 256:
					{
						switch (mode())
						{
							case ENC_MODE_CBC:
								evpCipher = EVP_aes_256_cbc();
								break;

							case ENC_MODE_CFB:
								evpCipher = EVP_aes_256_cfb();
								break;

							case ENC_MODE_CFB1:
								evpCipher = EVP_aes_256_cfb1();
								break;

							case ENC_MODE_CFB8:
								evpCipher = EVP_aes_256_cfb8();
								break;

							case ENC_MODE_ECB:
								evpCipher = EVP_aes_256_ecb();
								break;

							case ENC_MODE_OFB:
								evpCipher = EVP_aes_256_ofb();
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

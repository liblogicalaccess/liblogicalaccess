#include <vector>
#include "boost/shared_ptr.hpp"
#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/cmac.hpp"
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"

namespace logicalaccess
{
	namespace openssl
	{
		std::vector<unsigned char> CMACCrypto::cmac(const std::vector<unsigned char>& key, boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipherMAC, unsigned int block_size, const std::vector<unsigned char>& data, std::vector<unsigned char> lastIV)
		{
			boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipherK1K2;

			boost::shared_ptr<openssl::SymmetricKey> symkey;
			boost::shared_ptr<openssl::InitializationVector> iv;
			// 3DES
			if (boost::dynamic_pointer_cast<openssl::DESCipher>(cipherMAC))
			{
				cipherK1K2.reset(new openssl::DESCipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_ECB));

				symkey.reset(new openssl::DESSymmetricKey(openssl::DESSymmetricKey::createFromData(key)));
				iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createNull()));
			}
			// AES
			else
			{
				cipherK1K2.reset(new openssl::AESCipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_ECB));

				symkey.reset(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(key)));
				iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createNull()));
			}

			unsigned char Rb;

			// TDES
			if (block_size == 8)
			{
				Rb = 0x1b;
			}
			// AES
			else
			{
				Rb = 0x87;
			}

			std::vector<unsigned char> blankbuf;
			blankbuf.resize(block_size, 0x00);
			std::vector<unsigned char> L;
			cipherK1K2->cipher(blankbuf, L, *symkey.get(), *iv.get(), false);

			std::vector<unsigned char> K1;
			if ((L[0] & 0x80) == 0x00)
			{
				K1 = openssl::CMACCrypto::shift_string(L);
			}
			else
			{
				K1 = openssl::CMACCrypto::shift_string(L, Rb);
			}

			std::vector<unsigned char> K2;
			if ((K1[0] & 0x80) == 0x00)
			{
				K2 = openssl::CMACCrypto::shift_string(K1);
			}
			else
			{
				K2 = openssl::CMACCrypto::shift_string(K1, Rb);
			}

			int pad = (block_size - (data.size() % block_size)) % block_size;
			std::vector<unsigned char> padded_data = data;
			if (pad > 0)
			{
				padded_data.push_back(0x80);
				if (pad > 1)
				{
					for (int i = 0; i < (pad - 1); ++i)
					{
						padded_data.push_back(0x00);
					}
				}
			}

			// XOR with K1
			if (pad == 0)
			{
				for (unsigned int i = 0; i < K1.size(); ++i)
				{
					padded_data[padded_data.size() - K1.size() + i] = static_cast<unsigned char>(padded_data[padded_data.size() - K1.size() + i] ^ K1[i]);
				}
			}
			// XOR with K2
			else
			{
				for (unsigned int i = 0; i < K2.size(); ++i)
				{
					padded_data[padded_data.size() - K2.size() + i] = static_cast<unsigned char>(padded_data[padded_data.size() - K2.size() + i] ^ K2[i]);
				}
			}

			std::vector<unsigned char> ret;
			// 3DES
			if (boost::dynamic_pointer_cast<openssl::DESCipher>(cipherMAC))
			{
				iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createFromData(lastIV)));
			}
			// AES
			else
			{
				iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(lastIV)));
			}
			cipherMAC->cipher(padded_data, ret, *symkey.get(), *iv.get(), false);

			return ret;
		}

		std::vector<unsigned char> CMACCrypto::shift_string(const std::vector<unsigned char>& buf, unsigned char xorparam)
		{
			std::vector<unsigned char> ret = buf;

			unsigned int i;
			for (i = 0; i < ret.size() - 1; ++i)
			{
				ret[i] = ret[i] << 1;
				// add the carry over bit
				ret[i] = ret[i] | ((ret[i + 1] & 0x80) ? 0x01 : 0x00);
			}

			ret[ret.size()-1] = ret[ret.size()-1] << 1;

			if (xorparam != 0x00)
			{
				ret[ret.size()-1] = static_cast<unsigned char>(ret[ret.size()-1] ^ xorparam);
			}

			return ret;
		}
	}
}

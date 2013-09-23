#include "desfirekey.hpp"
#include "logicalaccess/logs.hpp"
#include "nxpkeydiversification.hpp"
#include "boost/shared_ptr.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "desfirecrypto.hpp"
#include <vector>

namespace logicalaccess
{
	bool NXPKeyDiversification::initDiversification(unsigned char *diversify, std::vector<unsigned char> identifier, unsigned char *AID)
	{
	//div_constant = BitArray(hex='01')
	//uid = BitArray(hex='04782E21801D80')
	//aid = BitArray(hex='3042F5')
	//sysid = BitArray(hex='4E585020416275')
 
	//m = BitArray().join([uid, aid, sysid])
	//d = BitArray().join([div_constant, m])
 
		return true;
	}

	std::vector<unsigned char> NXPKeyDiversification::XORVector(std::vector<unsigned char> str1, std::vector<unsigned char> str2)
	{
		std::vector<unsigned char> ret;

		for (unsigned int i = 0; i < str1.size(); ++i)
			ret.push_back(str1[i] ^ str2[i]);
		return ret;
	}

	std::vector<unsigned char> NXPKeyDiversification::getKeyDiversificated(boost::shared_ptr<Key> key, unsigned char* diversify)
	{
		INFO_("Using key diversification NXP with div : %s", BufferHelper::getHex(std::vector<unsigned char>(diversify, diversify + 16)).c_str());
		std::vector<unsigned char> keydiv;
		std::vector<unsigned char> keycipher(key->getData(), key->getData() + key->getLength());

		std::vector<unsigned char> empty, const_rb;
		std::vector<unsigned char> k0, k1, k2, ktmp;
		
		empty.resize(16);
		const_rb.resize(16);
		const_rb[15] = 0x80;

		boost::shared_ptr<openssl::OpenSSLSymmetricCipher> d_cipher(new openssl::AESCipher());
		openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(keycipher);
		openssl::AESInitializationVector iv = openssl::AESInitializationVector::createNull();


		d_cipher->cipher(empty, k0, aeskey, iv, false);
		//k1
		if (k0[2] == '0' && k0[3] == '0')
			std::copy(k0.begin() + 1, k0.end(), k1.end());
		else
		{
			ktmp.resize(16);
			std::copy(k0.begin() + 1, k0.end(), ktmp.begin());
			k1 = XORVector(ktmp, const_rb);
		}
		//k2
		if (k1[2] == '0' && k1[3] == '0')
			std::copy(k1.begin() + 1, k1.end(), k2.end());
		else
		{
			ktmp.clear();
			ktmp.resize(16);
			std::copy(k1.begin() + 1, k1.end(), ktmp.begin());
			k2 = XORVector(ktmp, const_rb);
		}


		return keydiv;
	}
}
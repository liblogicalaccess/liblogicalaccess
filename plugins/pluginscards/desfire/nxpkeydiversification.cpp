#include "desfirekey.hpp"
#include "logicalaccess/logs.hpp"
#include "nxpkeydiversification.hpp"
#include "boost/shared_ptr.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/CMAC.hpp"
#include "desfirecrypto.hpp"
#include <vector>

namespace logicalaccess
{
	void NXPKeyDiversification::initDiversification(std::vector<unsigned char> identifier, int AID, boost::shared_ptr<Key> key, std::vector<unsigned char>& diversify)
	{
		std::string const_id;

		if (identifier.size() != 0)
		{
			if (d_systemidentifier == "")
				const_id = "NXP Abu";
			else
				const_id = d_systemidentifier;

			diversify.push_back(0x01);
			for (unsigned int x = 0; x < identifier.size(); ++x)
				diversify.push_back(identifier[x]);
			
			unsigned char aidTab[3];
			for (char x = 2; x >= 0; --x)
			{
				aidTab[x] = AID & 0xff;
				AID >>= 8;
			}
			for (char x = 0; x < 3; ++x)
				diversify.push_back(aidTab[x]);

			std::vector<unsigned char> const_vector = std::vector<unsigned char>(const_id.begin(), const_id.end());
			if (boost::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DESFireKeyType::DF_KEY_AES)
				diversify.insert(diversify.end(), const_vector.begin(), const_vector.begin() + 7);
			else
				diversify.insert(diversify.end(), const_vector.begin(), const_vector.begin() + 5);
		}
		else
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Chip identifier is required for key diversification.");
		}
	}

	std::vector<unsigned char> NXPKeyDiversification::getDiversifiedKey(boost::shared_ptr<Key> key, std::vector<unsigned char> diversify)
	{
		INFO_("Using key diversification NXP with div : %s", BufferHelper::getHex(diversify).c_str());
		int block_size = 0;
		boost::shared_ptr<openssl::OpenSSLSymmetricCipher> d_cipher;
		std::vector<unsigned char> keycipher(key->getData(), key->getData() + key->getLength());
		std::vector<unsigned char> emptyIV, keydiv;

		if (boost::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DESFireKeyType::DF_KEY_DES)
		{
			d_cipher.reset(new openssl::DESCipher());
			block_size = 8;
		}
		else if (boost::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DESFireKeyType::DF_KEY_AES)
		{
			d_cipher.reset(new openssl::AESCipher());
			block_size = 16;
		}
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "NXP Diversification don't support this security");

		emptyIV.resize(block_size);
		if (boost::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DESFireKeyType::DF_KEY_AES)
		{
			std::vector<unsigned char> keydiv_tmp;
			keydiv_tmp = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV);
			keydiv.resize(16);
			std::copy(keydiv_tmp.end() - 16, keydiv_tmp.end(), keydiv.begin());
		}
		else
		{
			std::vector<unsigned char> keydiv_tmp_1, keydiv_tmp_2;
			diversify[0] = 0x21;
			keydiv_tmp_1 = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV);
			diversify[0] = 0x22;
			keydiv_tmp_2 = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV);
			keydiv.insert(keydiv.end(), keydiv_tmp_1.begin() + 8, keydiv_tmp_1.end());
			keydiv.insert(keydiv.end(), keydiv_tmp_2.begin() + 8, keydiv_tmp_2.end());
		}
		return keydiv;
	}

	
	void NXPKeyDiversification::serialize(boost::property_tree::ptree& parentNode)
	{
		parentNode.put("<xmlattr>.keyDiversificationType", getType());
		parentNode.put("<xmlattr>.SystemIdentifier", d_systemidentifier);
	}

	void NXPKeyDiversification::unSerialize(boost::property_tree::ptree& node)
	{
		d_systemidentifier = node.get_child("<xmlattr>.SystemIdentifier").get_value<std::string>();
	}
}
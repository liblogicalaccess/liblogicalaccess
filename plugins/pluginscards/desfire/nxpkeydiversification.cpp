#include "desfirekey.hpp"
#include "logicalaccess/logs.hpp"
#include "nxpkeydiversification.hpp"
#include "boost/shared_ptr.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/cmac.hpp"
#include "desfirecrypto.hpp"
#include <vector>

namespace logicalaccess
{
	void NXPKeyDiversification::initDiversification(std::vector<unsigned char> identifier, int AID, boost::shared_ptr<Key> key, unsigned char keyno, std::vector<unsigned char>& diversify)
	{
		if (d_divInput.size() == 0)
		{
			if (identifier.size() == 0)
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "NXP Diversification without divInput need an UID.");

			//UID
			//std::reverse(identifier.begin(), identifier.end());
			for (unsigned int x = 0; x < identifier.size(); ++x)
				diversify.push_back(identifier[x]);
			
			//AID
			unsigned char aidTab[3];
			for (unsigned char x = 2; x != (unsigned char)0xff; --x)
			{
				aidTab[x] = AID & 0xff;
				AID >>= 8;
			}
			//std::reverse(aidTab, aidTab + 3);
			for (unsigned char x = 0; x < 3; ++x)
				diversify.push_back(aidTab[x]);

			//keyID
			diversify.push_back(keyno);
		}
		else
		{
			if (d_divInput.size() > 31)
				d_divInput.resize(31);
			//divInput
			diversify.insert(diversify.end(), d_divInput.begin(), d_divInput.end());
		}
	}

	std::vector<unsigned char> NXPKeyDiversification::getDiversifiedKey(boost::shared_ptr<Key> key, std::vector<unsigned char> diversify)
	{
		//const AES 128
		diversify.insert(diversify.begin(), 0x01);

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
			keydiv_tmp = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV, 32);
			keydiv.resize(16);
			std::copy(keydiv_tmp.end() - 16, keydiv_tmp.end(), keydiv.begin());
		}
		else
		{
			std::vector<unsigned char> keydiv_tmp_1, keydiv_tmp_2;
			diversify[0] = 0x21;
			keydiv_tmp_1 = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV, block_size);
			diversify[0] = 0x22;
			keydiv_tmp_2 = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV, block_size);
			keydiv.insert(keydiv.end(), keydiv_tmp_1.begin() + 8, keydiv_tmp_1.end());
			keydiv.insert(keydiv.end(), keydiv_tmp_2.begin() + 8, keydiv_tmp_2.end());
		}
		return keydiv;
	}

	
	void NXPKeyDiversification::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		node.put("divInput", std::string(d_divInput.begin(), d_divInput.end()));
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void NXPKeyDiversification::unSerialize(boost::property_tree::ptree& node)
	{
		std::string divinput = node.get_child("divInput").get_value<std::string>();
		d_divInput = std::vector<unsigned char>(divinput.begin(), divinput.end());
	}
}

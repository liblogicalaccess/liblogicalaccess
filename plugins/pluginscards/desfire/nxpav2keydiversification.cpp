#include "desfirekey.hpp"
#include "logicalaccess/logs.hpp"
#include "nxpav2keydiversification.hpp"
#include <memory>
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/cmac.hpp"
#include "desfirecrypto.hpp"
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    void NXPAV2KeyDiversification::initDiversification(std::vector<unsigned char> identifier, int AID, std::shared_ptr<Key> key, unsigned char keyno, std::vector<unsigned char>& diversify)
    {
        if (d_divInput.size() == 0)
        {
            if (identifier.size() == 0)
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "NXP AV2 Diversification without divInput, need an UID.");

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
			if (d_revertAID)
				std::reverse(aidTab, aidTab + 3);
            for (unsigned char x = 0; x < 3; ++x)
                diversify.push_back(aidTab[x]);

            // If a system identifier is set we use it, otherwise we use key number
            if (d_systemIdentifier.size() > 0)
            {
                // System Identifier
                diversify.insert(diversify.end(), d_systemIdentifier.begin(), d_systemIdentifier.end());
            }
            else
            {
                //keyID
                diversify.push_back(keyno);
            }
        }
        else
        {
            if (d_divInput.size() > 31)
                d_divInput.resize(31);
            //divInput
            diversify.insert(diversify.end(), d_divInput.begin(), d_divInput.end());
        }
    }

    std::vector<unsigned char> NXPAV2KeyDiversification::getDiversifiedKey(std::shared_ptr<Key> key, std::vector<unsigned char> diversify)
    {
        LOG(LogLevel::INFOS) << "Using key diversification NXP AV2 with div : " << BufferHelper::getHex(diversify);
        int block_size = 0;
        std::shared_ptr<openssl::OpenSSLSymmetricCipher> d_cipher;
        std::vector<unsigned char> keycipher(key->getData(), key->getData() + key->getLength());
        std::vector<unsigned char> emptyIV, keydiv;

        if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DESFireKeyType::DF_KEY_DES
            || std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DESFireKeyType::DF_KEY_3K3DES)
        {
            d_cipher.reset(new openssl::DESCipher());
            block_size = 8;
        }
        else if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DESFireKeyType::DF_KEY_AES)
        {
            d_cipher.reset(new openssl::AESCipher());
            block_size = 16;
        }
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "NXP Diversification don't support this security");

        emptyIV.resize(block_size);
        if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DESFireKeyType::DF_KEY_AES)
        {
            //const AES 128
            diversify.insert(diversify.begin(), 0x01);
            std::vector<unsigned char> keydiv_tmp;
            keydiv_tmp = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV, 32, d_forceK2Use);
            keydiv.resize(16);
            std::copy(keydiv_tmp.end() - 16, keydiv_tmp.end(), keydiv.begin());
        }
        else if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DESFireKeyType::DF_KEY_DES)
        {
            std::vector<unsigned char> keydiv_tmp_1, keydiv_tmp_2;
            diversify.insert(diversify.begin(), 0x21);
            keydiv_tmp_1 = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV, 16, d_forceK2Use);
            diversify[0] = 0x22;
            keydiv_tmp_2 = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV, 16, d_forceK2Use);
            keydiv.insert(keydiv.end(), keydiv_tmp_1.begin() + 8, keydiv_tmp_1.end());
            keydiv.insert(keydiv.end(), keydiv_tmp_2.begin() + 8, keydiv_tmp_2.end());
        }
        else if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DESFireKeyType::DF_KEY_3K3DES)
        {
            std::vector<unsigned char> keydiv_tmp_1, keydiv_tmp_2, keydiv_tmp_3;
            diversify.insert(diversify.begin(), 0x31);
            keydiv_tmp_1 = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV, 16, d_forceK2Use);
            diversify[0] = 0x32;
            keydiv_tmp_2 = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV, 16, d_forceK2Use);
            diversify[0] = 0x33;
            keydiv_tmp_3 = openssl::CMACCrypto::cmac(keycipher, d_cipher, block_size, diversify, emptyIV, 16, d_forceK2Use);
            keydiv.insert(keydiv.end(), keydiv_tmp_1.begin() + 8, keydiv_tmp_1.end());
            keydiv.insert(keydiv.end(), keydiv_tmp_2.begin() + 8, keydiv_tmp_2.end());
            keydiv.insert(keydiv.end(), keydiv_tmp_3.begin() + 8, keydiv_tmp_3.end());
        }
        return keydiv;
    }

    void NXPAV2KeyDiversification::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        node.put("divInput", BufferHelper::getHex(d_divInput));
        node.put("systemIdentifier", BufferHelper::getHex(d_systemIdentifier));
		node.put("revertAID", d_revertAID);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void NXPAV2KeyDiversification::unSerialize(boost::property_tree::ptree& node)
    {
		d_revertAID = node.get_child("revertAID").get_value<bool>();
        std::string divinput = node.get_child("divInput").get_value<std::string>();
        d_divInput = BufferHelper::fromHexString(divinput);
        boost::optional<boost::property_tree::ptree&> siChild = node.get_child_optional("systemIdentifier");
        if (siChild)
        {
            std::string systemIdentifier = siChild.get().get_value<std::string>();
            d_systemIdentifier = BufferHelper::fromHexString(systemIdentifier);
        }
    }
}
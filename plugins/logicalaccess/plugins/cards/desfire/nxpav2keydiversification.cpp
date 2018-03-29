#include <logicalaccess/plugins/cards/desfire/desfirekey.hpp>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpav2keydiversification.hpp>
#include <memory>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecrypto.hpp>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
void NXPAV2KeyDiversification::initDiversification(ByteVector identifier,
                                                   unsigned int AID,
                                                   std::shared_ptr<Key> /*key*/,
                                                   unsigned char keyno,
                                                   ByteVector &diversify)
{
    if (d_divInput.size() == 0)
    {
        if (identifier.size() == 0)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "NXP AV2 Diversification without divInput, need an UID.");

        // UID
        // std::reverse(identifier.begin(), identifier.end());
        for (unsigned int x = 0; x < identifier.size(); ++x)
            diversify.push_back(identifier[x]);

        // AID
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
            diversify.insert(diversify.end(), d_systemIdentifier.begin(),
                             d_systemIdentifier.end());
        }
        else
        {
            // keyID
            diversify.push_back(keyno);
        }
    }
    else
    {
        if (d_divInput.size() > 31)
            d_divInput.resize(31);
        // divInput
        diversify.insert(diversify.end(), d_divInput.begin(), d_divInput.end());
    }
}

ByteVector NXPAV2KeyDiversification::getDiversifiedKey(std::shared_ptr<Key> key,
                                                       ByteVector diversify)
{
    LOG(LogLevel::INFOS) << "Using key diversification NXP AV2 with div : "
                         << BufferHelper::getHex(diversify);
    int block_size;
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> d_cipher;
    ByteVector keycipher(key->getData(), key->getData() + key->getLength());
    ByteVector emptyIV, keydiv;

    if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DF_KEY_DES ||
        std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DF_KEY_3K3DES)
    {
        d_cipher.reset(new openssl::DESCipher());
        block_size = 8;
    }
    else if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DF_KEY_AES)
    {
        d_cipher.reset(new openssl::AESCipher());
        block_size = 16;
    }
    else
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "NXP Diversification don't support this security");

    emptyIV.resize(block_size);
    if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DF_KEY_AES)
    {
        // const AES 128
        diversify.insert(diversify.begin(), 0x01);
        ByteVector keydiv_tmp = openssl::CMACCrypto::cmac(
            keycipher, d_cipher, block_size, diversify, emptyIV, 32, d_forceK2Use);
        keydiv.resize(16);
        copy(keydiv_tmp.end() - 16, keydiv_tmp.end(), keydiv.begin());
    }
    else if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DF_KEY_DES)
    {
        diversify.insert(diversify.begin(), 0x21);
        ByteVector keydiv_tmp_1 = openssl::CMACCrypto::cmac(
            keycipher, d_cipher, block_size, diversify, emptyIV, 16, d_forceK2Use);
        diversify[0]            = 0x22;
        ByteVector keydiv_tmp_2 = openssl::CMACCrypto::cmac(
            keycipher, d_cipher, block_size, diversify, emptyIV, 16, d_forceK2Use);
        keydiv.insert(keydiv.end(), keydiv_tmp_1.begin() + 8, keydiv_tmp_1.end());
        keydiv.insert(keydiv.end(), keydiv_tmp_2.begin() + 8, keydiv_tmp_2.end());
    }
    else if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DF_KEY_3K3DES)
    {
        diversify.insert(diversify.begin(), 0x31);
        ByteVector keydiv_tmp_1 = openssl::CMACCrypto::cmac(
            keycipher, d_cipher, block_size, diversify, emptyIV, 16, d_forceK2Use);
        diversify[0]            = 0x32;
        ByteVector keydiv_tmp_2 = openssl::CMACCrypto::cmac(
            keycipher, d_cipher, block_size, diversify, emptyIV, 16, d_forceK2Use);
        diversify[0]            = 0x33;
        ByteVector keydiv_tmp_3 = openssl::CMACCrypto::cmac(
            keycipher, d_cipher, block_size, diversify, emptyIV, 16, d_forceK2Use);
        keydiv.insert(keydiv.end(), keydiv_tmp_1.begin() + 8, keydiv_tmp_1.end());
        keydiv.insert(keydiv.end(), keydiv_tmp_2.begin() + 8, keydiv_tmp_2.end());
        keydiv.insert(keydiv.end(), keydiv_tmp_3.begin() + 8, keydiv_tmp_3.end());
    }
    return keydiv;
}

void NXPAV2KeyDiversification::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    node.put("divInput", BufferHelper::getHex(d_divInput));
    node.put("systemIdentifier", BufferHelper::getHex(d_systemIdentifier));
    node.put("revertAID", d_revertAID);
    node.put("forceK2Use", d_forceK2Use);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void NXPAV2KeyDiversification::unSerialize(boost::property_tree::ptree &node)
{
    d_revertAID          = node.get_child("revertAID").get_value<bool>();
    std::string divinput = node.get_child("divInput").get_value<std::string>();
    d_divInput           = BufferHelper::fromHexString(divinput);
    boost::optional<boost::property_tree::ptree &> siChild =
        node.get_child_optional("systemIdentifier");
    if (siChild)
    {
        std::string systemIdentifier = siChild.get().get_value<std::string>();
        d_systemIdentifier           = BufferHelper::fromHexString(systemIdentifier);
    }
    boost::optional<boost::property_tree::ptree &> fChild =
        node.get_child_optional("forceK2Use");
    if (fChild)
    {
        d_forceK2Use = fChild.get().get_value<bool>();
    }
}
}
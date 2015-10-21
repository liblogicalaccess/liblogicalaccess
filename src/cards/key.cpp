/**
 * \file key.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Key.
 */

#include "logicalaccess/key.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

#include "logicalaccess/logs.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/sha.hpp"
#include "logicalaccess/bufferhelper.hpp"

#include <openssl/rand.h>
#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/cards/keydiversification.hpp"

namespace logicalaccess
{
    const std::string Key::secureAiKey = "Obscurity is not security Julien would say. But...";

    Key::Key()
    {
        d_isEmpty = true;
        d_key_storage.reset(new ComputerMemoryKeyStorage());
        d_cipherKey = "";
        d_storeCipheredData = true;
    }

    void Key::clear()
    {
        if (getLength() > 0)
        {
            memset(getData(), 0x00, getLength());
        }
    }

    std::string Key::toString(bool withSpace) const
    {
        const unsigned char* data = getData();
        std::ostringstream oss;

        oss << std::setfill('0');

        if (!d_isEmpty)
        {
            for (size_t i = 0; i < getLength(); ++i)
            {
                oss << std::setw(2) << std::hex << static_cast<unsigned int>(data[i]);

                if (withSpace && i < (getLength() - 1))
                {
                    oss << " ";
                }
            }
        }

        return oss.str();
    }

    bool Key::fromString(const std::string& str)
    {
        unsigned char* data = getData();
        std::istringstream iss(str);

        if (str == "")
        {
            d_isEmpty = true;
        }
        else
        {
            for (size_t i = 0; i < getLength(); ++i)
            {
                unsigned int tmp;

                if (!iss.good())
                {
                    d_isEmpty = true;

                    return false;
                }

                iss >> std::hex >> tmp;

                data[i] = static_cast<unsigned char>(tmp);
            }

            d_isEmpty = false;
        }

        return true;
    }

    void Key::setData(const unsigned char* data)
    {
        memcpy(getData(), data, getLength());
        d_isEmpty = false;
    }

    void Key::setData(const std::vector<unsigned char>& data, size_t offset)
    {
        if (data.size() == 0)
        {
            d_isEmpty = true;
        }
        else
        {
            memcpy(getData(), &data[offset], getLength());
            d_isEmpty = false;
        }
    }

    void Key::setKeyStorage(std::shared_ptr<KeyStorage> key_storage)
    {
        d_key_storage = key_storage;
    }

    std::shared_ptr<KeyStorage> Key::getKeyStorage() const
    {
        return d_key_storage;
    }

    void Key::serialize(boost::property_tree::ptree& node)
    {
        node.put("<xmlattr>.keyStorageType", d_key_storage->getType());
        if (d_key_diversification)
        {
            boost::property_tree::ptree newnode;
            newnode.put("<xmlattr>.keyDiversificationType", d_key_diversification->getType());
            d_key_diversification->serialize(newnode);
            node.add_child("KeyDiversification", newnode);
        }
        node.put("IsCiphered", (d_storeCipheredData && !d_isEmpty));
        cipherKeyData(node);
        d_key_storage->serialize(node);
    }

    void Key::unSerialize(boost::property_tree::ptree& node)
    {
        LOG(LogLevel::INFOS) << "Unserializing Key...";

        if (node.get_child_optional("KeyDiversification"))
        {
            boost::property_tree::ptree keydivnode = node.get_child("KeyDiversification");
            d_key_diversification = KeyDiversification::getKeyDiversificationFromType(keydivnode.get_child("<xmlattr>.keyDiversificationType").get_value<std::string>());
            boost::property_tree::ptree kdnode = keydivnode.get_child(d_key_diversification->getDefaultXmlNodeName());
            if (!kdnode.empty())
            {
                d_key_diversification->unSerialize(kdnode);
            }
        }

        d_storeCipheredData = node.get_child("IsCiphered").get_value<bool>(false);
        uncipherKeyData(node);
        LOG(LogLevel::INFOS) << "Unserializing Key storage...";
        d_key_storage = KeyStorage::getKeyStorageFromType(static_cast<KeyStorageType>(node.get_child("<xmlattr>.keyStorageType").get_value<unsigned int>()));
        if (d_key_storage)
        {
            boost::property_tree::ptree ksnode = node.get_child(d_key_storage->getDefaultXmlNodeName());
            if (!ksnode.empty())
            {
                d_key_storage->unSerialize(ksnode);
            }
        }
    }

    void Key::setStoreCipheredData(bool cipher)
    {
        d_storeCipheredData = cipher;
    }

    bool Key::getStoreCipheredData()
    {
        return d_storeCipheredData;
    }

    void Key::setCipherKey(const std::string& key)
    {
        d_cipherKey = key;
    }

    void Key::cipherKeyData(boost::property_tree::ptree& node)
    {
        if (!d_storeCipheredData || d_isEmpty)
        {
            node.put("Data", toString());
        }
        else
        {
            std::string secureKey = ((d_cipherKey == "") ? Key::secureAiKey : d_cipherKey);
            openssl::AESSymmetricKey aes = openssl::AESSymmetricKey::createFromPassphrase(secureKey);
            openssl::AESInitializationVector iv = openssl::AESInitializationVector::createNull();
            openssl::AESCipher aescipher;

            std::vector<unsigned char> divaesbuf;
            std::string strdata = "Data";
            std::vector<unsigned char> keynamebuf = std::vector<unsigned char>(strdata.begin(), strdata.end());
            keynamebuf.resize(32, 0x00);
            aescipher.cipher(keynamebuf, divaesbuf, aes, iv, false);
            openssl::AESSymmetricKey divaes = openssl::AESSymmetricKey::createFromData(divaesbuf);

            strdata = toString();
            std::vector<unsigned char> keybuf = std::vector<unsigned char>(strdata.begin(), strdata.end());
            std::vector<unsigned char> cipheredkey;
            aescipher.cipher(keybuf, cipheredkey, divaes, iv, true);

            node.put("Data", BufferHelper::toBase64(cipheredkey));
        }
    }

    void Key::uncipherKeyData(boost::property_tree::ptree& node)
    {
        std::string data = node.get_child("Data").get_value<std::string>();
        //LOG(LogLevel::INFOS) << "Unciphering data... {%s}", data.c_str());

        if (!d_storeCipheredData)
        {
            //LOG(LogLevel::INFOS) << "Data was not ciphered ! Retrieving directly data...");
            fromString(data);
        }
        else
        {
            LOG(LogLevel::INFOS) << "Data was ciphered ! Unciphering..";
            std::string secureKey = ((d_cipherKey == "") ? Key::secureAiKey : d_cipherKey);
            std::vector<unsigned char> hash = openssl::SHA256Hash(secureKey);
            openssl::AESSymmetricKey aes = openssl::AESSymmetricKey::createFromData(hash);
            openssl::AESInitializationVector iv = openssl::AESInitializationVector::createNull();
            openssl::AESCipher aescipher;

            std::vector<unsigned char> divaesbuf;
            std::string strdata = "Data";
            std::vector<unsigned char> keynamebuf = std::vector<unsigned char>(strdata.begin(), strdata.end());
            keynamebuf.resize(32, 0x00);
            aescipher.cipher(keynamebuf, divaesbuf, aes, iv, false);
            openssl::AESSymmetricKey divaes = openssl::AESSymmetricKey::createFromData(divaesbuf);

            std::vector<unsigned char> keybuf(data.begin(), data.end()), uncipheredkey;
            aescipher.decipher(BufferHelper::fromBase64(BufferHelper::getStdString(keybuf)), uncipheredkey, divaes, iv, true);

            //LOG(LogLevel::DEBUGS) << "Data unciphered: {%s}", uncipheredkey.toStdString().c_str());

            fromString(BufferHelper::getStdString(uncipheredkey));
        }
    }

    bool Key::operator==(const Key& key) const
    {
        if (isEmpty() && key.isEmpty())
        {
            return true;
        }

        if (getLength() != key.getLength() || isEmpty() || key.isEmpty())
        {
            return false;
        }

        return (memcmp(getData(), key.getData(), getLength()) == 0);
    }

    bool Key::operator!=(const Key& key) const
    {
        if (isEmpty() && key.isEmpty())
        {
            return false;
        }

        if (getLength() != key.getLength() || isEmpty() || key.isEmpty())
        {
            return true;
        }

        return (memcmp(getData(), key.getData(), getLength()) != 0);
    }

    std::ostream& operator<<(std::ostream& os, const Key& key)
    {
        return os << key.toString();
    }

std::shared_ptr<KeyDiversification> Key::getKeyDiversification()
{ return d_key_diversification; }

void Key::setKeyDiversification(std::shared_ptr<KeyDiversification> div)
{ d_key_diversification = div; }
}
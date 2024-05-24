/**
 * \file key.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime@leosac.com>
 * \brief Key.
 */

#include <logicalaccess/key.hpp>
#include <logicalaccess/cards/computermemorykeystorage.hpp>
#include <logicalaccess/cards/accessinfo.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/sha.hpp>
#include <logicalaccess/bufferhelper.hpp>

#include <openssl/rand.h>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/cards/keydiversification.hpp>

namespace logicalaccess
{
const std::string Key::secureAiKey = "Obscurity is not security Julien would say. But...";

Key::Key()
{
    d_key_storage.reset(new ComputerMemoryKeyStorage());
    d_cipherKey         = "";
    d_storeCipheredData = true;
}

void Key::clear()
{
    d_data.clear();
}

bool Key::isEmpty() const
{
    return (d_data.size() == 0 && d_key_storage->getType() == KST_COMPUTER_MEMORY);
}

std::string Key::getString(bool withSpace) const
{
    std::ostringstream oss;

    oss << std::setfill('0');

    if (!isEmpty())
    {
        ByteVector data = getData();
        for (size_t i = 0; i < data.size(); ++i)
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

bool Key::fromString(const std::string &str)
{
    std::istringstream iss(str);
    clear();
    if (str != "")
    {
        unsigned int tmp;
        for (size_t i = 0; i < getLength(); ++i)
        {
            if (!iss.good())
            {
                return false;
            }

            iss >> std::hex >> tmp;
            d_data.push_back(static_cast<unsigned char>(tmp));
        }
    }

    return true;
}

void Key::setData(const void *buf, size_t buflen)
{
    clear();
    if (buf != nullptr)
    {
        size_t end = buflen;
        if (getLength() > 0)
        {
            end = getLength();
            if (buflen < end)
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Wrong key data length.");
            }
        }
        d_data.insert(d_data.end(), reinterpret_cast<const unsigned char *>(buf), reinterpret_cast<const unsigned char *>(buf) + end);
    }
}

void Key::setData(const ByteVector &data)
{
    clear();
    size_t end = data.size();
    if (getLength() > 0)
    {
        end = getLength();
        if (data.size() < end)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Wrong key data length.");
        }
    }
    d_data.insert(d_data.begin(), data.begin(), data.begin() + end);
}

ByteVector Key::getData() const
{
    ByteVector data = d_data;
    if (getLength() > 0 && data.size() != getLength())
    {
        data.resize(getLength(), getEmptyByte());
    }
    return data;
}

void Key::setKeyStorage(std::shared_ptr<KeyStorage> key_storage)
{
    d_key_storage = key_storage;
}

std::shared_ptr<KeyStorage> Key::getKeyStorage() const
{
    return d_key_storage;
}

void Key::serialize(boost::property_tree::ptree &node)
{
    node.put("<xmlattr>.keyStorageType", d_key_storage->getType());
    if (d_key_diversification)
    {
        boost::property_tree::ptree newnode;
        newnode.put("<xmlattr>.keyDiversificationType",
                    d_key_diversification->getKeyDiversificationType());
        d_key_diversification->serialize(newnode);
        node.add_child("KeyDiversification", newnode);
    }
    node.put("IsCiphered", (d_storeCipheredData && !isEmpty()));
    cipherKeyData(node);
    d_key_storage->serialize(node);
}

void Key::unSerialize(boost::property_tree::ptree &node)
{
    LOG(LogLevel::INFOS) << "Unserializing Key...";

    if (node.get_child_optional("KeyDiversification"))
    {
        boost::property_tree::ptree keydivnode = node.get_child("KeyDiversification");
        d_key_diversification = KeyDiversification::getKeyDiversificationFromType(
            keydivnode.get_child("<xmlattr>.keyDiversificationType")
                .get_value<std::string>());
        boost::property_tree::ptree kdnode =
            keydivnode.get_child(d_key_diversification->getDefaultXmlNodeName());
        if (!kdnode.empty())
        {
            d_key_diversification->unSerialize(kdnode);
        }
    }

    d_storeCipheredData = node.get_child("IsCiphered").get_value<bool>(false);
    uncipherKeyData(node);
    LOG(LogLevel::INFOS) << "Unserializing Key storage...";
    d_key_storage = KeyStorage::getKeyStorageFromType(static_cast<KeyStorageType>(
        node.get_child("<xmlattr>.keyStorageType").get_value<unsigned int>()));
    if (d_key_storage)
    {
        boost::property_tree::ptree ksnode =
            node.get_child(d_key_storage->getDefaultXmlNodeName());
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

bool Key::getStoreCipheredData() const
{
    return d_storeCipheredData;
}

void Key::setCipherKey(const std::string &key)
{
    d_cipherKey = key;
}

void Key::cipherKeyData(boost::property_tree::ptree &node)
{
    if (!d_storeCipheredData || isEmpty())
    {
        node.put("Data", getString());
    }
    else
    {
        std::string secureKey = ((d_cipherKey == "") ? secureAiKey : d_cipherKey);
        openssl::AESSymmetricKey aes =
            openssl::AESSymmetricKey::createFromPassphrase(secureKey);
        openssl::AESInitializationVector iv =
            openssl::AESInitializationVector::createNull();
        openssl::AESCipher aescipher;

        ByteVector divaesbuf;
        std::string strdata   = "Data";
        ByteVector keynamebuf = ByteVector(strdata.begin(), strdata.end());
        keynamebuf.resize(32, 0x00);
        aescipher.cipher(keynamebuf, divaesbuf, aes, iv, false);
        openssl::AESSymmetricKey divaes =
            openssl::AESSymmetricKey::createFromData(divaesbuf);

        strdata           = getString();
        ByteVector keybuf = ByteVector(strdata.begin(), strdata.end());
        ByteVector cipheredkey;
        aescipher.cipher(keybuf, cipheredkey, divaes, iv, true);

        node.put("Data", BufferHelper::toBase64(cipheredkey));
    }
}

void Key::uncipherKeyData(boost::property_tree::ptree &node)
{
    std::string data = node.get_child("Data").get_value<std::string>();
    // LOG(LogLevel::INFOS) << "Unciphering data... {%s}", data.c_str());

    if (!d_storeCipheredData)
    {
        // LOG(LogLevel::INFOS) << "Data was not ciphered ! Retrieving directly data...");
        fromString(data);
    }
    else
    {
        LOG(LogLevel::INFOS) << "Data was ciphered ! Unciphering..";
        std::string secureKey        = ((d_cipherKey == "") ? secureAiKey : d_cipherKey);
        ByteVector hash              = openssl::SHA256Hash(secureKey);
        openssl::AESSymmetricKey aes = openssl::AESSymmetricKey::createFromData(hash);
        openssl::AESInitializationVector iv =
            openssl::AESInitializationVector::createNull();
        openssl::AESCipher aescipher;

        ByteVector divaesbuf;
        std::string strdata   = "Data";
        ByteVector keynamebuf = ByteVector(strdata.begin(), strdata.end());
        keynamebuf.resize(32, 0x00);
        aescipher.cipher(keynamebuf, divaesbuf, aes, iv, false);
        openssl::AESSymmetricKey divaes =
            openssl::AESSymmetricKey::createFromData(divaesbuf);

        ByteVector keybuf(data.begin(), data.end()), uncipheredkey;
        aescipher.decipher(BufferHelper::fromBase64(BufferHelper::getStdString(keybuf)),
                           uncipheredkey, divaes, iv, true);

        // LOG(LogLevel::DEBUGS) << "Data unciphered: {%s}",
        // uncipheredkey.toStdString().c_str());

        fromString(BufferHelper::getStdString(uncipheredkey));
    }
}

bool Key::isEqual(const Key &key) const
{
    if (isEmpty() && key.isEmpty())
    {
        return true;
    }

    if (getLength() != key.getLength() || isEmpty() || key.isEmpty())
    {
        return false;
    }

    return std::equal(getData().begin(), getData().end(), key.getData().begin());
}

bool Key::operator==(const Key &key) const
{
    return isEqual(key);
}

bool Key::operator!=(const Key &key) const
{
    return !isEqual(key);
}

std::ostream &operator<<(std::ostream &os, const Key &key)
{
    return os << key.getString();
}

std::shared_ptr<KeyDiversification> Key::getKeyDiversification() const
{
    return d_key_diversification;
}

void Key::setKeyDiversification(std::shared_ptr<KeyDiversification> div)
{
    d_key_diversification = div;
}
}
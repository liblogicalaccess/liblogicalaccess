/**
 * \file sambasickeyentry.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMBasicKeyEntry source.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/cards/samav/sambasickeyentry.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
SAMBasicKeyEntry::SAMBasicKeyEntry()
    : d_updatemask(0)
{
    d_keyType = SAM_KEY_DES;
    d_key     = new unsigned char[getLength()];
    memset(&*d_key, 0, getLength());
    d_diversify  = false;
    d_updatemask = 0;
}

SAMBasicKeyEntry::SAMBasicKeyEntry(const SAMBasicKeyEntry &copy)
    : d_key(copy.d_key)
    , d_diversify(copy.d_diversify)
    , d_keyType(copy.d_keyType)
    , d_updatemask(copy.d_updatemask)
{
}

SAMBasicKeyEntry::SAMBasicKeyEntry(const std::string &str, const std::string &str1,
                                   const std::string &str2)
    : d_updatemask(0)
{
    d_keyType = SAM_KEY_DES;
    d_key     = new unsigned char[getLength()];
    memset(&*d_key, 0, getLength());
    d_diversify  = false;
    d_updatemask = 0;
    if (str != "")
        memcpy(d_key, str.c_str(), getSingleLength());
    if (str1 != "")
        memcpy(d_key, str1.c_str(), getSingleLength());
    if (str2 != "")
        memcpy(d_key, str2.c_str(), getSingleLength());
}

SAMBasicKeyEntry::SAMBasicKeyEntry(const void **buf, size_t buflen, char numberkey)
    : d_updatemask(0)
{
    d_keyType = SAM_KEY_DES;
    d_key     = new unsigned char[getLength()];
    memset(&*d_key, 0, getLength());
    d_diversify  = false;
    d_updatemask = 0;
    if (buf != nullptr)
    {
        if (buflen * numberkey >= getLength())
        {
            if (numberkey >= 1)
            {
                memcpy(d_key, buf[0], getSingleLength());
            }
            if (numberkey >= 2)
            {
                memcpy(d_key + getSingleLength(), buf[1], getSingleLength());
            }
            if (numberkey >= 3)
            {
                memcpy(d_key + (getSingleLength() * 2), buf[2], getSingleLength());
            }
        }
    }
}

SAMBasicKeyEntry::~SAMBasicKeyEntry()
{
    delete[] d_key;
}

size_t SAMBasicKeyEntry::getSingleLength() const
{
    size_t length = 0;

    switch (d_keyType)
    {
    case SAM_KEY_DES: length = SAM_DES_KEY_SIZE; break;

    case SAM_KEY_3K3DES: length = SAM_MAXKEY_SIZE; break;

    case SAM_KEY_AES: length = SAM_AES_KEY_SIZE; break;
    }

    return length;
}

size_t SAMBasicKeyEntry::getLength() const
{
    size_t length = 0;

    switch (d_keyType)
    {
    case SAM_KEY_DES: length = SAM_DES_KEY_SIZE * 3; break;

    case SAM_KEY_3K3DES: length = SAM_MAXKEY_SIZE * 2; break;

    case SAM_KEY_AES: length = SAM_AES_KEY_SIZE * 3; break;
    }

    return length;
}

std::vector<ByteVector> SAMBasicKeyEntry::getKeysData() const
{
    std::vector<ByteVector> ret;
    size_t keysize      = getSingleLength();
    unsigned char keynb = 3;

    if (d_keyType == SAM_MAXKEY_SIZE)
        keynb = 2;

    for (unsigned char x = 0; x < keynb; ++x)
        ret.push_back(ByteVector(d_key + (x * keysize), d_key + (x * keysize) + keysize));
    return ret;
}

void SAMBasicKeyEntry::setKeysData(std::vector<ByteVector> keys, SAMKeyType type)
{
    if (keys.size() == 0)
        return;
    unsigned char keynb = 3;
    d_keyType           = type;

    if (d_keyType == SAM_KEY_3K3DES)
        keynb = 2;

    size_t keysize = getSingleLength();

    delete[] d_key;
    d_key = new unsigned char[getLength()];
    for (unsigned char x = 0; x < keynb; ++x)
    {
        if (keys[x].size() != keysize)
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "setKey failed because key don't have all the same size.");
        memcpy(d_key + (x * keysize), &keys[x][0], keysize);
    }
}

void SAMBasicKeyEntry::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("KeyType", d_keyType);
    node.put("Diversify", d_diversify);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void SAMBasicKeyEntry::unSerialize(boost::property_tree::ptree &node)
{
    d_keyType =
        static_cast<SAMKeyType>(node.get_child("KeyType").get_value<unsigned int>());
    d_diversify = node.get_child("Diversify").get_value<bool>();
}

std::string SAMBasicKeyEntry::getDefaultXmlNodeName() const
{
    return "SAMKeyEntry";
}

bool SAMBasicKeyEntry::operator==(const SAMBasicKeyEntry &key) const
{
    if (d_keyType != key.d_keyType)
    {
        return false;
    }
    if (memcmp(&*d_key, &*(key.d_key), getLength()) == 0)
    {
        return false;
    }
    return true;
}

std::string SAMBasicKeyEntry::SAMKeyEntryTypeStr(SAMKeyType t)
{
    switch (t)
    {
    case SAM_KEY_DES: return "SAM_KEY_DES";
    case SAM_KEY_3K3DES: return "SAM_KEY_3K3DES";
    case SAM_KEY_AES: return "SAM_KEY_AES";
    default: return "Unknown";
    }
}

KeyEntryUpdateSettings SAMBasicKeyEntry::getUpdateSettings()
{
    KeyEntryUpdateSettings settings;

    bool *x = (bool *)&settings;
    for (unsigned char i = 0; i < sizeof(settings); ++i)
    {
        if ((d_updatemask & 0x80) == 0x80)
            x[i] = true;
        else
            x[i] = false;
        if (i + (unsigned int)1 < sizeof(settings))
            d_updatemask = d_updatemask << 1;
    }

    return settings;
}

void SAMBasicKeyEntry::setUpdateSettings(const KeyEntryUpdateSettings &t)
{
    bool *x      = (bool *)&t;
    d_updatemask = 0;
    for (unsigned char i = 0; i < sizeof(KeyEntryUpdateSettings); ++i)
    {
        d_updatemask += x[i];
        if (i + (unsigned int)1 < sizeof(KeyEntryUpdateSettings))
            d_updatemask = d_updatemask << 1;
    }
}
}
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
    memset(d_key, 0, sizeof(d_key));
    d_diversify  = false;
    d_updatemask = 0;
}

SAMBasicKeyEntry::SAMBasicKeyEntry(const SAMBasicKeyEntry &copy)
    : d_diversify(copy.d_diversify)
    , d_keyType(copy.d_keyType)
    , d_updatemask(copy.d_updatemask)
{
    memcpy(d_key, copy.d_key, sizeof(copy.d_key));
}

SAMBasicKeyEntry::SAMBasicKeyEntry(const std::string &str, const std::string &str1,
                                   const std::string &str2)
    : d_updatemask(0)
{
    d_keyType = SAM_KEY_DES;
    memset(d_key, 0, sizeof(d_key));
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
    memset(d_key, 0, sizeof(d_key));
    d_diversify  = false;
    d_updatemask = 0;
    if (buf != nullptr)
    {
        if (buflen * numberkey >= getLength())
        {
            size_t keysize = getSingleLength();
            for (unsigned char i = 0; i < numberkey; ++i)
            {
                memcpy(d_key + keysize * i, buf[i], keysize);
            }
        }
    }
}

SAMBasicKeyEntry::~SAMBasicKeyEntry()
{
}

size_t SAMBasicKeyEntry::getSingleLength() const
{
    size_t length = 0;

    switch (d_keyType)
    {
	case SAM_KEY_MIFARE:
    case SAM_KEY_DES:
    case SAM_KEY_AES128:
        length = SAM_KEY_SIZE_128;
        break;

    case SAM_KEY_3K3DES:
    case SAM_KEY_AES192:
        length = SAM_KEY_SIZE_192;
        break;

    case SAM_KEY_AES256:
        length = SAM_KEY_SIZE_256;
        break;
    }

    return length;
}

unsigned char SAMBasicKeyEntry::getKeyNb() const
{
    unsigned char keynb = 3;
    size_t keysize      = getSingleLength();
    if (keysize == SAM_KEY_SIZE_192)
        keynb = 2;
    else if (keysize == SAM_KEY_SIZE_256)
        keynb = 1;
    return keynb;
}

size_t SAMBasicKeyEntry::getLength() const
{
    size_t length = 0;

    switch (d_keyType)
    {
    case SAM_KEY_MIFARE:
    case SAM_KEY_DES:
    case SAM_KEY_AES128:
        length = SAM_KEY_SIZE_128 * 3;
        break;

    case SAM_KEY_3K3DES:
    case SAM_KEY_AES192:
        length = SAM_KEY_SIZE_192 * 2;
        break;

    case SAM_KEY_AES256:
        length = SAM_KEY_SIZE_256;
        break;
    }

    return length;
}

std::vector<ByteVector> SAMBasicKeyEntry::getKeysData() const
{
    unsigned char keynb = getKeyNb();
    size_t keysize = getSingleLength();
    std::vector<ByteVector> ret;
    for (unsigned char x = 0; x < keynb; ++x)
        ret.push_back(ByteVector(d_key + (x * keysize), d_key + (x * keysize) + keysize));
    return ret;
}

void SAMBasicKeyEntry::setKeysData(std::vector<ByteVector> keys, SAMKeyType type)
{
    if (keys.size() == 0)
        return;
    d_keyType           = type;
    unsigned char keynb = getKeyNb();
    size_t keysize = getSingleLength();

    memset(d_key, 0, sizeof(d_key));
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
    if (memcmp(d_key, key.d_key, getLength()) == 0)
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
	case SAM_KEY_MIFARE: return "SAM_KEY_MIFARE";
    case SAM_KEY_3K3DES: return "SAM_KEY_3K3DES";
    case SAM_KEY_AES128: return "SAM_KEY_AES128";
	case SAM_KEY_AES192: return "SAM_KEY_AES192";
	case SAM_KEY_AES256: return "SAM_KEY_AES256";
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
    d_updatemask = SAMBasicKeyEntry::getUpdateMask(t);
}

unsigned char SAMBasicKeyEntry::getUpdateMask(const KeyEntryUpdateSettings &t)
{
	bool *x      = (bool *)&t;
    unsigned char updatemask = 0;
    for (unsigned char i = 0; i < sizeof(KeyEntryUpdateSettings); ++i)
    {
        updatemask += x[i];
        if (i + (unsigned int)1 < sizeof(KeyEntryUpdateSettings))
            updatemask = updatemask << 1;
    }
	return updatemask;
}
}
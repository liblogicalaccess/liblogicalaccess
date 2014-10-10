/**
 * \file mifarepluskey.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus Key.
 */

#include "mifarepluskey.hpp"

namespace logicalaccess
{
    MifarePlusKey::MifarePlusKey(size_t keySize)
        : Key()
    {
        InitKey(keySize);
        clear();
    }

    MifarePlusKey::MifarePlusKey(const std::string& str, size_t keySize)
        : Key()
    {
        InitKey(keySize);
        clear();
        fromString(str);
    }

    MifarePlusKey::MifarePlusKey(const void* buf, size_t buflen, size_t keySize)
        : Key()
    {
        InitKey(keySize);
        clear();

        if (buf != NULL)
        {
            if (buflen >= d_keySize)
            {
                memcpy(d_key, buf, d_keySize);
                d_isEmpty = false;
            }
        }
        getLength();
    }

    MifarePlusKey::~MifarePlusKey()
    {
        free(d_key);
    }

    void MifarePlusKey::InitKey(size_t keySize)
    {
        d_keySize = keySize;
        d_key = (unsigned char*)malloc(keySize * sizeof(unsigned char));
    }

    void MifarePlusKey::clear()
    {
        memset(d_key, 0xFF, d_keySize);
    }

    void MifarePlusKey::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        Key::serialize(node);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void MifarePlusKey::unSerialize(boost::property_tree::ptree& node)
    {
        Key::unSerialize(node);
    }

    std::string MifarePlusKey::getDefaultXmlNodeName() const
    {
        return "MifarePlusKey";
    }
}
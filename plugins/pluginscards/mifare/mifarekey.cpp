/**
 * \file mifarekey.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Key.
 */

#include "mifarekey.hpp"

namespace logicalaccess
{
    MifareKey::MifareKey()
        : Key()
    {
        memset(d_key, 0x00, sizeof(d_key));
        clear();
    }

    MifareKey::MifareKey(const std::string& str)
        : Key()
    {
        memset(d_key, 0x00, sizeof(d_key));
        fromString(str);
    }

    MifareKey::MifareKey(const void* buf, size_t buflen)
        : Key()
    {
        clear();

        if (buf != NULL)
        {
            if (buflen >= MIFARE_KEY_SIZE)
            {
                memcpy(d_key, buf, MIFARE_KEY_SIZE);
                d_isEmpty = false;
            }
        }
    }

    void MifareKey::clear()
    {
        memset(getData(), 0xFF, getLength());
    }

    void MifareKey::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        Key::serialize(node);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void MifareKey::unSerialize(boost::property_tree::ptree& node)
    {
        Key::unSerialize(node);
    }

    std::string MifareKey::getDefaultXmlNodeName() const
    {
        return "MifareKey";
    }
}
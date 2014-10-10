/**
 * \file TripleDESKey.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Triple DES Key.
 */

#include "logicalaccess/cards/tripledeskey.hpp"

namespace logicalaccess
{
    TripleDESKey::TripleDESKey()
        : Key()
    {
        memset(d_key, 0x00, sizeof(d_key));
        clear();
    }

    TripleDESKey::TripleDESKey(const std::string& str)
        : Key()
    {
        memset(d_key, 0x00, sizeof(d_key));
        fromString(str);
    }

    TripleDESKey::TripleDESKey(const void* buf, size_t buflen)
        : Key()
    {
        clear();

        if (buf != NULL)
        {
            if (buflen >= TRIPLEDES_KEY_SIZE)
            {
                memcpy(d_key, buf, TRIPLEDES_KEY_SIZE);
                d_isEmpty = false;
            }
        }
    }

    void TripleDESKey::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        Key::serialize(node);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void TripleDESKey::unSerialize(boost::property_tree::ptree& node)
    {
        Key::unSerialize(node);
    }

    std::string TripleDESKey::getDefaultXmlNodeName() const
    {
        return "TripleDESKey";
    }
}
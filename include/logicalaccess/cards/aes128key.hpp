/**
 * \file aes128key.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AES128 key.
 */

#ifndef LOGICALACCESS_AES128KEY_HPP
#define LOGICALACCESS_AES128KEY_HPP

#include "logicalaccess/key.hpp"

/**
 * \brief The default aes 128-bit key size
 */
#define AES128_KEY_SIZE 16

namespace logicalaccess
{
    /**
     * \brief A AES128 key class.
     */
    class LIBLOGICALACCESS_API AES128Key : public Key
    {
    public:
        using XmlSerializable::serialize;
        using XmlSerializable::unSerialize;

        /**
         * \brief Build an empty 16-bytes AES128 key.
         */
        AES128Key();

        /**
         * \brief Build a AES128 key given a string representation of it.
         * \param str The string representation.
         */
        AES128Key(const std::string& str);

        /**
         * \brief Build a AES128 key given a buffer.
         * \param buf The buffer.
         * \param buflen The buffer length.
         */
        AES128Key(const void* buf, size_t buflen);

        /**
         * \brief Get the key length.
         * \return The key length.
         */
        inline size_t getLength() const { return AES128_KEY_SIZE; };

        /**
         * \brief Get the key data.
         * \return The key data.
         */
        inline const unsigned char* getData() const { return d_key; };

        /**
         * \brief Get the key data.
         * \return The key data.
         */
        inline unsigned char* getData() { return d_key; };

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         * \return The XML string.
         */
        virtual void serialize(boost::property_tree::ptree& parentNode);

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
        virtual void unSerialize(boost::property_tree::ptree& node);

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
        virtual std::string getDefaultXmlNodeName() const;

    private:

        /**
         * \brief The key bytes;
         */
        unsigned char d_key[AES128_KEY_SIZE];
    };
}

#endif /* LOGICALACCESS_AES128KEY_H */
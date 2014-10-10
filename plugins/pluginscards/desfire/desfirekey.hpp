/**
 * \file desfirekey.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire key.
 */

#ifndef LOGICALACCESS_DESFIREKEY_HPP
#define LOGICALACCESS_DESFIREKEY_HPP

#include "logicalaccess/key.hpp"

/**
 * \brief The DESFire DES key size
 */
#define DESFIRE_DES_KEY_SIZE 16

/**
 * \brief The DESFire AES key size
 */
#define DESFIRE_AES_KEY_SIZE 16

/**
 * \brief The DESFire max key size
 */
#define DESFIRE_MAXKEY_SIZE 24

namespace logicalaccess
{
    /**
     * \brief The DESFire key type.
     */
    typedef enum
    {
        DF_KEY_DES = 0x00,
        DF_KEY_3K3DES = 0x40,
        DF_KEY_AES = 0x80
    }DESFireKeyType;

    /**
     * \brief A DESFire Key class.
     */
    class LIBLOGICALACCESS_API DESFireKey : public Key
    {
    public:

        /**
         * \brief Build an empty DESFire key.
         */
        DESFireKey();

        /**
         * \brief Build a DESFire key given a string representation of it.
         * \param str The string representation.
         */
        DESFireKey(const std::string& str);

        /**
         * \brief Build a DESFire key given a buffer.
         * \param buf The buffer.
         * \param buflen The buffer length.
         */
        DESFireKey(const void* buf, size_t buflen);

        /**
         * \brief Get the key length.
         * \return The key length.
         */
        size_t getLength() const;

        /**
         * \brief Get the key data.
         * \return The key data.
         */
        inline const unsigned char* getData() const { return &d_key[0]; };

        /**
         * \brief Get the key data.
         * \return The key data.
         */
        inline unsigned char* getData() { return &d_key[0]; };

        /**
         * \brief Set the key version
         * \param key_version The key version.
         */
        void setKeyVersion(unsigned char key_version) { d_key_version = key_version; };

        /**
         * \brief Get the key version
         * \return The key version.
         */
        unsigned char getKeyVersion() const { return d_key_version; };

        /**
         * \brief Set the key type.
         * \param keyType The key type.
         */
        void setKeyType(DESFireKeyType keyType) { d_keyType = keyType; };

        /**
         * \brief Get the key type.
         * \return The key type.
         */
        DESFireKeyType getKeyType() const { return d_keyType; };

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
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

        /**
         * \brief Equality operator
         * \param ai DESFire key to compare.
         * \return True if equals, false otherwise.
         */
        virtual bool operator==(const DESFireKey& key) const;

        /**
         * \brief Inequality operator
         * \param ai DESFire key to compare.
         * \return True if inequals, false otherwise.
         */
        inline bool operator!=(const DESFireKey& key) const { return !operator==(key); };

        /**
         * \brief Get the DESFire Key Type in string format.
         * \return The key type in string.
         */
        static std::string DESFireKeyTypeStr(DESFireKeyType t);

    private:

        /**
         * \brief The key bytes;
         */
        std::vector<unsigned char> d_key;

        /**
         * \brief The key version.
         */
        unsigned char d_key_version;

        /**
         * \brief The DESFire key type.
         */
        DESFireKeyType d_keyType;
    };
}

#endif /* LOGICALACCESS_DESFIREKEY_HPP */
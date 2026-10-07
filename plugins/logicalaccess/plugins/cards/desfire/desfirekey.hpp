/**
 * \file desfirekey.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire key.
 */

#ifndef LOGICALACCESS_DESFIREKEY_HPP
#define LOGICALACCESS_DESFIREKEY_HPP

#include <cstddef>
#include <string>

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/desfire/lla_cards_desfire_api.hpp>

/**
 * \brief The DESFire DES key size
 */
#define DESFIRE_DES_KEY_SIZE 16

/**
 * \brief The DESFire AES-128 key size
 */
#define DESFIRE_AES_KEY_SIZE 16

/**
 * \brief The DESFire AES-256 key size
 *
 * DUOX uses key type bits 11b to identify AES-256 keys
 */
#define DESFIRE_AES256_KEY_SIZE 32

/**
 * \brief The DESFire max key size
 * 
 * Value is kept at 24 bytes for compatibility with the existing DESFire implementation.
 * It is not the maximum size of every key type
 */
#define DESFIRE_MAXKEY_SIZE 24

namespace logicalaccess
{
/**
 * \brief The DESFire key type.
 */
typedef enum {
    DF_KEY_DES    = 0x00,
    DF_KEY_3K3DES = 0x40,
    DF_KEY_AES    = 0x80, // TODO later : rename DF_KEY_AES to DF_KEY_AES128 in a larger refactor
    DF_KEY_AES256 = 0xC0
} DESFireKeyType;

/**
 * \brief A DESFire Key class.
 */
class LLA_CARDS_DESFIRE_API DESFireKey : public Key
{
  public:
    using XmlSerializable::serialize;
    using XmlSerializable::unSerialize;

    /**
     * \brief Build an empty DESFire key.
     */
    DESFireKey();

    /**
     * \brief Build a DESFire key given a string representation of it.
     * \param str The string representation.
     */
    explicit DESFireKey(const std::string &str);

    /**
     * \brief Build a DESFire key given a buffer.
     * \param buf The buffer.
     * \param buflen The buffer length.
     */
    DESFireKey(const void *buf, std::size_t buflen);
    
    /**
     * Create a key from a bytes vector.
     * \param data The key data.
     */
    explicit DESFireKey(const ByteVector &data);

    /**
     * \brief Get the key length.
     * 
     * Length configured with setLength() takes precedence. Otherwise the length determined from the DESFire key type.
     * 
     * \return The key length.
     */
    std::size_t getLength() const override;
    
    /**
     * \brief Override the key length.
     * \param length The key length in bytes.
     */
    void setLength(std::size_t length)
    {
        d_length = length;
    }

    /**
     * \brief Set the key version
     * \param key_version The key version.
     */
    void setKeyVersion(unsigned char key_version)
    {
        d_key_version = key_version;
    }

    /**
     * \brief Get the key version
     * \return The key version.
     */
    unsigned char getKeyVersion() const
    {
        return d_key_version;
    }

    /**
     * \brief Set the key type.
     * \param keyType The key type.
     */
    void setKeyType(DESFireKeyType keyType);

    /**
     * \brief Get the key type.
     * \return The key type.
     */
    DESFireKeyType getKeyType() const
    {
        return d_keyType;
    }

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override;

    /**
     * \brief Equality operator
     * \param key DESFire key to compare.
     * \return True if equals, false otherwise.
     */
    virtual bool operator==(const DESFireKey &key) const;

    /**
     * \brief Inequality operator
     * \param key DESFire key to compare.
     * \return True if inequals, false otherwise.
     */
    bool operator!=(const DESFireKey &key) const
    {
        return !operator==(key);
    }

    /**
     * \brief Get the DESFire Key Type in string format.
     * \param t The DESFire key type.
     * \return The key type in string.
     */
    static std::string DESFireKeyTypeStr(DESFireKeyType t);

  private:
    /**
     * \brief The key version.
     */
    unsigned char d_key_version;

    /**
     * \brief The DESFire key type.
     */
    DESFireKeyType d_keyType;
    
    /**
     * \brief The overriden key length, if forced.
     * 
     * A value of 0 means that the length is determined from d_keyType.
     */
    std::size_t d_length;
};

} // namespace logicalaccess

#endif /* LOGICALACCESS_DESFIREKEY_HPP */
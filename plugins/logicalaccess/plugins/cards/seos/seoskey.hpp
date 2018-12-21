/**
 * \file seoskey.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SEOS key.
 */

#ifndef LOGICALACCESS_SEOSKEY_HPP
#define LOGICALACCESS_SEOSKEY_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/seos/lla_cards_seos_api.hpp>

/**
 * \brief The SEOS key size
 */
#define SEOS_KEY_SIZE 16

namespace logicalaccess
{
/**
 * \brief The SEOS key type.
 */
typedef enum
{
    SEOS_KEY_DES    = 0x00,
    SEOS_KEY_AES    = 0x40
} SeosKeyType;

/**
 * \brief A SEOS Key class.
 */
class LLA_CARDS_SEOS_API SeosKey : public Key
{
  public:
#ifndef SWIG
    using XmlSerializable::serialize;
    using XmlSerializable::unSerialize;
#endif

    /**
     * \brief Build an empty SEOS key.
     */
    SeosKey();

    /**
     * \brief Build a SEOS key given a string representation of it.
     * \param str The string representation.
     */
    explicit SeosKey(const std::string &str);

    /**
     * \brief Build a SEOS key given a buffer.
     * \param buf The buffer.
     * \param buflen The buffer length.
     */
    SeosKey(const void *buf, size_t buflen);

    /**
     * \brief Get the key length.
     * \return The key length.
     */
    size_t getLength() const override;

    /**
     * \brief Get the key data.
     * \return The key data.
     */
    const unsigned char *getData() const override
    {
        return &d_key[0];
    }

    /**
     * \brief Get the key data.
     * \return The key data.
     */
    unsigned char *getData() override
    {
        return &d_key[0];
    }

    /**
     * \brief Set the key type.
     * \param keyType The key type.
     */
    void setKeyType(SeosKeyType keyType);

    /**
     * \brief Get the key type.
     * \return The key type.
     */
    SeosKeyType getKeyType() const
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
     * \param ai SEOS key to compare.
     * \return True if equals, false otherwise.
     */
    virtual bool operator==(const SeosKey &key) const;

    /**
     * \brief Inequality operator
     * \param ai SEOS key to compare.
     * \return True if inequals, false otherwise.
     */
    bool operator!=(const SeosKey &key) const
    {
        return !operator==(key);
    }

    /**
     * \brief Get the SEOS Key Type in string format.
     * \return The key type in string.
     */
    static std::string SeosKeyTypeStr(SeosKeyType t);

  private:
    /**
     * \brief The key bytes;
     */
    ByteVector d_key;

    /**
     * \brief The SEOS key type.
     */
    SeosKeyType d_keyType;
};
}

#endif /* LOGICALACCESS_SEOSKEY_HPP */
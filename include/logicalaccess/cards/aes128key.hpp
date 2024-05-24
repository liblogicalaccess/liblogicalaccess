/**
 * \file aes128key.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief AES128 key.
 */

#ifndef LOGICALACCESS_AES128KEY_HPP
#define LOGICALACCESS_AES128KEY_HPP

#include <logicalaccess/key.hpp>

/**
 * \brief The default aes 128-bit key size
 */
#define AES128_KEY_SIZE 16

namespace logicalaccess
{
/**
 * \brief A AES128 key class.
 */
class LLA_CORE_API AES128Key : public Key
{
  public:
    using XmlSerializable::serialize;
    using XmlSerializable::unSerialize;

    /**
     * \brief Build an empty 16-bytes AES128 key.
     */
    AES128Key();

    virtual ~AES128Key() = default;

    /**
     * \brief Build a AES128 key given a string representation of it.
     * \param str The string representation.
     */
    explicit AES128Key(const std::string &str);

    /**
     * \brief Build a AES128 key given a buffer.
     * \param buf The buffer.
     * \param buflen The buffer length.
     */
    AES128Key(const void *buf, size_t buflen);

    /**
     * Create a key from a bytes vector.
     */
    explicit AES128Key(const ByteVector &data);

    /**
     * \brief Get the key length.
     * \return The key length.
     */
    size_t getLength() const override
    {
        return AES128_KEY_SIZE;
    }

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     * \return The XML string.
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
};
}

#endif /* LOGICALACCESS_AES128KEY_H */
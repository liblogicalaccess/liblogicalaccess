/**
 * \file mifarekey.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Mifare key.
 */

#ifndef LOGICALACCESS_MIFAREKEY_HPP
#define LOGICALACCESS_MIFAREKEY_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/mifare/lla_cards_mifare_api.hpp>

/**
 * \brief The default Mifare key size
 */
#define MIFARE_KEY_SIZE 6

namespace logicalaccess
{
/**
 * \brief A Mifare Key class.
 */
class LLA_CARDS_MIFARE_API MifareKey : public Key
{
  public:
    using XmlSerializable::serialize;
    using XmlSerializable::unSerialize;

    /**
     * \brief Build an empty 6-bytes Mifare key.
     */
    MifareKey();

    /**
     * \brief Build a Mifare key given a string representation of it.
     * \param str The string representation.
     */
    explicit MifareKey(const std::string &str);

    /**
     * \brief Build a Mifare key given a buffer.
     * \param buf The buffer.
     * \param buflen The buffer length.
     */
    MifareKey(const void *buf, size_t buflen);
    
    /**
     * Create a key from a bytes vector.
     */
    explicit MifareKey(const ByteVector &data);

    /**
     * \brief Get the key length.
     * \return The key length.
     */
    size_t getLength() const override
    {
        return MIFARE_KEY_SIZE;
    }
    
    virtual unsigned char getEmptyByte() const
    {
        return 0xff;
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
};
}

#endif /* LOGICALACCESS_MIFAREKEY_HPP */

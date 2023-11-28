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
     * \brief Clear the key.
     */
    void clear() override;

    /**
     * \brief Get the key length.
     * \return The key length.
     */
    size_t getLength() const override
    {
        return MIFARE_KEY_SIZE;
    }

    /**
     * \brief Get the key data.
     * \return The key data.
     */
    const unsigned char *getData() const override
    {
        return d_key;
    }

    /**
     * \brief Get the key data.
     * \return The key data.
     */
    unsigned char *getData() override
    {
        return d_key;
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

    bool operator==(MifareKey b)
    {
      unsigned char *keyA = this->getData();
      unsigned char *keyB = b.getData();

      for (unsigned int i = 0; i != 6; i++)
      {;
        if (keyA[i] != keyB[i])
          return false;
      }
      return true;
    }

    bool operator!=(MifareKey b)
    {
      bool a = (*this) == b;
      return !a;
    }
  private:
    /**
     * \brief The key bytes
     */
    unsigned char d_key[MIFARE_KEY_SIZE];
};
}

#endif /* LOGICALACCESS_MIFAREKEY_HPP */

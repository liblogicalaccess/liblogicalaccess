/**
 * \file hmac1key.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief HMAC1 key.
 */

#ifndef LOGICALACCESS_HMAC1KEY_HPP
#define LOGICALACCESS_HMAC1KEY_HPP

#include "logicalaccess/key.hpp"

/**
 * \brief The default hmac-1 key size
 */
#define HMAC1_KEY_SIZE 10

namespace logicalaccess
{
/**
 * \brief A hmac-1 key class.
 */
class LIBLOGICALACCESS_API HMAC1Key : public Key
{
  public:
    /**
     * \brief Build an empty 10-bytes hmac-1 key.
     */
    HMAC1Key();

    virtual ~HMAC1Key() = default;

    /**
     * \brief Build a hmac-1 key given a string representation of it.
     * \param str The string representation.
     */
    explicit HMAC1Key(const std::string &str);

    /**
     * \brief Build a hmac-1 key given a buffer.
     * \param buf The buffer.
     * \param buflen The buffer length.
     */
    HMAC1Key(const void *buf, size_t buflen);

    /**
     * \brief Get the key length.
     * \return The key length.
     */
    size_t getLength() const override
    {
        return HMAC1_KEY_SIZE;
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

  private:
    /**
     * \brief The key bytes;
     */
    unsigned char d_key[HMAC1_KEY_SIZE];
};
}

#endif /* LOGICALACCESS_HMAC1KEY_HPP */
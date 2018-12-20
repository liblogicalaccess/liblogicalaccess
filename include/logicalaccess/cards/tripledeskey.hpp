/**
 * \file tripledeskey.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Triple DES key.
 */

#ifndef LOGICALACCESS_TRIPLEDESKEY_HPP
#define LOGICALACCESS_TRIPLEDESKEY_HPP

#include <logicalaccess/key.hpp>

/**
 * \brief The default triple des key size
 */
#define TRIPLEDES_KEY_SIZE 16

namespace logicalaccess
{
/**
 * \brief A triple DES class.
 */
class LLA_CORE_API TripleDESKey : public Key
{
  public:
#ifndef SWIG
    using XmlSerializable::serialize;
    using XmlSerializable::unSerialize;
#else
    /**
     * \brief Serialize object to Xml Node string.
     * \return The serialized object.
     */
    virtual std::string serialize();

    /**
     * \brief UnSerialize object from a Xml Node string.
     * \param xmlstring The Xml Node string.
     * \param rootNode The root node.
     * \return True on success, false otherwise.
     */
    virtual void unSerialize(const std::string &xmlstring, const std::string &rootNode);

    /**
     * \brief UnSerialize object from a Xml node.
     * \param is The Xml stream.
     * \param rootNode The root node.
     * \return True on success, false otherwise.
     */
    virtual void unSerialize(std::istream &is, const std::string &rootNode);

    /**
     * \brief UnSerialize object from a Xml node.
     * \param node The Xml node.
     * \param rootNode The root node.
     * \return True on success, false otherwise.
     */
    virtual void unSerialize(boost::property_tree::ptree &node,
                             const std::string &rootNode);
#endif

    /**
     * \brief Build an empty 16-bytes triple DES key.
     */
    TripleDESKey();

    virtual ~TripleDESKey() = default;

    /**
     * \brief Build a triple DES key given a string representation of it.
     * \param str The string representation.
     */
    explicit TripleDESKey(const std::string &str);

    /**
     * \brief Build a triple DES key given a buffer.
     * \param buf The buffer.
     * \param buflen The buffer length.
     */
    TripleDESKey(const void *buf, size_t buflen);

    /**
     * \brief Get the key length.
     * \return The key length.
     */
    size_t getLength() const override
    {
        return TRIPLEDES_KEY_SIZE;
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
    unsigned char d_key[TRIPLEDES_KEY_SIZE];
};
}

#endif /* LOGICALACCESS_TRIPLEDESKEY_HPP */
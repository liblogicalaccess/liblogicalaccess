/**
 * \file Key.hpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime@leosac.com>
 * \brief Key card.
 */

#ifndef LOGICALACCESS_KEY_HPP
#define LOGICALACCESS_KEY_HPP

#include <logicalaccess/cards/keystorage.hpp>
#include <logicalaccess/cards/keydiversification.hpp>

namespace logicalaccess
{
class KeyDiversification;
/**
 * \brief A Key base class. The key object is used to describe key chip information used
 * for authentication on secured memory area or for restricted operation.
 */
class LLA_CORE_API Key : public XmlSerializable,
                                 public std::enable_shared_from_this<Key>
{
  public:
    /**
     * \brief Constructor.
     */
    Key();

    virtual ~Key() = default;

    /**
     * \brief Set the key to its default (only 0's).
     */
    virtual void clear();

    /**
     * \brief Get the key length.
     * \return The key length.
     */
    virtual size_t getLength() const = 0;

    /**
     * \brief Get the key data.
     * \return The key data.
     */
    virtual const unsigned char *getData() const = 0;

    /**
     * \brief Get the key data.
     * \return The key data.
     */
    virtual unsigned char *getData() = 0;

    /**
     * \brief Set the key data.
     * \param data The key data.
     */
    virtual void setData(const unsigned char *data);

    /**
     * \brief Set the key data.
     * \param data The key data.
     */
    virtual void setData(const ByteVector &data, size_t offset = 0);

    /**
     * \brief Get the string representation of the key.
     * \return The string representation of the key.
     */
    std::string getString(bool withSpace = true) const;

    /**
     * \brief Get if key data are empty.
     * \return True if empty, false otherwise.
     */
    bool isEmpty() const
    {
        return (d_isEmpty && d_key_storage->getType() == KST_COMPUTER_MEMORY);
    }

    /**
     * \brief Set the key from a string representation of it.
     * \param str The string representation.
     * \return true on success, false otherwise.
     * \warning On failure, the content of the key is undefined.
     */
    virtual bool fromString(const std::string &str);

    /**
     * \brief Set the key storage used for this key.
     * \param key_storage The key storage.
     */
    void setKeyStorage(std::shared_ptr<KeyStorage> key_storage);

    /**
     * \brief Get the key storage used for this key.
     * \return The key storage.
     */
    std::shared_ptr<KeyStorage> getKeyStorage() const;

    /**
     * \brief Serialize the current object to XML.
     * \param npde The XML node.
     * \return The XML string.
     */
    void serialize(boost::property_tree::ptree &node) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

    /**
     * \brief Equality operator
     * \param key Key to compare.
     * \return True if equals, false otherwise.
     */
    virtual bool operator==(const Key &key) const;

    /**
     * \brief Inequality operator
     * \param key Key to compare.
     * \return True if inequals, false otherwise.
     */
    virtual bool operator!=(const Key &key) const;

    /**
     * \brief Set the cipher key.
     * \param key The key.
     */
    void setCipherKey(const std::string &key);

    /**
     * \brief Set if the data is stored ciphered.
     * \param cipher The data is stored ciphered or not.
     */
    void setStoreCipheredData(bool cipher);

    /**
     * \brief Get if the data is stored ciphered.
     * \return True if data is stored ciphered, false otherwise.
     */
    bool getStoreCipheredData() const;

    void setKeyDiversification(std::shared_ptr<KeyDiversification> div);

    std::shared_ptr<KeyDiversification> getKeyDiversification() const;

    /**
     * Retrieve the bytes of the key.
     */
    ByteVector getBytes() const;

  private:
    /**
     * \brief The default 'secure' key for ciphering.
     */
    static const std::string secureAiKey;

    /**
     * \brief Cipher a key if store ciphered data is true.
     * \param xmlwriter The xml node.
     */
    void cipherKeyData(boost::property_tree::ptree &node);

    /**
     * \brief Uncipher a key if store ciphered data is true.
     * \param node The node containing key data.
     */
    void uncipherKeyData(boost::property_tree::ptree &node);

  protected:
    /**
     * \brief Checked if key data are empty.
     */
    bool d_isEmpty;

    /**
     * \brief The key storage used for this key.
     */
    std::shared_ptr<KeyStorage> d_key_storage;

    /**
     * \brief The key storage used for this key.
     */
    std::shared_ptr<KeyDiversification> d_key_diversification;

    /**
     * \brief The cipher key.
     */
    std::string d_cipherKey;

    /**
     * \brief The data is stored ciphered or not.
     */
    bool d_storeCipheredData;
};

/**
 * \brief Output to stream operator.
 * \param os The output stream.
 * \param key The key.
 * \return os.
 */
LLA_CORE_API std::ostream &operator<<(std::ostream &os, const Key &key);
}

#endif /* LOGICALACCESS_KEY_HPP */
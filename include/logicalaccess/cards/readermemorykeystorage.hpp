/**
 * \file readermemorykeystorage.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Reader memory key storage description.
 */

#ifndef LOGICALACCESS_READERMEMORYKEYSTORAGE_HPP
#define LOGICALACCESS_READERMEMORYKEYSTORAGE_HPP

#include <logicalaccess/cards/keystorage.hpp>

namespace logicalaccess
{
/**
 * \brief A reader memory key storage class.
 */
class LLA_CORE_API ReaderMemoryKeyStorage : public KeyStorage
{
  public:
    /**
     * \brief Constructor.
     */
    ReaderMemoryKeyStorage();

    virtual ~ReaderMemoryKeyStorage() = default;

    /**
     * \brief Get the key storage type.
     * \return The key storage type.
     */
    KeyStorageType getType() const override;

    /**
     * \brief Set the key slot.
     * \param key_slot The key slot.
     */
    void setKeySlot(unsigned char key_slot);

    /**
     * \brief Get the key slot.
     * \return The key slot.
     */
    unsigned char getKeySlot() const;

    /**
     * \brief Set if the key is stored volatile.
     * \param vol True if the key is stored volatile, false otherwise.
     */
    void setVolatile(bool vol);

    /**
     * \brief Set if the key is stored volatile.
     * \return True if the key is stored volatile, false otherwise.
     */
    bool getVolatile() const;

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

  protected:
    /**
     * \brief The key slot.
     */
    unsigned char d_key_slot;

    /**
     * \brief True if the key is stored volatile, false otherwise.
     */
    bool d_volatile;
};
}

#endif /* LOGICALACCESS_READERMEMORYKEYSTORAGE_HPP */
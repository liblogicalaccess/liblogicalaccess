/**
 * \file samkeystorage.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SAM key storage description.
 */

#ifndef LOGICALACCESS_SAMKEYSTORAGE_HPP
#define LOGICALACCESS_SAMKEYSTORAGE_HPP

#include <logicalaccess/cards/keystorage.hpp>

namespace logicalaccess
{
/**
 * \brief A SAM key storage class.
 */
class LLA_CORE_API SAMKeyStorage : public KeyStorage
{
  public:
    /**
     * \brief Constructor.
     */
    SAMKeyStorage();

    virtual ~SAMKeyStorage() = default;

    /**
     * \brief Get the key storage type.
     * \return The key storage type.
     */
    KeyStorageType getType() const override;

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

    unsigned char getKeySlot() const
    {
        return d_key_slot;
    }

    void setKeySlot(unsigned char key_slot)
    {
        d_key_slot = key_slot;
    }

    bool getDumpKey() const
    {
        return d_dumpKey;
    }

    void setDumpKey(bool dumpKey)
    {
        d_dumpKey = dumpKey;
    }

  protected:
    /**
     * \brief The key slot. Optional for most PICC operation, automatically detected
     */
    unsigned char d_key_slot;

    bool d_dumpKey;
};
}

#endif /* LOGICALACCESS_SAMKEYSTORAGE_HPP */
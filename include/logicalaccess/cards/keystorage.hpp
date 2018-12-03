/**
 * \file keystorage.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Key storage description.
 */

#ifndef LOGICALACCESS_KEYSTORAGE_HPP
#define LOGICALACCESS_KEYSTORAGE_HPP

#include <logicalaccess/readerproviders/readerprovider.hpp>

namespace logicalaccess
{
/**
 * \brief The key storage types.
 */
typedef enum {
    KST_COMPUTER_MEMORY = 0x00, /* Key stored in computer memory */
    KST_READER_MEMORY   = 0x01, /* Key stored in reader memory */
    KST_SAM             = 0x02, /* Key stored in SAM */
    KST_SERVER          = 0x03, /* Key store in IKS */
    KST_PKCS            = 0x04  /* Key store in PKCS token */
} KeyStorageType;

/**
 * \brief A Key storage base class. The key storage specify where the key is stored in
 * memory. It can have cryptographic functionalities.
 */
class LLA_CORE_API KeyStorage : public XmlSerializable,
                                        public std::enable_shared_from_this<KeyStorage>
{
  public:
#ifndef SWIG
    using XmlSerializable::serialize;
    using XmlSerializable::unSerialize;
#endif

    virtual ~KeyStorage() = default;

    /**
     * \brief Get the key storage type.
     * \return The key storage type.
     */
    virtual KeyStorageType getType() const = 0;

    /**
     * \brief Get the key storage instance from a key storage type.
     * \return The key storage instance.
     */
    static std::shared_ptr<KeyStorage> getKeyStorageFromType(KeyStorageType kst);

    void serialize(boost::property_tree::ptree &parentNode) override;

    void unSerialize(boost::property_tree::ptree &node) override;

    /**
      * Retrieve an entry from the metadata key/value store
      */
    std::string getMetadata(const std::string &key);

    /**
     * Check whether or not a metadata named "key"
     * is available
     */
    bool hasMetadata(const std::string &key) const;

    /**
     * Add a new metadata to the KeyStorage
     */
    void addMetadata(const std::string &key, const std::string &value);

  protected:
    std::map<std::string, std::string> metadata_;
};
}

#endif /* LOGICALACCESS_KEYSTORAGE_HPP */
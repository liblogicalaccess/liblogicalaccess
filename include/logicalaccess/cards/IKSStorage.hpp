#pragma once

#include "logicalaccess/cards/keystorage.hpp"

namespace logicalaccess
{
/**
 * The client side of a remote key storage.
 *
 * IKSStorage is backed by IKS (Islog Key Server).
 */
class LIBLOGICALACCESS_API IKSStorage : public KeyStorage
{
  public:
    /**
     * Create an IKSStorage, specifying the identity of the
     * remote key represented by this storage.
     */
    IKSStorage(const std::string &key_identity);

    /**
     * Default construction. Key identity will be empty.
     *
     * This storage will fail when used.
     */
    IKSStorage() = default;

    virtual KeyStorageType getType() const;

    virtual void serialize(boost::property_tree::ptree &parentNode);

    virtual void unSerialize(boost::property_tree::ptree &node);

    virtual std::string getDefaultXmlNodeName() const;

    /**
     * Returns the key identity represents by the key/storage combination.
     *
     * The "key identity" is what uniquely represents a key living
     * on the IKS.
     */
    const std::string &getKeyIdentity() const;

  protected:
    std::string key_identity_;
};
}

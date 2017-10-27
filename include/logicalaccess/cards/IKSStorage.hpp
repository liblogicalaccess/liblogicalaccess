#pragma once

#include "logicalaccess/cards/keystorage.hpp"
#include "logicalaccess/iks/IslogKeyServer.hpp"

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
    explicit IKSStorage(const std::string &key_identity);

    /**
     * Default construction. Key identity will be empty.
     *
     * This storage will fail when used.
     */
    IKSStorage()
        : saveIKSConfig_(false)
    {
    }

    virtual ~IKSStorage() = default;

    KeyStorageType getType() const override;

    void serialize(boost::property_tree::ptree &parentNode) override;

    void unSerialize(boost::property_tree::ptree &node) override;

    std::string getDefaultXmlNodeName() const override;

    /**
     * Returns the key identity represents by the key/storage combination.
     *
     * The "key identity" is what uniquely represents a key living
     * on the IKS.
     */
    const std::string &getKeyIdentity() const;

    void setKeyIdentity(const std::string &idt);

    /**
     * Control the value of the saveIKSConfig_ flag.
     */
    void setSerializeIKSConfig(bool v);

    bool getSerializeIKSConfig() const;

    void setIKSConfig(const std::string &ip, uint16_t port,
                      const std::string &client_cert, const std::string &client_key,
                      const std::string &root_ca);

    const iks::IslogKeyServer::IKSConfig &getIKSConfig() const;

    /**
     * Retrieve the config stored internally.
     * This method is here to be exported by the COM wrapper.
     */
    void getIKSConfig(std::string &ip, uint16_t &port, std::string &client_cert,
                      std::string &client_key, std::string &root_ca) const;

  protected:
    std::string key_identity_;

    /**
     * Do we save IKS when serializing the object,
     * or do we clear it instead?
     */
    bool saveIKSConfig_;

    /**
     * We store the IKS configuration
     * in most IKSStorage object in order to ease
     * the loading of configuration from a serialized object.
     */
    iks::IslogKeyServer::IKSConfig config_;
};
}

#include <assert.h>
#include <logicalaccess/cards/IKSStorage.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace logicalaccess;

KeyStorageType IKSStorage::getType() const
{
    return KST_SERVER;
}

void IKSStorage::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    node.put("KeyIdentity", key_identity_);

    if (saveIKSConfig_)
    {
        node.put("IKS.ip", config_.ip);
        node.put("IKS.port", config_.port);
        node.put("IKS.client_cert", config_.client_cert);
        node.put("IKS.client_key", config_.client_key);
        node.put("IKS.root_ca", config_.root_ca);
    }
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void IKSStorage::unSerialize(boost::property_tree::ptree &node)
{
    key_identity_       = node.get_child("KeyIdentity").get_value<std::string>("");
    config_.ip          = node.get_child("IKS.ip").get_value<std::string>("");
    config_.port        = node.get_child("IKS.port").get_value<uint16_t>(0);
    config_.client_cert = node.get_child("IKS.client_cert").get_value<std::string>("");
    config_.client_key  = node.get_child("IKS.client_key").get_value<std::string>("");
    config_.root_ca     = node.get_child("IKS.root_ca").get_value<std::string>("");

    if (config_.port != 0)
    {
        // port 0 being invalid it would mean this object doesn't have any valid config
        iks::IslogKeyServer::configureGlobalInstance(config_.ip, config_.port,
                                                     config_.client_cert,
                                                     config_.client_key, config_.root_ca);
    }
}

std::string IKSStorage::getDefaultXmlNodeName() const
{
    return "IKSStorage";
}

const std::string &IKSStorage::getKeyIdentity() const
{
    return key_identity_;
}

IKSStorage::IKSStorage(const std::string &key_identity)
    : key_identity_(key_identity)
    , saveIKSConfig_(false)
{
}

void IKSStorage::setKeyIdentity(const std::string &idt)
{
    key_identity_ = idt;
}

void IKSStorage::setSerializeIKSConfig(bool v)
{
    saveIKSConfig_ = v;
}

void IKSStorage::setIKSConfig(const std::string &ip, uint16_t port,
                              const std::string &client_cert,
                              const std::string &client_key, const std::string &root_ca)
{
    config_ = iks::IslogKeyServer::IKSConfig(ip, port, client_cert, client_key, root_ca);
}

const iks::IslogKeyServer::IKSConfig &IKSStorage::getIKSConfig() const
{
    return config_;
}

bool IKSStorage::getSerializeIKSConfig() const
{
    return saveIKSConfig_;
}

void IKSStorage::getIKSConfig(std::string &ip, uint16_t &port, std::string &client_cert,
                              std::string &client_key, std::string &root_ca) const
{
    ip          = config_.ip;
    port        = config_.port;
    client_cert = config_.client_cert;
    client_key  = config_.client_key;
    root_ca     = config_.root_ca;
}

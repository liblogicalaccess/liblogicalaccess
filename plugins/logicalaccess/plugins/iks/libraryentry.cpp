#include <logicalaccess/iks/IslogKeyServer.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/plugins/iks/lla_iks_api.hpp>
#include "IKSRPCClient.hpp"

extern "C" {

LLA_IKS_API char *getLibraryName()
{
    return (char *)"RemoteCryptoIKS";
}

LLA_IKS_API void
getRemoteCrypto(const logicalaccess::iks::IslogKeyServer::IKSConfig &cfg,
                logicalaccess::RemoteCryptoPtr &remoteCrypto)
{
    remoteCrypto = std::make_shared<logicalaccess::iks::RemoteCryptoIKSProvider>(cfg);
}
}

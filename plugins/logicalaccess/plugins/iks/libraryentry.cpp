#include <logicalaccess/iks/IslogKeyServer.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include "logicalaccess/logicalaccess_api.hpp"
#include "IKSRPCClient.hpp"

extern "C" {

LIBLOGICALACCESS_API char *getLibraryName()
{
    return (char *)"RemoteCryptoIKS";
}

LIBLOGICALACCESS_API void
getRemoteCrypto(const logicalaccess::iks::IslogKeyServer::IKSConfig &cfg,
                logicalaccess::RemoteCryptoPtr &remoteCrypto)
{
    remoteCrypto = std::make_shared<logicalaccess::iks::RemoteCryptoIKSProvider>(cfg);
}
}

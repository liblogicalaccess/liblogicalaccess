#pragma once

#include "logicalaccess/logicalaccess_api.hpp"
#include <string>

namespace logicalaccess
{
namespace iks
{
/**
 * Main interface with the Islog Key Server.
 *
 * This object is movable but not copyable.
 */
class LIBLOGICALACCESS_API IslogKeyServer
{
  public:
    IslogKeyServer()                       = delete;
    IslogKeyServer(const IslogKeyServer &) = delete;
    //    IslogKeyServer(IslogKeyServer &&) = default;

    struct LIBLOGICALACCESS_API IKSConfig
    {
        IKSConfig() = default;
        IKSConfig(const std::string &ip, uint16_t port, const std::string &client_cert,
                  const std::string &client_key, const std::string &root_ca);
        std::string ip;
        uint16_t port;

        /**
         * Path to the file containing the client certificate.
         */
        std::string client_cert;

        /**
         * Path to the file containing the client key.
         */
        std::string client_key;

        /**
         * Path to the root CA.
         */
        std::string root_ca;

        /**
         * Return the *content* (PEM) of the client cert.
         */
        std::string get_client_cert_pem() const;

        /**
         * Retrieve the content of the client's private key.
         */
        std::string get_client_key_pem() const;

        /**
         * Retrieve the root CA content.
         */
        std::string get_root_ca_pem() const;

        /**
         * Network target (host:port).
         */
        std::string get_target() const;
    };

    /**
     * Pre-configure the global IKS instance.
     *
     * This method should be called before calling `fromGlobalSettings()`.
     */
    static void configureGlobalInstance(const std::string &ip, uint16_t port,
                                        const std::string &client_cert,
                                        const std::string &client_key,
                                        const std::string &root_ca);

    /**
     * Retrieve a copy of the global IKS configuration.
     */
    static IKSConfig get_global_config();

  private:
    /**
     * The registered pre-configuration is stored here.
     */
    static IKSConfig pre_configuration_;

    IKSConfig config_;
};
}
}

#pragma once

#include <logicalaccess/iks/SSLTransport.hpp>
#include <logicalaccess/iks/packet/Base.hpp>
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
        std::string client_cert;
        std::string client_key;
        std::string root_ca;
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
     * Returns a reference to a static Islog Key Server created with parameters
     * available in the `pre_configuration` data structure.
     *
     * The settings are picked by the user by calling `configureGlobalInstance()`.
     */
    static IslogKeyServer &fromGlobalSettings();

    /**
     * Connect to the server locate at `ip`:`port`
     */
    IslogKeyServer(const std::string &ip, uint16_t port, const std::string &client_cert,
                   const std::string &client_key, const std::string &root_ca);

    /**
     * Ask Islog Key Server for random data.
     */
    ByteVector get_random(size_t sz);

    /**
     * Request an AES Encryption by the key server.
     */
    ByteVector aes_encrypt(const ByteVector &in, const std::string &key_name,
                           const std::array<uint8_t, 16> &iv);
    /**
     * Request an AES Decryption by the key server.
     */
    ByteVector aes_decrypt(const ByteVector &in, const std::string &key_name,
                           const std::array<uint8_t, 16> &iv);

    ByteVector aes_decrypt(const ByteVector &in, const std::string &key_name,
                           const ByteVector &iv);

    /**
     * Request an DES Encryption by the key server using CBC mode.
     */
    ByteVector des_cbc_encrypt(const ByteVector &in, const std::string &key_name,
                               const std::array<uint8_t, 8> &iv);
    /**
     * Request an DES Decryption by the key server using CBC mode.
     */
    ByteVector des_cbc_decrypt(const ByteVector &in, const std::string &key_name,
                               const std::array<uint8_t, 8> &iv);

    /**
         * Request an DES Encryption by the key server using CBC mode.
         */
    ByteVector des_ecb_encrypt(const ByteVector &in, const std::string &key_name,
                               const std::array<uint8_t, 8> &iv);
    /**
     * Request an DES Decryption by the key server using CBC mode.
     */
    ByteVector des_ecb_decrypt(const ByteVector &in, const std::string &key_name,
                               const std::array<uint8_t, 8> &iv);

    /**
     * Send a command and retrieve a response.
     * On network error, attempt to reconnect and try again.
     */
    std::shared_ptr<BaseResponse> transact(const BaseCommand &cmd);

    void send_command(const BaseCommand &cmd) const;

    std::shared_ptr<BaseResponse> recv() const;

  private:
    void setup_transport();
    std::shared_ptr<BaseResponse> build_response(uint32_t size, uint16_t opcode,
                                                 uint16_t status,
                                                 const ByteVector &data) const;

    ByteVector des_crypto(const ByteVector &in, const std::string &key_name,
                          const std::array<uint8_t, 8> &iv, bool use_ecb, bool decrypt);

#ifdef ENABLE_SSLTRANSPORT
    boost::asio::ssl::context ssl_ctx_;
#endif /* ENABLE_SSLTRANSPORT */
    std::unique_ptr<SSLTransport> transport_;

    /**
     * The registered pre-configuration is stored here.
     */
    static IKSConfig pre_configuration_;

    IKSConfig config_;
};
}
}

#pragma once

#include "logicalaccess/iks/IslogKeyServer.hpp"
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include <logicalaccess/iks/RemoteCrypto.hpp>
#include "logicalaccess/lla_fwd.hpp"
#include "logicalaccess/plugins/iks/iks.grpc.pb.h"

namespace logicalaccess
{
namespace iks
{

/**
 * Wraps a RPC client to IKS.
 */
class IKSRPCClient
{
  public:
    explicit IKSRPCClient(IslogKeyServer::IKSConfig config);

    ByteVector gen_random(int size);

    ByteVector aes_encrypt(const ByteVector &in, const std::string &key_name,
                           const ByteVector &iv);

    ByteVector aes_decrypt(const ByteVector &in, const std::string &key_name,
                           const ByteVector &iv,
                           ::logicalaccess::SignatureResult *out_signature = nullptr);

    SMSG_DesfireISOAuth_Step1 desfire_auth_iso_step1(CMSG_DesfireISOAuth_Step1 req);
    SMSG_DesfireAuth_Step2 desfire_auth_iso_step2(CMSG_DesfireAuth_Step2 req);

    SMSG_DesfireAESAuth_Step1 desfire_auth_aes_step1(CMSG_DesfireAESAuth_Step1 req);
    SMSG_DesfireAuth_Step2 desfire_auth_aes_step2(CMSG_DesfireAuth_Step2 req);

    SMSG_DesfireChangeKey desfire_change_key(CMSG_DesfireChangeKey req);

  private:
    IslogKeyServer::IKSConfig config_;
    std::shared_ptr<::grpc::ChannelInterface> channel;
    std::unique_ptr<IKSService::Stub> stub_;
};

/**
 * Implement the "RemoteCrypto" API by delegating to IKS.
 */
class RemoteCryptoIKSProvider : public RemoteCrypto
{
  public:
    explicit RemoteCryptoIKSProvider(IslogKeyServer::IKSConfig config);

    bool verify_signature(const SignatureResult &sr,
                          const std::string &pubkey_pem) override;

    ByteVector aes_encrypt(const ByteVector &in, const std::string &key_name,
                           const ByteVector &iv) override;

    ByteVector aes_decrypt(const ByteVector &in, const std::string &key_name,
                           const ByteVector &iv, SignatureResult *out_signature) override;

    void iso_authenticate_step1(const std::string &key_identity,
                                const ByteVector &random_picc, const MyDivInfo &div_info,
                                bool &out_success, ByteVector &out_random2,
                                ByteVector &out_encrypted_cryptogram,
                                ByteVector &out_auth_context_id) override;

    void iso_authenticate_step2(const std::string &key_identity,
                                const ByteVector &picc_cryptogram,
                                const ByteVector &auth_context_id,
                                const MyDivInfo &div_info, bool &out_success,
                                ByteVector &out_session_key,
                                ByteVector &out_session_key_ref) override;

    void aes_authenticate_step1(const std::string &key_identity,
                                const ByteVector &encrypted_random_picc,
                                const MyDivInfo &div_info, bool &out_success,
                                ByteVector &out_encrypted_cryptogram,
                                ByteVector &out_auth_context_id) override;

    void aes_authenticate_step2(const std::string &key_identity,
                                const ByteVector &picc_cryptogram,
                                const ByteVector &auth_context_id,
                                const MyDivInfo &div_info, bool &out_success,
                                ByteVector &out_session_key,
                                ByteVector &out_session_key_ref) override;

    void change_key(const std::string &old_key_identity,
                    const std::string &new_key_identity, bool change_same_key,
                    const std::string &session_key_uuid, const ByteVector &session_key,
                    const MyDivInfo &old_key_div, const MyDivInfo &new_key_div,
                    uint8_t key_no, const ByteVector &iv,
                    ByteVector &out_cryptogram) override;

  private:
    /**
     * Convert a LLA DivInfo object into a protobuf message.
     */
    KeyDiversificationInfo convert_div_info(const MyDivInfo &);

    IKSRPCClient iks_rpc_client_;
};
}
}

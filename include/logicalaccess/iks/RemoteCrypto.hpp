#pragma once

#include "logicalaccess/lla_fwd.hpp"

namespace logicalaccess
{
// mostly copy-past protobuf SignatureResult message.
struct SignatureResult
{
    struct Description
    {
        ByteVector payload;
        uint64_t nonce{};
        ByteVector run_uuid;
        uint64_t timestamp{};
    } desc;

    // Cryptographic signature over proto_serialized_desc
    ByteVector signature;
};

// mostly copy-paste protobuf KeyDiversificationInfo
struct MyDivInfo
{
    std::string div_type; // support only NXPAV2
    ByteVector div_input;
};

/**
 * Base plugin API for RemoteCrypto.
 *
 * Currently, only access to IKS is implemented through this
 * plugin.
 */
class RemoteCrypto
{
  public:
    /**
     * This verify a signature provided by IKS.
     *
     * This is not really a remote call, but in order to perform verification
     * we must built and serialize a protobuf message.
     *
     * Therefore this function only reason to exists is to avoid link-time
     * dependencies.
     */
    virtual bool verify_signature(const SignatureResult &sr,
                                  const std::string &pubkey_pem) = 0;

    virtual ByteVector aes_encrypt(const ByteVector &in, const std::string &key_name,
                                   const ByteVector &iv) = 0;

    virtual ByteVector aes_decrypt(const ByteVector &in, const std::string &key_name,
                                   const ByteVector &iv,
                                   SignatureResult *out_signature = nullptr) = 0;

    virtual void iso_authenticate_step1(const std::string &key_identity,
                                        const ByteVector &random_picc,
                                        const MyDivInfo &div_info, bool &out_success,
                                        ByteVector &out_random2,
                                        ByteVector &out_encrypted_cryptogram,
                                        ByteVector &out_auth_context_id) = 0;

    virtual void iso_authenticate_step2(const std::string &key_identity,
                                        const ByteVector &picc_cryptogram,
                                        const ByteVector &auth_context_id,
                                        const MyDivInfo &div_info, bool &out_success,
                                        ByteVector &out_session_key,
                                        ByteVector &out_session_key_ref) = 0;

    virtual void aes_authenticate_step1(const std::string &key_identity,
                                        const ByteVector &encrypted_random_picc,
                                        const MyDivInfo &div_info, bool &out_success,
                                        ByteVector &out_encrypted_cryptogram,
                                        ByteVector &out_auth_context_id) = 0;

    virtual void aes_authenticate_step2(const std::string &key_identity,
                                        const ByteVector &picc_cryptogram,
                                        const ByteVector &auth_context_id,
                                        const MyDivInfo &div_info, bool &out_success,
                                        ByteVector &out_session_key,
                                        ByteVector &out_session_key_ref) = 0;

    /**
     * Either session_key_uuid or session_key.
     */
    virtual void change_key(const std::string &old_key_identity,
                            const std::string &new_key_identity, bool change_same_key,
                            const std::string &session_key_uuid,
                            const ByteVector &session_key, const MyDivInfo &old_key_div,
                            const MyDivInfo &new_key_div, uint8_t key_no,
                            const ByteVector &iv, ByteVector &out_cryptogram) = 0;
};
}

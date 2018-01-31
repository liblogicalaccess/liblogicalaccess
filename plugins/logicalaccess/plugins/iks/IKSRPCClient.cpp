#include <chrono>
#include <logicalaccess/iks/IslogKeyServer.hpp>
#include "logicalaccess/plugins/iks/IKSRPCClient.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/plugins/iks/iks.grpc.pb.h"
#include "logicalaccess/plugins/iks/RPCException.hpp"

namespace logicalaccess
{
namespace iks
{
IKSRPCClient::IKSRPCClient(IslogKeyServer::IKSConfig config)
    : config_(config)
{
    // Configure gRPC ssl from IKSConfig.
    grpc::SslCredentialsOptions ssl_opts;
    ssl_opts.pem_cert_chain  = config.get_client_cert_pem();
    ssl_opts.pem_private_key = config.get_client_key_pem();
    ssl_opts.pem_root_certs  = config.get_root_ca_pem();

    channel = grpc::CreateChannel(config.get_target(), grpc::SslCredentials(ssl_opts));
    // test only, no ssl.
    // channel = grpc::CreateChannel(config.get_target(),
    // grpc::InsecureChannelCredentials());

    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(10000);
    channel->WaitForConnected(deadline);
    stub_ = std::unique_ptr<IKSService::Stub>(IKSService::NewStub(channel));
}

ByteVector IKSRPCClient::gen_random(int size)
{
    grpc::ClientContext context;
    CMSG_GenRandom req;
    req.set_size(size);

    SMSG_GenRandom rep;
    grpc::Status rpc_status = stub_->GenRandom(&context, req, &rep);
    if (rpc_status.ok())
    {
        return ByteVector(rep.randombytes().begin(), rep.randombytes().end());
    }
    throw RPCException(rpc_status.error_message() + ": " + rpc_status.error_details());
}

ByteVector IKSRPCClient::aes_encrypt(const ByteVector &in, const std::string &key_name,
                                     const ByteVector &iv)
{
    grpc::ClientContext context;
    CMSG_AESOperation req;
    req.set_key_uuid(key_name);
    req.set_payload(std::string(in.begin(), in.end()));
    req.set_iv(std::string(iv.begin(), iv.end()));

    SMSG_AESResult rep;
    grpc::Status rpc_status = stub_->AESEncrypt(&context, req, &rep);
    if (rpc_status.ok())
    {
        return ByteVector(rep.payload().begin(), rep.payload().end());
    }
    throw RPCException(rpc_status.error_message() + ": " + rpc_status.error_details());
}

ByteVector IKSRPCClient::aes_decrypt(const ByteVector &in, const std::string &key_name,
                                     const ByteVector &iv, SignatureResult *out_signature)
{
    grpc::ClientContext context;
    CMSG_AESOperation req;
    req.set_key_uuid(key_name);
    req.set_payload(std::string(in.begin(), in.end()));
    req.set_iv(std::string(iv.begin(), iv.end()));
    req.set_request_signature(!!out_signature);

    SMSG_AESResult rep;
    grpc::Status rpc_status = stub_->AESDecrypt(&context, req, &rep);
    if (rpc_status.ok())
    {
        if (out_signature)
        {
            // copy signature and its description.
            auto sig_str             = rep.signature();
            out_signature->signature = ByteVector(sig_str.begin(), sig_str.end());
            // todo copy description
            // out_signature->signature_description_ = rep.signaturedescription();
        }
        return ByteVector(rep.payload().begin(), rep.payload().end());
    }
    throw RPCException(rpc_status.error_message() + ": " + rpc_status.error_details());
}

SMSG_DesfireISOAuth_Step1
IKSRPCClient::desfire_auth_iso_step1(CMSG_DesfireISOAuth_Step1 req)
{
    grpc::ClientContext context;

    SMSG_DesfireISOAuth_Step1 rep;
    grpc::Status rpc_status = stub_->DESFireISOAuth1(&context, req, &rep);
    if (rpc_status.ok())
    {
        return rep;
    }
    throw RPCException(rpc_status.error_message() + ": " + rpc_status.error_details());
}

SMSG_DesfireAuth_Step2 IKSRPCClient::desfire_auth_iso_step2(CMSG_DesfireAuth_Step2 req)
{
    grpc::ClientContext context;

    SMSG_DesfireAuth_Step2 rep;
    grpc::Status rpc_status = stub_->DESFireISOAuth2(&context, req, &rep);
    if (rpc_status.ok())
    {
        return rep;
    }
    throw RPCException(rpc_status.error_message() + ": " + rpc_status.error_details());
}

SMSG_DesfireAESAuth_Step1
IKSRPCClient::desfire_auth_aes_step1(CMSG_DesfireAESAuth_Step1 req)
{
    grpc::ClientContext context;

    SMSG_DesfireAESAuth_Step1 rep;
    grpc::Status rpc_status = stub_->DESFireAESAuth1(&context, req, &rep);
    if (rpc_status.ok())
    {
        return rep;
    }
    throw RPCException(rpc_status.error_message() + ": " + rpc_status.error_details());
}

SMSG_DesfireAuth_Step2 IKSRPCClient::desfire_auth_aes_step2(CMSG_DesfireAuth_Step2 req)
{
    grpc::ClientContext context;

    SMSG_DesfireAuth_Step2 rep;
    grpc::Status rpc_status = stub_->DESFireAESAuth2(&context, req, &rep);
    if (rpc_status.ok())
    {
        return rep;
    }
    throw RPCException(rpc_status.error_message() + ": " + rpc_status.error_details());
}

SMSG_DesfireChangeKey IKSRPCClient::desfire_change_key(CMSG_DesfireChangeKey req)
{
    grpc::ClientContext context;

    SMSG_DesfireChangeKey rep;
    grpc::Status rpc_status = stub_->DESFireChangeKey(&context, req, &rep);
    if (rpc_status.ok())
    {
        return rep;
    }
    throw RPCException(rpc_status.error_message() + ": " + rpc_status.error_details());
}

RemoteCryptoIKSProvider::RemoteCryptoIKSProvider(IslogKeyServer::IKSConfig config)
    : iks_rpc_client_(config)
{
}

bool RemoteCryptoIKSProvider::verify_signature(const SignatureResult &sr,
                                               const std::string &pubkey_pem)
{
    return false;
}

ByteVector RemoteCryptoIKSProvider::aes_encrypt(const ByteVector &in,
                                                const std::string &key_name,
                                                const ByteVector &iv)
{
    return iks_rpc_client_.aes_encrypt(in, key_name, iv);
}

ByteVector RemoteCryptoIKSProvider::aes_decrypt(const ByteVector &in,
                                                const std::string &key_name,
                                                const ByteVector &iv,
                                                SignatureResult *out_signature)
{
    return iks_rpc_client_.aes_decrypt(in, key_name, iv);
}

void RemoteCryptoIKSProvider::iso_authenticate_step1(
    const std::string &key_identity, const ByteVector &random_picc,
    const MyDivInfo &div_info, bool &out_success, ByteVector &out_random2,
    ByteVector &out_encrypted_cryptogram, ByteVector &out_auth_context_id)
{
    CMSG_DesfireISOAuth_Step1 req;
    req.set_key_uuid(key_identity);
    req.set_random_picc(BufferHelper::getStdString(random_picc));
    *req.mutable_diversification() = convert_div_info(div_info);

    auto rep    = iks_rpc_client_.desfire_auth_iso_step1(req);
    out_success = rep.success();
    out_random2 = ByteVector(rep.random2().begin(), rep.random2().end());
    out_encrypted_cryptogram =
        ByteVector(rep.encrypted_cryptogram().begin(), rep.encrypted_cryptogram().end());
    out_auth_context_id =
        ByteVector(rep.auth_context_id().begin(), rep.auth_context_id().end());
}

void RemoteCryptoIKSProvider::iso_authenticate_step2(
    const std::string &key_identity, const ByteVector &picc_cryptogram,
    const ByteVector &auth_context_id, const MyDivInfo &div_info, bool &out_success,
    ByteVector &out_session_key, ByteVector &out_session_key_ref)
{
    CMSG_DesfireAuth_Step2 req;
    req.set_key_uuid(key_identity);
    req.set_auth_context_id(BufferHelper::getStdString(auth_context_id));
    req.set_picc_cryptogram(BufferHelper::getStdString(picc_cryptogram));
    *req.mutable_diversification() = convert_div_info(div_info);

    auto rep        = iks_rpc_client_.desfire_auth_iso_step2(req);
    out_success     = rep.success();
    out_session_key = ByteVector(rep.session_key().begin(), rep.session_key().end());
    out_session_key_ref =
        ByteVector(rep.session_key_ref().begin(), rep.session_key_ref().end());
}

void RemoteCryptoIKSProvider::aes_authenticate_step1(
    const std::string &key_identity, const ByteVector &encrypted_random_picc,
    const MyDivInfo &div_info, bool &out_success, ByteVector &out_encrypted_cryptogram,
    ByteVector &out_auth_context_id)
{
    CMSG_DesfireAESAuth_Step1 req;
    req.set_key_uuid(key_identity);
    req.set_encrypted_random_picc(BufferHelper::getStdString(encrypted_random_picc));
    *req.mutable_diversification() = convert_div_info(div_info);

    auto rep    = iks_rpc_client_.desfire_auth_aes_step1(req);
    out_success = rep.success();
    out_encrypted_cryptogram =
        ByteVector(rep.encrypted_cryptogram().begin(), rep.encrypted_cryptogram().end());
    out_auth_context_id =
        ByteVector(rep.auth_context_id().begin(), rep.auth_context_id().end());
}

void RemoteCryptoIKSProvider::aes_authenticate_step2(
    const std::string &key_identity, const ByteVector &picc_cryptogram,
    const ByteVector &auth_context_id, const MyDivInfo &div_info, bool &out_success,
    ByteVector &out_session_key, ByteVector &out_session_key_ref)
{
    CMSG_DesfireAuth_Step2 req;
    req.set_key_uuid(key_identity);
    req.set_auth_context_id(BufferHelper::getStdString(auth_context_id));
    req.set_picc_cryptogram(BufferHelper::getStdString(picc_cryptogram));
    *req.mutable_diversification() = convert_div_info(div_info);

    auto rep        = iks_rpc_client_.desfire_auth_aes_step2(req);
    out_success     = rep.success();
    out_session_key = ByteVector(rep.session_key().begin(), rep.session_key().end());
    out_session_key_ref =
        ByteVector(rep.session_key_ref().begin(), rep.session_key_ref().end());
}

void RemoteCryptoIKSProvider::change_key(const std::string &old_key_identity,
                                         const std::string &new_key_identity,
                                         bool change_same_key,
                                         const std::string &session_key_uuid,
                                         const ByteVector &session_key,
                                         const MyDivInfo &old_key_div,
                                         const MyDivInfo &new_key_div, uint8_t key_no,
                                         const ByteVector &iv, ByteVector &out_cryptogram)
{
    CMSG_DesfireChangeKey req;
    req.set_old_key_uuid(old_key_identity);
    req.set_new_key_uuid(new_key_identity);
    req.set_change_same_key(change_same_key);
    req.set_session_key_uuid(session_key_uuid);
    req.set_session_key(BufferHelper::getStdString(session_key));
    *req.mutable_old_key_div() = convert_div_info(old_key_div);
    *req.mutable_new_key_div() = convert_div_info(new_key_div);
    req.set_key_number(key_no);
    req.set_iv(BufferHelper::getStdString(iv));

    auto rep       = iks_rpc_client_.desfire_change_key(req);
    out_cryptogram = ByteVector(rep.cryptogram().begin(), rep.cryptogram().end());
}

KeyDiversificationInfo RemoteCryptoIKSProvider::convert_div_info(const MyDivInfo &in)
{
    KeyDiversificationInfo kdi;
    if (in.div_type == "NXPAV2")
        kdi.set_div_type(KeyDiversificationInfo::Type::KeyDiversificationInfo_Type_AV2);
    kdi.set_div_input(BufferHelper::getStdString(in.div_input));
    return kdi;
}
}
}

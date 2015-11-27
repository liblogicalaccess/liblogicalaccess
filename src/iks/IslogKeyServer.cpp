#include <boost/asio.hpp>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/iks/packet/GenRandom.hpp>
#include <logicalaccess/iks/packet/AesEncrypt.hpp>
#include <logicalaccess/iks/packet/DesEncrypt.hpp>
#include <logicalaccess/iks/packet/DesfireAuth.hpp>
#include <logicalaccess/settings.hpp>
#include <logicalaccess/iks/packet/DesfireChangeKey.hpp>
#include "logicalaccess/iks/IslogKeyServer.hpp"

using namespace logicalaccess;
using namespace logicalaccess::iks;

IslogKeyServer &IslogKeyServer::fromGlobalSettings()
{
    static iks::IslogKeyServer iks(pre_configuration_.ip, pre_configuration_.port,
                                   pre_configuration_.client_cert,
                                   pre_configuration_.client_key,
                                   pre_configuration_.root_ca);
    return iks;
}

IslogKeyServer::IslogKeyServer(const std::string &ip, uint16_t port,
                               const std::string &client_cert,
                               const std::string &client_key,
                               const std::string &root_ca)
    : ssl_ctx_(boost::asio::ssl::context::tlsv12_client)
{
    ssl_ctx_.use_certificate_file(client_cert,
                                  boost::asio::ssl::context_base::file_format::pem);
    ssl_ctx_.use_private_key_file(client_key,
                                  boost::asio::ssl::context_base::file_format::pem);

    ssl_ctx_.load_verify_file(root_ca);
    ssl_ctx_.set_verify_mode(boost::asio::ssl::verify_peer);

    transport_ = std::unique_ptr<SSLTransport>(new SSLTransport(ssl_ctx_));

    transport_->setIpAddress(ip);
    transport_->setPort(port);

    if (!transport_->connect(2500))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Failed to connect to Islog Key Server.");
}

std::vector<uint8_t> IslogKeyServer::get_random(size_t sz)
{
    assert(sz <= UINT16_MAX);
    GenRandomCommand cmd;
    cmd.nb_bytes_ = static_cast<uint16_t>(sz);

    transport_->send(cmd.serialize());
    auto ret = std::dynamic_pointer_cast<GenRandomResponse>(recv());
    assert(ret && ret->opcode_ == SMSG_OP_GENRANDOM);
    LOG(INFOS) << ret->bytes_;
    return ret->bytes_;
}

std::vector<uint8_t> IslogKeyServer::aes_encrypt(const std::vector<uint8_t> &in,
                                                 const std::string &key_name,
                                                 const std::array<uint8_t, 16> &iv)
{
    AesEncryptCommand cmd;
    cmd.key_name_ = key_name;
    cmd.iv_       = iv;
    cmd.payload_  = in;

    transport_->send(cmd.serialize());
    auto ret = std::dynamic_pointer_cast<AesEncryptResponse>(recv());
    assert(ret && ret->opcode_ == SMSG_OP_AES_ENCRYPT);
    if (ret->status_ != SMSG_STATUS_SUCCESS)
    {
        THROW_EXCEPTION_WITH_LOG(IKSException,
                                 "AESEncrypt failed: " + strstatus(ret->status_));
    }
    LOG(INFOS) << "Encrypted: " << ret->bytes_;
    return ret->bytes_;
}

std::vector<uint8_t> IslogKeyServer::aes_decrypt(const std::vector<uint8_t> &in,
                                                 const std::string &key_name,
                                                 const std::vector<uint8_t> &iv)
{
    if (iv.size() != 16)
    {
        THROW_EXCEPTION_WITH_LOG(IKSException, "Invalid IV size.");
    }
    std::array<uint8_t, 16> iv_array;
    memcpy(&iv_array[0], &iv[0], 16);
    return aes_decrypt(in, key_name, iv_array);
}

std::vector<uint8_t> IslogKeyServer::aes_decrypt(const std::vector<uint8_t> &in,
                                                 const std::string &key_name,
                                                 const std::array<uint8_t, 16> &iv)
{
    AesEncryptCommand cmd;
    cmd.decrypt_  = true;
    cmd.key_name_ = key_name;
    cmd.iv_       = iv;
    cmd.payload_  = in;

    transport_->send(cmd.serialize());
    auto ret = std::dynamic_pointer_cast<AesEncryptResponse>(recv());
    assert(ret && ret->opcode_ == SMSG_OP_AES_ENCRYPT);
    if (ret->status_ != SMSG_STATUS_SUCCESS)
    {
        THROW_EXCEPTION_WITH_LOG(IKSException,
                                 "AESDecrypt failed: " + strstatus(ret->status_));
    }
    LOG(INFOS) << "Decrypted: " << ret->bytes_;
    return ret->bytes_;
}

std::vector<uint8_t>
IslogKeyServer::des_cbc_encrypt(const std::vector<uint8_t> &in,
                                const std::string &key_name,
                                const std::array<uint8_t, 8> &iv)
{
    return des_crypto(in, key_name, iv, false, false);
}

std::vector<uint8_t>
IslogKeyServer::des_cbc_decrypt(const std::vector<uint8_t> &in,
                                const std::string &key_name,
                                const std::array<uint8_t, 8> &iv)
{
    return des_crypto(in, key_name, iv, false, true);
}

std::vector<uint8_t>
IslogKeyServer::des_ecb_encrypt(const std::vector<uint8_t> &in,
                                const std::string &key_name,
                                const std::array<uint8_t, 8> &iv)
{
    return des_crypto(in, key_name, iv, true, false);
}

std::vector<uint8_t>
IslogKeyServer::des_ecb_decrypt(const std::vector<uint8_t> &in,
                                const std::string &key_name,
                                const std::array<uint8_t, 8> &iv)
{
    return des_crypto(in, key_name, iv, true, true);
}

std::vector<uint8_t> IslogKeyServer::des_crypto(const std::vector<uint8_t> &in,
                                                const std::string &key_name,
                                                const std::array<uint8_t, 8> &iv,
                                                bool use_ecb, bool decrypt)
{
    DesEncryptCommand cmd;
    cmd.key_name_ = key_name;
    cmd.iv_       = iv;
    cmd.payload_ = in;
    cmd.flags_ =
        use_ecb ? COMMAND_DES_ENCRYPT_FLAG_ECB : COMMAND_DES_ENCRYPT_FLAG_CBC;
    cmd.decrypt_ = decrypt;

    transport_->send(cmd.serialize());
    auto ret = std::dynamic_pointer_cast<DesEncryptResponse>(recv());
    assert(ret && ret->opcode_ == SMSG_OP_DES_ENCRYPT);
    if (ret->status_ != SMSG_STATUS_SUCCESS)
    {
        THROW_EXCEPTION_WITH_LOG(IKSException,
                                 "DESCrypto failed: " + strstatus(ret->status_));
    }
    return ret->bytes_;
}

void IslogKeyServer::send_command(const BaseCommand &cmd)
{
    transport_->send(cmd.serialize());
}

std::shared_ptr<BaseResponse> IslogKeyServer::recv()
{
    uint32_t packet_size;
    uint16_t opcode;
    uint16_t status;

    bool has_size   = false;
    bool has_opcode = false;
    bool has_status = false;
    size_t needle   = 0;
    std::vector<uint8_t> buffer;
    // 3 attempts of 1sec
    for (int i = 0; i < 3; ++i)
    {
        try
        {
            auto tmpbuf = transport_->receive(3000);
            buffer.insert(buffer.end(), tmpbuf.begin(), tmpbuf.end());
        }
        catch (LibLogicalAccessException &)
        {
            // probably timeout error;
            continue;
        }

        // size
        if (!has_size && buffer.size() - needle >= sizeof(packet_size))
        {
            memcpy(&packet_size, &buffer[needle], sizeof(packet_size));
            packet_size = ntohl(packet_size);
            needle += sizeof(packet_size);
            has_size = true;
        }

        // opcode
        if (!has_opcode && buffer.size() - needle >= sizeof(opcode))
        {
            memcpy(&opcode, &buffer[needle], sizeof(opcode));
            opcode = ntohs(opcode);
            needle += sizeof(opcode);
            has_opcode = true;
        }

        // status code
        if (!has_status && buffer.size() - needle >= sizeof(status))
        {
            memcpy(&status, &buffer[needle], sizeof(status));
            status = ntohs(status);
            needle += sizeof(status);
            has_status = true;
        }

        if (has_size && has_opcode && has_status &&
            buffer.size() - needle >= packet_size - 8) // total size of header
        {
            LOG(INFOS) << "Size: " << packet_size << ". Op: " << opcode
                       << ". St: " << status << ". Bufsize: " << buffer.size()
                       << ". Needle: " << needle;

            return build_response(
                packet_size, opcode, status,
                std::vector<uint8_t>(buffer.begin() + needle, buffer.end()));
        }
    }
    return nullptr;
}

std::shared_ptr<BaseResponse>
IslogKeyServer::build_response(uint32_t size, uint16_t opcode, uint16_t status,
                               const std::vector<uint8_t> &data)
{
    std::shared_ptr<BaseResponse> resp;
    switch (opcode)
    {
    case SMSG_OP_GENRANDOM:
        resp = std::make_shared<GenRandomResponse>(status, data);
        break;

    case SMSG_OP_AES_ENCRYPT:
        resp = std::make_shared<AesEncryptResponse>(status, data);
        break;

    case SMSG_OP_DESFIRE_AUTH:
        resp = std::make_shared<DesfireAuthResponse>(status, data);
        break;

    case SMSG_OP_DES_ENCRYPT:
        resp = std::make_shared<DesEncryptResponse>(status, data);
        break;

    case SMSG_OP_DESFIRE_CHANGEKEY:
        resp = std::make_shared<DesfireChangeKeyResponse>(status, data);
        break;
    default:
        LOG(WARNINGS) << "Unkown opcode " << opcode << " from server.";
    }

    return resp;
}

IslogKeyServer::IKSConfig IslogKeyServer::pre_configuration_;

void IslogKeyServer::configureGlobalInstance(const std::string &ip, uint16_t port,
                                             const std::string &client_cert,
                                             const std::string &client_key,
                                             const std::string &root_ca)
{
    IKSConfig cfg;
    cfg.ip                             = ip;
    cfg.port                           = port;
    cfg.client_cert                    = client_cert;
    cfg.client_key                     = client_key;
    cfg.root_ca                        = root_ca;
    IslogKeyServer::pre_configuration_ = cfg;
}

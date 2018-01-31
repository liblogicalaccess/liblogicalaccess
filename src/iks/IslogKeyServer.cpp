#include <logicalaccess/iks/IslogKeyServer.hpp>
#include <boost/asio.hpp>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/settings.hpp>

using namespace logicalaccess;
using namespace logicalaccess::iks;

IslogKeyServer::IKSConfig IslogKeyServer::pre_configuration_;

void IslogKeyServer::configureGlobalInstance(const std::string &ip, uint16_t port,
                                             const std::string &client_cert,
                                             const std::string &client_key,
                                             const std::string &root_ca)
{
    pre_configuration_ = IKSConfig(ip, port, client_cert, client_key, root_ca);
}

IslogKeyServer::IKSConfig IslogKeyServer::get_global_config()
{
    return pre_configuration_;
}

IslogKeyServer::IKSConfig::IKSConfig(const std::string &ip, uint16_t port,
                                     const std::string &client_cert,
                                     const std::string &client_key,
                                     const std::string &root_ca)
{
    this->ip          = ip;
    this->port        = port;
    this->client_cert = client_cert;
    this->client_key  = client_key;
    this->root_ca     = root_ca;
}

std::string read_file_content(const std::string &filepath)
{
    std::ifstream ifs(filepath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

    if (ifs.good())
    {
        std::ifstream::pos_type fileSize = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        std::vector<char> bytes(fileSize);
        ifs.read(bytes.data(), fileSize);
        return std::string(bytes.data(), fileSize);
    }
    throw LibLogicalAccessException("Cannot open file {" + filepath + "}");
}

std::string IslogKeyServer::IKSConfig::get_client_cert_pem() const
{
    return read_file_content(client_cert);
}

std::string IslogKeyServer::IKSConfig::get_client_key_pem() const
{
    return read_file_content(client_key);
}

std::string IslogKeyServer::IKSConfig::get_root_ca_pem() const
{
    return read_file_content(root_ca);
}

std::string IslogKeyServer::IKSConfig::get_target() const
{
    std::stringstream ss;

    ss << ip << ':' << port;
    return ss.str();
}

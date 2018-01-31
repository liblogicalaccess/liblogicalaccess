/*
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include "logicalaccess/plugins/crypto/signature_helper.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/iks/signature.hpp"

namespace logicalaccess
{
namespace iks
{
bool SignatureResult::verify(const std::string &pem_pubkey)
{
    return SignatureHelper::verify_sha512(signature_description_.SerializeAsString(),
                                          BufferHelper::getStdString(signature_),
                                          pem_pubkey);
}
}
}
*/

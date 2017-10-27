/**
 * \file openssl_symmetric_cipher.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Open SSL symmetric cipher base class.
 */

#include <stdexcept>
#include "logicalaccess/crypto/openssl.hpp"
#include "logicalaccess/crypto/openssl_symmetric_cipher.hpp"
#include "logicalaccess/crypto/openssl_symmetric_cipher_context.hpp"
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/logs.hpp"
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/crypto/openssl_exception.hpp"
#include "logicalaccess/crypto/sha.hpp"

#include <cstring>

namespace logicalaccess
{
namespace openssl
{
OpenSSLSymmetricCipher::OpenSSLSymmetricCipher(EncMode _mode)
    : d_mode(_mode)
{
    OpenSSLInitializer::GetInstance();
}

OpenSSLSymmetricCipherContext
OpenSSLSymmetricCipher::start(Method method, const SymmetricKey &key,
                              const InitializationVector &iv, bool padding) const
{
    OpenSSLSymmetricCipherContext context(method);

    int r = 0;

    const EVP_CIPHER *evpCipher = getEVPCipher(key);

    EXCEPTION_ASSERT(evpCipher, std::invalid_argument,
                     "No cipher found that can use the supplied key");

    switch (context.method())
    {
    case M_ENCRYPT:
    {
        r = EVP_EncryptInit_ex(context.ctx(), evpCipher, nullptr, &key.data()[0],
                               &iv.data()[0]);
        break;
    }
    case M_DECRYPT:
    {
        r = EVP_DecryptInit_ex(context.ctx(), evpCipher, nullptr, &key.data()[0],
                               &iv.data()[0]);
        break;
    }
    default: { THROW_EXCEPTION_WITH_LOG(std::runtime_error, "Unhandled method");
    }
    }

    EXCEPTION_ASSERT_WITH_LOG(r == 1, OpenSSLException, "");

    context.data().clear();

    context.setPadding(padding);

    return context;
}

void OpenSSLSymmetricCipher::update(OpenSSLSymmetricCipherContext &context,
                                    const ByteVector &src)
{
    int r      = 0;
    int outlen = 0;

    unsigned char *buf = new unsigned char[src.size() + context.blockSize()];

    switch (context.method())
    {
    case M_ENCRYPT:
    {
        r = EVP_EncryptUpdate(context.ctx(), buf, &outlen, &src[0],
                              static_cast<int>(src.size()));
        break;
    }
    case M_DECRYPT:
    {
        r = EVP_DecryptUpdate(context.ctx(), buf, &outlen, &src[0],
                              static_cast<int>(src.size()));
        break;
    }
    default: { THROW_EXCEPTION_WITH_LOG(std::runtime_error, "Unhandled method");
    }
    }

    if (r != 1)
    {
        delete[] buf;
        THROW_EXCEPTION_WITH_LOG(OpenSSLException, "");
    }

    context.data().insert(context.data().end(), buf, buf + outlen);
    delete[] buf;
}

ByteVector OpenSSLSymmetricCipher::stop(OpenSSLSymmetricCipherContext &context)
{
    int r      = 0;
    int outlen = 0;

    unsigned char *buf = new unsigned char[context.blockSize()];

    switch (context.method())
    {
    case M_ENCRYPT:
    {
        r = EVP_EncryptFinal_ex(context.ctx(), buf, &outlen);
        break;
    }
    case M_DECRYPT:
    {
        r = EVP_DecryptFinal_ex(context.ctx(), buf, &outlen);
        break;
    }
    default: { THROW_EXCEPTION_WITH_LOG(std::runtime_error, "Unhandled method");
    }
    }

    if (r != 1)
    {
        delete[] buf;
        THROW_EXCEPTION_WITH_LOG(OpenSSLException, "OpenSSL Error.");
    }

    context.data().insert(context.data().end(), buf, buf + outlen);
    delete[] buf;

    ByteVector data = context.data();

    context.reset();

    return data;
}

void OpenSSLSymmetricCipher::cipher(const ByteVector &src, ByteVector &dest,
                                    const SymmetricKey &key,
                                    const InitializationVector &iv, bool padding)
{
    OpenSSLSymmetricCipherContext context = start(M_ENCRYPT, key, iv, padding);

    update(context, src);

    dest = stop(context);
}

void OpenSSLSymmetricCipher::decipher(const ByteVector &src, ByteVector &dest,
                                      const SymmetricKey &key,
                                      const InitializationVector &iv, bool padding)
{
    OpenSSLSymmetricCipherContext context = start(M_DECRYPT, key, iv, padding);

    update(context, src);

    dest = stop(context);
}
}
}
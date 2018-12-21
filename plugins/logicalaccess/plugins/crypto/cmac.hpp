#ifndef CMAC_HPP__
#define CMAC_HPP__

#include <vector>
#include <memory>
#include <logicalaccess/plugins/crypto/openssl.hpp>
#include <logicalaccess/plugins/crypto/openssl_symmetric_cipher.hpp>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{
namespace openssl
{
/**
 * NIST SP 800-38B / ISO 9797-1:2011 MAC Algorithm 5
*/
class LLA_CRYPTO_API CMACCrypto
{
  public:
    CMACCrypto()
    {
        OpenSSLInitializer::GetInstance();
    }
    ~CMACCrypto()
    {
    }

    /*
     * \brief Calculate CMAC for a crypto algorithm and key.
     * \param key The key to use.
     * \param crypto The crypto algorithm (3DES, AES, ...).
     * \param data The data buffer to calculate CMAC.
     * \param padding_size The padding size.
     * \return The CMAC result for the message.
     */
    static ByteVector cmac(const ByteVector &key, std::string crypto,
                           const ByteVector &data, const ByteVector &iv = {},
                           int padding_size = 0);

    /**
     * \brief  Return data part for the encrypted communication mode.
     * \param key The key to use, shall be the session key from the previous
     * authentication
     * \param cipher The cypher to use
     * \param data The data source buffer to calculate MAC
     * \param lastIV The last initialisation vector
     * \return The MAC result for the message.
     */
    static ByteVector cmac(const ByteVector &key,
                           std::shared_ptr<SymmetricCipher> cipherMAC,
                           const ByteVector &data, const ByteVector &lastIV = {},
                           unsigned int padding_size = 0, bool forceK2Use = false);

    /**
     * CMAC computation that delegates AES encryption to IKS.
     * @param iks_key_name
     * @param data
     * @param iv
     * @param padding_size
     * @return
     */
    static ByteVector cmac_iks(const std::string &iks_key_name, const ByteVector &data,
                               const ByteVector &lastIv, unsigned int padding_size,
                               const std::shared_ptr<RemoteCrypto> &remote_crypto);

    /**
     * \brief Shift a string.
     * \param buf The buffer string
     * \param xorparam The optional xor for the string.
     */
    static ByteVector shift_string(const ByteVector &buf, unsigned char xorparam = 0x00);
};
}
}

#endif
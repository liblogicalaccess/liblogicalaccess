#include <vector>
#include <memory>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/crypto/des_cipher.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/des_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/des_initialization_vector.hpp>
#include <logicalaccess/bufferhelper.hpp>

namespace logicalaccess
{
namespace openssl
{
ByteVector CMACCrypto::cmac(const ByteVector &key, std::string crypto,
                            const ByteVector &data, const ByteVector &iv,
                            int padding_size)
{
    std::shared_ptr<OpenSSLSymmetricCipher> cipherMAC;
    if (crypto == "des" || crypto == "3des")
    {
        cipherMAC.reset(new DESCipher(OpenSSLSymmetricCipher::ENC_MODE_CBC));
    }
    else if (crypto == "aes")
    {
        cipherMAC.reset(new AESCipher(OpenSSLSymmetricCipher::ENC_MODE_CBC));
    }
    else
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Wrong crypto mechanism: " + crypto);
    }

    return CMACCrypto::cmac(key, cipherMAC, data, iv, padding_size);
}

ByteVector CMACCrypto::cmac(const ByteVector &key,
                            std::shared_ptr<SymmetricCipher> cipherMAC,
                            const ByteVector &data, const ByteVector &lastIV,
                            unsigned int padding_size, bool forceK2Use)
{
    std::shared_ptr<OpenSSLSymmetricCipher> cipherK1K2;

    std::shared_ptr<SymmetricKey> symkey;
    std::shared_ptr<InitializationVector> iv;
    // 3DES
    if (std::dynamic_pointer_cast<DESCipher>(cipherMAC))
    {
        cipherK1K2.reset(new DESCipher(OpenSSLSymmetricCipher::ENC_MODE_ECB));

        symkey.reset(new DESSymmetricKey(DESSymmetricKey::createFromData(key)));
        iv.reset(new DESInitializationVector(DESInitializationVector::createNull()));
    }
    // AES
    else
    {
        cipherK1K2.reset(new AESCipher(OpenSSLSymmetricCipher::ENC_MODE_ECB));

        symkey.reset(new AESSymmetricKey(AESSymmetricKey::createFromData(key)));
        iv.reset(new AESInitializationVector(AESInitializationVector::createNull()));
    }

    unsigned char Rb;

    // TDES
    if (cipherMAC->getBlockSize() == 8)
    {
        Rb = 0x1b;
    }
    // AES
    else
    {
        Rb = 0x87;
    }

    ByteVector blankbuf;
    blankbuf.resize(cipherK1K2->getBlockSize(), 0x00);
    ByteVector L;
    cipherK1K2->cipher(blankbuf, L, *symkey.get(), *iv.get(), false);

    ByteVector K1;
    if ((L[0] & 0x80) == 0x00)
    {
        K1 = shift_string(L);
    }
    else
    {
        K1 = shift_string(L, Rb);
    }

    ByteVector K2;
    if ((K1[0] & 0x80) == 0x00)
    {
        K2 = shift_string(K1);
    }
    else
    {
        K2 = shift_string(K1, Rb);
    }

	if (padding_size == 0)
    {
        padding_size = cipherMAC->getBlockSize();
    }
    int pad = (padding_size - (data.size() % padding_size)) % padding_size;
    if (data.size() == 0)
        pad = padding_size;

    ByteVector padded_data = data;
    if (pad > 0)
    {
        padded_data.push_back(0x80);
        if (pad > 1)
        {
            for (int i = 0; i < (pad - 1); ++i)
            {
                padded_data.push_back(0x00);
            }
        }
    }

    // XOR with K1
    if (pad == 0 && !forceK2Use)
    {
        for (unsigned int i = 0; i < K1.size(); ++i)
        {
            padded_data[padded_data.size() - K1.size() + i] = static_cast<unsigned char>(
                padded_data[padded_data.size() - K1.size() + i] ^ K1[i]);
        }
    }
    // XOR with K2
    else
    {
        for (unsigned int i = 0; i < K2.size(); ++i)
        {
            padded_data[padded_data.size() - K2.size() + i] = static_cast<unsigned char>(
                padded_data[padded_data.size() - K2.size() + i] ^ K2[i]);
        }
    }

    ByteVector ret;
    if (lastIV.size() > 0)
	{
		// 3DES
		if (std::dynamic_pointer_cast<DESCipher>(cipherMAC))
		{
			iv.reset(
				new DESInitializationVector(DESInitializationVector::createFromData(lastIV)));
		}
		// AES
		else
		{
			iv.reset(
				new AESInitializationVector(AESInitializationVector::createFromData(lastIV)));
		}
	}
    cipherMAC->cipher(padded_data, ret, *symkey.get(), *iv.get(), false);

	if (ret.size() > cipherMAC->getBlockSize())
    {
        ret = ByteVector(ret.end() - cipherMAC->getBlockSize(), ret.end());
    }
    return ret;
}

ByteVector CMACCrypto::shift_string(const ByteVector &buf, unsigned char xorparam)
{
    ByteVector ret = buf;

    for (unsigned int i = 0; i < ret.size() - 1; ++i)
    {
        ret[i] = ret[i] << 1;
        // add the carry over bit
        ret[i] = ret[i] | ((ret[i + 1] & 0x80) ? 0x01 : 0x00);
    }

    ret[ret.size() - 1] = ret[ret.size() - 1] << 1;

    if (xorparam != 0x00)
    {
        ret[ret.size() - 1] = static_cast<unsigned char>(ret[ret.size() - 1] ^ xorparam);
    }

    return ret;
}
}
}
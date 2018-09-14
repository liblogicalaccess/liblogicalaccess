#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/cards/PKCSkeystorage.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include "logicalaccess/services/aes_crypto_service.hpp"
#include "logicalaccess/key.hpp"

namespace logicalaccess
{

ByteVector AESCryptoService::aes_encrypt(const ByteVector &data, const ByteVector &iv,
                                         std::shared_ptr<Key> key)
{
    return perform_operation(data, adjust_iv(iv), key, true);
}

ByteVector AESCryptoService::in_memory(const ByteVector &data, const ByteVector &iv,
                                       std::shared_ptr<Key> key, bool encrypt)
{
    EXCEPTION_ASSERT_WITH_LOG(key->getBytes().size() == 16, LibLogicalAccessException,
                              "Key length is not valid for AES crypto");

    if (encrypt)
        return AESHelper::AESEncrypt(data, key->getBytes(), iv);
    else
        return AESHelper::AESDecrypt(data, key->getBytes(), iv);
}

ByteVector AESCryptoService::with_pkcs(const ByteVector &data, const ByteVector &iv,
                                       std::shared_ptr<Key> key, bool encrypt)
{
    std::shared_ptr<PKCSKeyStorage> storage =
        std::dynamic_pointer_cast<PKCSKeyStorage>(key->getKeyStorage());
    EXCEPTION_ASSERT_WITH_LOG(storage, LibLogicalAccessException, "No key storage.");

    auto kid                 = storage->get_key_id();
    auto pkcs_crypto_service = LibraryManager::getInstance()->getPKCSAESCrypto(
        storage->get_proteccio_conf_dir(), storage->get_pkcs_shared_object_path());

    if (encrypt)
        return pkcs_crypto_service->aes_encrypt(data, iv, key);
    else
        return pkcs_crypto_service->aes_decrypt(data, iv, key);
}

ByteVector AESCryptoService::perform_operation(const ByteVector &data,
                                               const ByteVector &iv,
                                               std::shared_ptr<Key> key, bool encrypt)
{
    switch (key->getKeyStorage()->getType())
    {
    case KST_COMPUTER_MEMORY: return in_memory(data, iv, key, encrypt);

    case KST_PKCS: return with_pkcs(data, iv, key, encrypt);

    default:
        throw LibLogicalAccessException("Key type not supported in AESCryptoService");
    }
}

ByteVector AESCryptoService::aes_decrypt(const ByteVector &data, const ByteVector &iv,
                                         std::shared_ptr<Key> key)
{
    return perform_operation(data, adjust_iv(iv), key, false);
}

ByteVector AESCryptoService::adjust_iv(const ByteVector &iv)
{
    if (iv.empty())
        return ByteVector(16, 0);

    return iv;
}
}

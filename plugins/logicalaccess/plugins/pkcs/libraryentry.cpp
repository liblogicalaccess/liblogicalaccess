#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/services/aes_crypto_service.hpp>
#include "cppkcs11/services/crypto_service.hpp"
#include "cppkcs11/services/object_service.hpp"
#include "cppkcs11/session.hpp"
#include <logicalaccess/cards/pkcskeystorage.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include "cppkcs11/cppkcs11.hpp"
#include "logicalaccess/key.hpp"

namespace logicalaccess
{
class AESCryptoPKCSProvider : public IAESCryptoService
{
  public:
    ByteVector aes_encrypt(const ByteVector &data, const ByteVector &iv,
                           std::shared_ptr<logicalaccess::Key> key) override
    {
        std::shared_ptr<PKCSKeyStorage> storage =
            std::dynamic_pointer_cast<PKCSKeyStorage>(key->getKeyStorage());
        EXCEPTION_ASSERT_WITH_LOG(storage, LibLogicalAccessException, "No key storage.");

        auto session        = cppkcs::open_session(storage->get_slot_id(), 0);
        std::string pw_copy = storage->get_pkcs_session_password();
        session.login(cppkcs::SecureString(std::move(pw_copy)));

        auto pkcs_key = find_key(session, *storage);

        cppkcs::CryptoService cs(session);

        cppkcs::SecureString secure_data(data.data(), data.size());
        return cs.aes_encrypt(secure_data, iv, pkcs_key);
    }

    ByteVector aes_decrypt(const ByteVector &data, const ByteVector &iv,
                           std::shared_ptr<logicalaccess::Key> key) override
    {
        std::shared_ptr<PKCSKeyStorage> storage =
            std::dynamic_pointer_cast<PKCSKeyStorage>(key->getKeyStorage());
        EXCEPTION_ASSERT_WITH_LOG(storage, LibLogicalAccessException, "No key storage.");

        auto session        = cppkcs::open_session(storage->get_slot_id(), 0);
        std::string pw_copy = storage->get_pkcs_session_password();
        session.login(cppkcs::SecureString(std::move(pw_copy)));

        auto pkcs_key = find_key(session, *storage);

        cppkcs::CryptoService cs(session);

        auto clear = cs.aes_decrypt(data, iv, pkcs_key);
        return ByteVector(clear.data(), clear.data() + clear.size());
    }

  private:
    cppkcs::Object find_key(cppkcs::Session &session, const PKCSKeyStorage &storage)
    {
        cppkcs::ObjectService os(session);

        return std::move(
            os.find_objects(cppkcs::make_attribute<CKA_ID>(storage.get_key_id())).at(0));
    }
};
}

extern "C" {

char *getLibraryName()
{
    return (char *)"PKCSAESCrypto";
}

static bool pkcs_initialized = false;

void
getPKCSAESCrypto(std::shared_ptr<logicalaccess::IAESCryptoService> &aes_crypto,
                 const std::string &pkcs_shared_object_path)
{
    if (!pkcs_initialized)
    {
        cppkcs::load_pkcs(pkcs_shared_object_path);
        cppkcs::initialize();
        pkcs_initialized = true;
    }
    aes_crypto = std::make_shared<logicalaccess::AESCryptoPKCSProvider>();
}
}

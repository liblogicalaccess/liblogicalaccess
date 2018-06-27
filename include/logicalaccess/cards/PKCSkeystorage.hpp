#pragma once

#include <logicalaccess/cards/keystorage.hpp>
#include <logicalaccess/iks/IslogKeyServer.hpp>

namespace logicalaccess
{
/**
 * A storage to represent a key store in a PKCS token.
 */
class LIBLOGICALACCESS_API PKCSKeyStorage : public KeyStorage
{
  public:
    KeyStorageType getType() const override
    {
        return KST_PKCS;
    }

    std::string getDefaultXmlNodeName() const override
    {
        return "PKCSStorage";
    }

    const ByteVector &get_key_id() const
    {
        return pkcs_object_id_;
    }

    void set_key_id(const ByteVector &keyid)
    {
        pkcs_object_id_ = keyid;
    }

    size_t get_slot_id() const
    {
        return slot_id_;
    }

    void set_slot_id(size_t slot_id)
    {
        slot_id_ = slot_id;
    }

    const std::string &get_pkcs_session_password() const
    {
        return pkcs_session_password_;
    }

    void set_pkcs_session_password(const std::string &pwd)
    {
        pkcs_session_password_ = pwd;
    }

    const std::string &get_proteccio_conf_dir() const
    {
        return env_PROTECCIO_CONF_DIR_;
    }

    void set_proteccio_conf_dir(const std::string &d)
    {
        env_PROTECCIO_CONF_DIR_ = d;
    }

    const std::string &get_pkcs_shared_object_path() const
    {
        return pkcs_library_shared_object_path_;
    }

    void set_pkcs_shared_object_path(const std::string &d)
    {
        pkcs_library_shared_object_path_ = d;
    }

  private:
    /**
     * The CKA_ID attribute value that represent
     * the ID of the key on the PKCS token.
     */
    ByteVector pkcs_object_id_;

    // Password to log into the PKCS session.
    // This feel like a weird place to store that...
    std::string pkcs_session_password_;

    /*
     * PKCS Token slot id.
     */
    size_t slot_id_;

    // The following value should not lives in the KeyStorage object.
    // But until we can refactor the cryptographic API of LLA, this will
    // have to do.

    // cppkcs will ignore the value of env_PROTECCIO_CONF_DIR_ and
    // pkcs_library_shared_object_path_ if the underlying was already
    // loaded. env_PROTECCIO_CONF_DIR_ is also ignored if the environment
    // variable is already set.

    // The value that we will set to the PROTECCIO_CONF_DIR
    // environment variable. This is needed by the atosnethsm PKCS
    // library implementation.
    std::string env_PROTECCIO_CONF_DIR_;

    // Path to the PKCS shared object that the cppkcs library will use.
    std::string pkcs_library_shared_object_path_;
};
}

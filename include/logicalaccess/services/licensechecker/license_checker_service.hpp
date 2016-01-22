#pragma once

#include "logicalaccess/services/reader_service.hpp"
#include "logicalaccess/logicalaccess_api.hpp"

namespace logicalaccess
{
/**
 * The license checker service permits verification
 * of license keys that are stored on a physical RFID reader.
 *
 * The service implementation being dependant on the type of reader unit,
 * the service is instantiated through the ReaderUnit class.
 * The implementation may live in other DLL (lla-private) and the
 * LibraryManager will be needed to instantiated the correct implementation
 * for the reader.
 */
class LIBLOGICALACCESS_API LicenseCheckerService : public ReaderService
{
  public:
    LicenseCheckerService(ReaderUnitPtr ru)
        : ReaderService(ru){};

    virtual ~LicenseCheckerService() = default;

    /**
     * Write the license key to the reader.
     *
     * This operation may involve changing the current security mode in which
     * the reader operate, as well as overwrite previous key stored at the
     * location provided by `storage_info`.
     *
     * @param storage_info is the KeyStorage description where to write
     * the license key. Most underlying LicenseCheckerService will
     * expect the object to be of type ReaderMemoryKeyStorage.
     *
     * @param key is the key object that will be written in the reader's key
     * slot. The underlying depends on what type of key can be stored
     * on a given reader.
     * @param oldkey is the actually that is currently on the reader and that we need
     * to use in order to authenticate and change the. It is optional, and if not
     * specified
     * will default to representing the default key.
     */
    virtual void writeLicenseKey(const KeyStoragePtr &storage_info,
                                 const KeyPtr &key, KeyPtr oldKey = nullptr) = 0;

    /**
     * Attempt to validate the license key against the one stored on the reader.
     *
     * This operation may involve changing the reader's security mode, as well as
     * re-authenticate against the reader, canceling any previous authenticated
     * session that may exists.
     * This operation is non-destruction as it will not write anything on the reader.
     *
     * @param storage_info is the KeyStorage describing which key to authenticate
     * against.
     * @param key is the key that will be used to attempt the authentication.
     */
    virtual bool validateLicenseKey(const KeyStoragePtr &storage_info,
                                    const KeyPtr &key) = 0;

    virtual ReaderServiceType getServiceType() const override
    {
        return RST_LICENSE_CHECKER;
    }
};
}
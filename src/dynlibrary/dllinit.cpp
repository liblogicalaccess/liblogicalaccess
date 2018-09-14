#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/crypto/tomcrypt.h>
#include <logicalaccess/plugins/llacommon/settings.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

/**
 * We create a static struct of this type to run code on library load.
 *
 * Note that this file is compiled only under Linux. On windows, the dllmain() stuff
 * is used instead.
 */
struct dll_init_s
{
    dll_init_s()
    {
        int ret;
        ret = register_cipher(&des_desc);
        EXCEPTION_ASSERT_WITH_LOG(ret >= 0, logicalaccess::LibLogicalAccessException,
                                  "Failed to register DES cipher with TomCrypt");

        ret = register_cipher(&des3_desc);
        EXCEPTION_ASSERT_WITH_LOG(ret >= 0, logicalaccess::LibLogicalAccessException,
                                  "Failed to register 3DES cipher with TomCrypt");

        logicalaccess::Settings::getInstance()->Initialize();
        LOG(logicalaccess::LogLevel::INFOS) << "Process attached !";
    }

    ~dll_init_s()
    {
        LOG(logicalaccess::LogLevel::INFOS) << "Process detached !";
        logicalaccess::Settings::getInstance()->Uninitialize();

        unregister_cipher(&des3_desc);
        unregister_cipher(&des_desc);
    }
};

static dll_init_s initializer;

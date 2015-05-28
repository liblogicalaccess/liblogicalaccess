#include "logicalaccess/crypto/tomcrypt.h"

#include "logicalaccess/settings.hpp"
#include "logicalaccess/logs.hpp"

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
        register_cipher(&des_desc);
        register_cipher(&des3_desc);

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

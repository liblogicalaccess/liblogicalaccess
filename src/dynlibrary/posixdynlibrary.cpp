#include <dlfcn.h>
#include <logicalaccess/logs.hpp>

#include "logicalaccess/dynlibrary/posixdynlibrary.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
IDynLibrary *newDynLibrary(const std::string &dlName)
{
    return new PosixDynLibrary(dlName);
}

PosixDynLibrary::PosixDynLibrary(const std::string &dlName)
    : _name(dlName)
{
    ::dlerror(); // clear potential old error
    if ((_handle = ::dlopen(dlName.c_str(), RTLD_NOW)) == NULL)
    {
        const char *error = ::dlerror();
        if (error)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Failed to dlopen() " +
                                                                    dlName + +": " +
                                                                    std::string(error));
        }
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Failed to dlopen() " +
                                                                    dlName +
                                                                    " for unkown reason");
    }
}

void *PosixDynLibrary::getSymbol(const char *symName)
{
    void *sym;
    const char *err;

    ::dlerror();
    sym = ::dlsym(_handle, symName);
    err = ::dlerror();
    if (err)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, err);
    return sym;
}

bool PosixDynLibrary::hasSymbol(const char *name)
{
    void *sym;
    const char *err;

    ::dlerror();
    sym = ::dlsym(_handle, name);
    err = ::dlerror();
    if (err || !sym)
    {
        return false;
    }
    return true;
}
}

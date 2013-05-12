#include <dlfcn.h>

#include "logicalaccess/dynlibrary/posixdynlibrary.hpp"

namespace LOGICALACCESS
{
  IDynLibrary* newDynLibrary(const std::string& dlName)
  { return new PosixDynLibrary(dlName); }

  PosixDynLibrary::PosixDynLibrary(const std::string& dlName) : _name(dlName)
  {
    if ((_handle = ::dlopen(dlName.c_str(), RTLD_NOW)) == NULL)
      THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, ::dlerror());
  }

  void* PosixDynLibrary::getSymbol(const char* symName)
  {
    void* sym;
    char* err;

    ::dlerror();
    sym = ::dlsym(_handle, symName);
    err = ::dlerror();
    if (err)
      THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, ::dlerror());
    return sym;
  }
}

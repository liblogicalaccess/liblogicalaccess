#ifndef IDYNLIBRARY_HPP_
#define IDYNLIBRARY_HPP_

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/cards/keydiversification.hpp>
#include <string>

#if defined __APPLE__
#define CDECL_WIN32_
#define EXTENSION_LIB ".dylib"
#elif defined(__unix__) || defined(ANDROID)
#define CDECL_WIN32_
#define EXTENSION_LIB ".so"
#else
#define CDECL_WIN32_ _cdecl
#define EXTENSION_LIB ".dll"
#endif

namespace logicalaccess
{
typedef char *(*getlibname)();
typedef void(CDECL_WIN32_ *getprovider)(std::shared_ptr<ReaderProvider> *);
typedef void(CDECL_WIN32_ *getcard)(std::shared_ptr<Chip> *);
typedef void(CDECL_WIN32_ *getdiversification)(std::shared_ptr<KeyDiversification> *);
typedef void(CDECL_WIN32_ *getcommands)(std::shared_ptr<Commands> *);
typedef void(CDECL_WIN32_ *setcryptocontext)(std::shared_ptr<Commands> *,
                                             std::shared_ptr<Chip> *);
typedef bool(CDECL_WIN32_ *getobjectinfoat)(unsigned int, char *, size_t, void **);

class IDynLibrary
{
  public:
    virtual ~IDynLibrary()
    {
    }

    virtual const std::string &getName() const   = 0;
    virtual void *getSymbol(const char *symName) = 0;
    virtual bool hasSymbol(const char *name)     = 0;
};

IDynLibrary *newDynLibrary(const std::string &dlName);
}

#endif

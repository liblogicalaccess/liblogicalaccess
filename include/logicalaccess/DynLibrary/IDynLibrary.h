#ifndef IDYNLIBRARY_H_
#define IDYNLIBRARY_H_

#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "logicalaccess/Cards/Chip.h"
#include <string>

#ifdef UNIX
#define CDECL_WIN32_
#define EXTENSION_LIB ".so"
#else
#define CDECL_WIN32_ _cdecl
#define EXTENSION_LIB ".dll"
#endif

namespace logicalaccess
{
	typedef char* (*getlibname)();
	typedef void (CDECL_WIN32_ *getprovider)(boost::shared_ptr<ReaderProvider>*);
	typedef void (CDECL_WIN32_ *getcard)(boost::shared_ptr<Chip>*);
	typedef void (CDECL_WIN32_ *getcardprovider)(boost::shared_ptr<CardProvider>*);
	typedef void (CDECL_WIN32_ *getcommands)(boost::shared_ptr<Commands>*);
	typedef void (CDECL_WIN32_ *setcryptocontext)(boost::shared_ptr<Commands>*, boost::shared_ptr<Chip>*);
	typedef bool (CDECL_WIN32_ *getobjectinfoat)(unsigned int, char*, size_t, void**);

  class	IDynLibrary
  {
  public:
	virtual ~IDynLibrary() {}

	virtual const std::string& getName(void) const = 0;
	virtual void* getSymbol(const char* symName) = 0;
  };

  IDynLibrary* newDynLibrary(const std::string& dlName);
}

#endif

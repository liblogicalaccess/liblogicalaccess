#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "A3MLGM5600ReaderProvider.h"

#ifdef _MSC_VER
#include "logicalaccess/msliblogicalaccess.h"
#else
#ifndef LIBLOGICALACCESS_API
#define LIBLOGICALACCESS_API
#endif
#ifndef DISABLE_PRAGMA_WARNING
#define DISABLE_PRAGMA_WARNING /**< \brief winsmcrd.h was modified to support this macro, to avoid MSVC specific warnings pragma */
#endif
#endif

extern "C"
{
	LIBLOGICALACCESS_API char *getLibraryName()
	{
		return (char *)"A3MLGM5600";
	}

	LIBLOGICALACCESS_API void getA3MLGM5600Reader(boost::shared_ptr<LOGICALACCESS::ReaderProvider>* rp)
	{
		if (rp != NULL)
		{
			*rp = LOGICALACCESS::A3MLGM5600ReaderProvider::getSingletonInstance();
		}
	}
}

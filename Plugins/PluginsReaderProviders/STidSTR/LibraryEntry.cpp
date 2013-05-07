#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "STidSTRReaderProvider.h"

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
		return (char *)"STidSTR";
	}

	LIBLOGICALACCESS_API void getSTidSTRReader(boost::shared_ptr<LOGICALACCESS::STidSTRReaderProvider>* rp)
	{
		if (rp != NULL)
		{
			*rp = LOGICALACCESS::STidSTRReaderProvider::getSingletonInstance();
		}
	}

	LIBLOGICALACCESS_API bool getReaderInfoAt(unsigned int index, char* readername, size_t readernamelen, void** getterfct)
	{
		bool ret = false;
		if (readername != NULL && readernamelen == PLUGINOBJECT_MAXLEN && getterfct != NULL)
		{
			switch (index)
			{
			case 0:
				{
					*getterfct = &getSTidSTRReader;
					sprintf(readername, READER_STIDSTR);
					ret = true;
					break;
				}
			}
		}

		return ret;
	}
}

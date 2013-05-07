#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "AxessTMC13ReaderProvider.h"

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
		return (char *)"AxessTMC13";
	}

	LIBLOGICALACCESS_API void getAxessTMC13Reader(boost::shared_ptr<LOGICALACCESS::ReaderProvider>* rp)
	{
		if (rp != NULL)
		{
			*rp = LOGICALACCESS::AxessTMC13ReaderProvider::getSingletonInstance();
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
					*getterfct = &getAxessTMC13Reader;
					sprintf(readername, READER_AXESSTMC13);
					ret = true;
					break;
				}
			}
		}

		return ret;
	}
}

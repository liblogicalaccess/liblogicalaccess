#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "ok5553readerprovider.hpp"

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
		return (char *)"OK5553";
	}

	LIBLOGICALACCESS_API void getOK5553Reader(boost::shared_ptr<logicalaccess::ReaderProvider>* rp)
	{
		if (rp != NULL)
		{
			*rp = logicalaccess::OK5553ReaderProvider::getSingletonInstance();
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
					*getterfct = (void*)&getOK5553Reader;
					sprintf(readername, READER_OK5553);
					ret = true;
				}
				break;
			}
		}

		return ret;
	}
}

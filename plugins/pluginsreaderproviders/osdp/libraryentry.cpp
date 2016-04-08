#include <string>
#include "osdpreaderprovider.hpp"

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
		return (char *)"OSDP";
	}

	LIBLOGICALACCESS_API void getOSDPReader(std::shared_ptr<logicalaccess::OSDPReaderProvider>* rp)
	{
		if (rp != NULL)
		{
			*rp = logicalaccess::OSDPReaderProvider::getSingletonInstance();
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
					*getterfct = (void*)&getOSDPReader;
					sprintf(readername, READER_OSDP);
					ret = true;
				}
				break;
			}
		}

		return ret;
	}
}

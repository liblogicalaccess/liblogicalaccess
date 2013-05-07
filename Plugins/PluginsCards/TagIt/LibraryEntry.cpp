#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "TagItChip.h"

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
		return (char *)"TagIt";
	}
	
	LIBLOGICALACCESS_API void getTagItChip(boost::shared_ptr<LOGICALACCESS::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<LOGICALACCESS::TagItChip>(new LOGICALACCESS::TagItChip());
		}
	}

	LIBLOGICALACCESS_API void getTagItCardProvider(boost::shared_ptr<LOGICALACCESS::CardProvider>* cp)
	{
		if (cp != NULL)
		{
			*cp = boost::shared_ptr<LOGICALACCESS::TagItCardProvider>(new LOGICALACCESS::TagItCardProvider());
		}
	}

	LIBLOGICALACCESS_API bool getChipInfoAt(unsigned int index, char* chipname, size_t chipnamelen, void** getterfct)
	{
		bool ret = false;
		if (chipname != NULL && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != NULL)
		{
			switch (index)
			{
			case 0:
				{
					*getterfct = &getTagItChip;
					sprintf(chipname, CHIP_TAGIT);
					ret = true;
					break;
				}
			}
		}

		return ret;
	}
}

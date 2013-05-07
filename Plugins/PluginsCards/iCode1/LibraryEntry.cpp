#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "ICode1Chip.h"

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
		return (char*)"iCode1";
	}
	
	LIBLOGICALACCESS_API void getiCode1Chip(boost::shared_ptr<LOGICALACCESS::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<LOGICALACCESS::ICode1Chip>(new LOGICALACCESS::ICode1Chip());
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
					*getterfct = &getiCode1Chip;
					sprintf(chipname, CHIP_ICODE1);
					ret = true;
					break;
				}
			}
		}

		return ret;
	}
}

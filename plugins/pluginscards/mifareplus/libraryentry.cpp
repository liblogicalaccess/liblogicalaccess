#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "mifareplus4kchip.hpp"

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
		return (char *)"MifarePlus";
	}	
	
	LIBLOGICALACCESS_API void getMifarePlus4KChip(boost::shared_ptr<logicalaccess::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<logicalaccess::MifarePlus4KChip>(new logicalaccess::MifarePlus4KChip());
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
					*getterfct = (void*)&getMifarePlus4KChip;
					sprintf(chipname, CHIP_MIFAREPLUS4K);
					ret = true;
				}
				break;
			}
		}

		return ret;
	}
}

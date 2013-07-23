#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "desfireev1chip.hpp"
#include "desfirechip.hpp"

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
		return (char *)"DESFire";
	}

	LIBLOGICALACCESS_API void getDESFireEV1Chip(boost::shared_ptr<logicalaccess::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<logicalaccess::DESFireEV1Chip>(new logicalaccess::DESFireEV1Chip());
		}
	}
	
	LIBLOGICALACCESS_API void getDESFireChip(boost::shared_ptr<logicalaccess::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<logicalaccess::DESFireChip>(new logicalaccess::DESFireChip());
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
					*getterfct = (void*)&getDESFireChip;
					sprintf(chipname, CHIP_DESFIRE);
					ret = true;
				}
				break;

			case 1:
				{
					*getterfct = (void*)&getDESFireEV1Chip;
					sprintf(chipname, CHIP_DESFIRE_EV1);
					ret = true;
				}
				break;
			}
		}

		return ret;
	}
}

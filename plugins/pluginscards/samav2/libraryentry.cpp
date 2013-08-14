#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "samav1chip.hpp"
#include "samav2chip.hpp"


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
		return (char *)"SAM";
	}
	
	LIBLOGICALACCESS_API void getSAM_AV1Chip(boost::shared_ptr<logicalaccess::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<logicalaccess::SAMAV1Chip>(new logicalaccess::SAMAV1Chip());
		}
	}

	LIBLOGICALACCESS_API void getSAM_AV2Chip(boost::shared_ptr<logicalaccess::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<logicalaccess::SAMAV2Chip>(new logicalaccess::SAMAV2Chip());
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
					*getterfct = (void*)&getSAM_AV1Chip;
					sprintf(chipname, CHIP_SAMAV1);
					ret = true;
				}
				break;
			case 1:
				{
					*getterfct = (void*)&getSAM_AV2Chip;
					sprintf(chipname, CHIP_SAMAV2);
					ret = true;
				}
				break;
			}
		}

		return ret;
	}
}

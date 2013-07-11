#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "generictagchip.hpp"

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
		return (char*)"GenericTag";
	}
	
	LIBLOGICALACCESS_API void getGenericTagChip(boost::shared_ptr<logicalaccess::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<logicalaccess::GenericTagChip>(new logicalaccess::GenericTagChip());
		}
	}

	LIBLOGICALACCESS_API void setTagIdBitsLengthOfGenericTagChip(boost::shared_ptr<logicalaccess::Chip>* chip, unsigned int bits)
	{
		if (chip != NULL)
		{
			boost::dynamic_pointer_cast<logicalaccess::GenericTagChip>(*chip)->setTagIdBitsLength(bits);
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
					*getterfct = (void*)&getGenericTagChip;
					sprintf(chipname, CHIP_GENERICTAG);
					ret = true;
				}
				break;
			}
		}

		return ret;
	}
}
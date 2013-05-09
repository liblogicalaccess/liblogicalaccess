#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "Mifare1KChip.h"
#include "Mifare4KChip.h"
#include "MifareChip.h"

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
		return (char *)"Mifare";
	}
	
	LIBLOGICALACCESS_API void getMifare1KChip(boost::shared_ptr<LOGICALACCESS::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<LOGICALACCESS::Mifare1KChip>(new LOGICALACCESS::Mifare1KChip());
		}
	}

	LIBLOGICALACCESS_API void getMifare4KChip(boost::shared_ptr<LOGICALACCESS::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<LOGICALACCESS::Mifare4KChip>(new LOGICALACCESS::Mifare4KChip());
		}
	}

	LIBLOGICALACCESS_API void getMifareChip(boost::shared_ptr<LOGICALACCESS::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<LOGICALACCESS::MifareChip>(new LOGICALACCESS::MifareChip());
		}
	}

	LIBLOGICALACCESS_API void getMifareCardProvider(boost::shared_ptr<LOGICALACCESS::CardProvider>* cp)
	{
		if (cp != NULL)
		{
			*cp = boost::shared_ptr<LOGICALACCESS::MifareCardProvider>(new LOGICALACCESS::MifareCardProvider());
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
					*getterfct = (void*)&getMifare1KChip;
					sprintf(chipname, CHIP_MIFARE1K);
					ret = true;
				}
				break;
			case 1:
				{
					*getterfct = (void*)&getMifare1KChip;
					sprintf(chipname, CHIP_MIFARE4K);
					ret = true;
				}
				break;
			}
		}

		return ret;
	}
}

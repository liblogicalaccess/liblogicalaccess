#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "DESFireEV1Chip.h"
#include "DESFireChip.h"

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

	LIBLOGICALACCESS_API void getDESFireEV1Chip(boost::shared_ptr<LOGICALACCESS::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<LOGICALACCESS::DESFireEV1Chip>(new LOGICALACCESS::DESFireEV1Chip());
		}
	}
	
	LIBLOGICALACCESS_API void getDESFireChip(boost::shared_ptr<LOGICALACCESS::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<LOGICALACCESS::DESFireChip>(new LOGICALACCESS::DESFireChip());
		}
	}

	LIBLOGICALACCESS_API void getDESFireEV1CardProvider(boost::shared_ptr<LOGICALACCESS::CardProvider>* cp)
	{
		if (cp != NULL)
		{
			*cp = boost::shared_ptr<LOGICALACCESS::DESFireEV1CardProvider>(new LOGICALACCESS::DESFireEV1CardProvider());
		}
	}

	LIBLOGICALACCESS_API void getDESFireCardProvider(boost::shared_ptr<LOGICALACCESS::CardProvider>* cp)
	{
		if (cp != NULL)
		{
			*cp = boost::shared_ptr<LOGICALACCESS::DESFireCardProvider>(new LOGICALACCESS::DESFireCardProvider());
		}
	}
}

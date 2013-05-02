#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "TwicChip.h"

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
		return (char *)"Twic";
	}
	
	LIBLOGICALACCESS_API void getTwicChip(boost::shared_ptr<LOGICALACCESS::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<LOGICALACCESS::TwicChip>(new LOGICALACCESS::TwicChip());
		}
	}

	LIBLOGICALACCESS_API void getTwicCardProvider(boost::shared_ptr<LOGICALACCESS::CardProvider>* cp)
	{
		if (cp != NULL)
		{
			*cp = boost::shared_ptr<LOGICALACCESS::TwicCardProvider>(new LOGICALACCESS::TwicCardProvider());
		}
	}
}

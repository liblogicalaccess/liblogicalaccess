#include <string>
#include <boost/shared_ptr.hpp>
#include "TwicCommands.h"
#include "Commands/TwicISO7816Commands.h"
#include "Commands/DESFireISO7816Commands.h"
#include "Commands/DESFireEV1ISO7816Commands.h"
#include "logicalaccess/Cards/Chip.h"

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
		return (char *)"ISO7816";
	}

	LIBLOGICALACCESS_API void getTwicISO7816Commands(boost::shared_ptr<LOGICALACCESS::Commands>* commands)
	{
		if (commands != NULL)
		{
			*commands = boost::shared_ptr<LOGICALACCESS::TwicISO7816Commands>(new LOGICALACCESS::TwicISO7816Commands());
		}
	}

	LIBLOGICALACCESS_API void getDESFireISO7816Commands(boost::shared_ptr<LOGICALACCESS::Commands>* commands)
	{
		if (commands != NULL)
		{
			*commands = boost::shared_ptr<LOGICALACCESS::DESFireISO7816Commands>(new LOGICALACCESS::DESFireISO7816Commands());
		}
	}

	LIBLOGICALACCESS_API void setCryptoContextDESFireISO7816Commands(boost::shared_ptr<LOGICALACCESS::Commands>* commands, boost::shared_ptr<LOGICALACCESS::Chip>* chip)
	{
		if (commands != NULL && chip != NULL)
		{
			boost::dynamic_pointer_cast<LOGICALACCESS::DESFireISO7816Commands>(*commands)->getCrypto().setCryptoContext(boost::dynamic_pointer_cast<LOGICALACCESS::DESFireProfile>((*chip)->getProfile()), (*chip)->getChipIdentifier());
		}
	}

	LIBLOGICALACCESS_API void getDESFireEV1ISO7816Commands(boost::shared_ptr<LOGICALACCESS::Commands>* commands)
	{
		if (commands != NULL)
		{
			*commands = boost::shared_ptr<LOGICALACCESS::DESFireEV1ISO7816Commands>(new LOGICALACCESS::DESFireEV1ISO7816Commands());
		}
	}

	LIBLOGICALACCESS_API void setCryptoContextDESFireEV1ISO7816Commands(boost::shared_ptr<LOGICALACCESS::Commands>* commands, boost::shared_ptr<LOGICALACCESS::Chip>* chip)
	{
		if (commands != NULL && chip != NULL)
		{
			boost::dynamic_pointer_cast<LOGICALACCESS::DESFireEV1ISO7816Commands>(*commands)->getCrypto().setCryptoContext(boost::dynamic_pointer_cast<LOGICALACCESS::DESFireProfile>((*chip)->getProfile()), (*chip)->getChipIdentifier());
		}
	}
}

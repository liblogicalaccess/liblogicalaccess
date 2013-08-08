#include <string>
#include <boost/shared_ptr.hpp>
#include "twiccommands.hpp"
#include "commands/twiciso7816commands.hpp"
#include "commands/desfireiso7816commands.hpp"
#include "commands/desfireev1iso7816commands.hpp"
#include "logicalaccess/cards/chip.hpp"

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

	LIBLOGICALACCESS_API void getTwicISO7816Commands(boost::shared_ptr<logicalaccess::Commands>* commands)
	{
		if (commands != NULL)
		{
			*commands = boost::shared_ptr<logicalaccess::TwicISO7816Commands>(new logicalaccess::TwicISO7816Commands());
		}
	}

	LIBLOGICALACCESS_API void getDESFireISO7816Commands(boost::shared_ptr<logicalaccess::Commands>* commands)
	{
		if (commands != NULL)
		{
			*commands = boost::shared_ptr<logicalaccess::DESFireISO7816Commands>(new logicalaccess::DESFireISO7816Commands());
		}
	}

	LIBLOGICALACCESS_API void setCryptoContextDESFireISO7816Commands(boost::shared_ptr<logicalaccess::Commands>* commands, boost::shared_ptr<logicalaccess::Chip>* chip)
	{
		if (commands != NULL && chip != NULL)
		{
			boost::dynamic_pointer_cast<logicalaccess::DESFireISO7816Commands>(*commands)->getCrypto()->setCryptoContext(boost::dynamic_pointer_cast<logicalaccess::DESFireProfile>((*chip)->getProfile()), (*chip)->getChipIdentifier());
		}
	}

	LIBLOGICALACCESS_API void getDESFireEV1ISO7816Commands(boost::shared_ptr<logicalaccess::Commands>* commands)
	{
		if (commands != NULL)
		{
			*commands = boost::shared_ptr<logicalaccess::DESFireEV1ISO7816Commands>(new logicalaccess::DESFireEV1ISO7816Commands());
		}
	}

	LIBLOGICALACCESS_API void setCryptoContextDESFireEV1ISO7816Commands(boost::shared_ptr<logicalaccess::Commands>* commands, boost::shared_ptr<logicalaccess::Chip>* chip)
	{
		if (commands != NULL && chip != NULL)
		{
			boost::dynamic_pointer_cast<logicalaccess::DESFireEV1ISO7816Commands>(*commands)->getCrypto()->setCryptoContext(boost::dynamic_pointer_cast<logicalaccess::DESFireProfile>((*chip)->getProfile()), (*chip)->getChipIdentifier());
		}
	}
}

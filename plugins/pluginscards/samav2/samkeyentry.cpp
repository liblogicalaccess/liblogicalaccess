/**
 * \file samkeyentry.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMKeyEntry source.
 */

#include "samkeyentry.hpp"

namespace logicalaccess
{
	//This declaration made SAMKeyEntry available for external link
	template SAMKeyEntry<logicalaccess::KeyEntryAV2Information, logicalaccess::SETAV2>;
	template SAMKeyEntry<logicalaccess::KeyEntryAV1Information, logicalaccess::SETAV1>;
}

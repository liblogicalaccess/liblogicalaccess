/**
 * \file tagitcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Tag-It commands.
 */

#include "tagitcommands.hpp"


namespace logicalaccess
{
	bool TagItCommands::isLocked(size_t block)
	{
		unsigned char status = getSecurityStatus(block);
		
		return ((status & 0x01) != 0);
	}
}
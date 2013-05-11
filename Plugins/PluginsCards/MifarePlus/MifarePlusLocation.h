/**
 * \file MifarePlusLocation.h
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlusLocation.
 */

#ifndef LOGICALACCESS_MIFAREPLUSLOCATION_H
#define LOGICALACCESS_MIFAREPLUSLOCATION_H

#include "../Mifare/MifareLocation.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
	/**
	 * \brief A  MifarePlus location informations.
	 */
	class LIBLOGICALACCESS_API MifarePlusLocation : public MifareLocation
	{
		public:
			/**
			 * \brief Constructor.
			 */
			MifarePlusLocation();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifarePlusLocation();
	};	
}

#endif /* LOGICALACCESS_MIFAREPLUSLOCATION_H */

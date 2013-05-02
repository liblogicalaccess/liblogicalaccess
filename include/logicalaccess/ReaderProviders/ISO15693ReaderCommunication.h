/**
 * \file ISO15693ReaderCommunication.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief ISO15693 Reader communication. 
 */

#ifndef LOGICALACCESS_ISO15693BREADERCOMMUNICATION_H
#define LOGICALACCESS_ISO15693BREADERCOMMUNICATION_H

#include "logicalaccess/ReaderProviders/ReaderCommunication.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

namespace LOGICALACCESS
{
	/**
	 * \brief A ISO15693 reader communication base class.
	 */
	class LIBLOGICALACCESS_API ISO15693ReaderCommunication : public ReaderCommunication
	{
		public:
			

	};

}

#endif /* LOGICALACCESS_ISO15693BREADERCOMMUNICATION_H */


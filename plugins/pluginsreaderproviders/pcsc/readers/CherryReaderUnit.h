/**
 * \file CherryReaderUnit.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Cherry Reader unit.
 */

#ifndef LOGICALACCESS_CHERRYREADERUNIT_H
#define LOGICALACCESS_CHERRYREADERUNIT_H

#include "../PCSCReaderUnit.h"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	/**
	 * \brief The Cherry reader unit class.
	 */
	class LIBLOGICALACCESS_API CherryReaderUnit : public PCSCReaderUnit
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			CherryReaderUnit(const std::string& name);

			/**
			 * \brief Destructor.
			 */
			virtual ~CherryReaderUnit();			

			/**
			 * \brief Get the PC/SC reader unit type.
			 * \return The PC/SC reader unit type.
			 */
			virtual PCSCReaderUnitType getPCSCType() const;


		protected:
			
	};
}

#endif

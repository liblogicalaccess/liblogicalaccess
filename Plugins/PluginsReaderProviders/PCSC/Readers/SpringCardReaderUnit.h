/**
 * \file SpringCardReaderUnit.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief SpringCard Reader unit.
 */

#ifndef LOGICALACCESS_SPRINGCARDREADERUNIT_H
#define LOGICALACCESS_SPRINGCARDREADERUNIT_H

#include "../PCSCReaderUnit.h"

#include <string>
#include <vector>
#include <iostream>

namespace LOGICALACCESS
{
	/**
	 * \brief The SpringCard reader unit class.
	 */
	class LIBLOGICALACCESS_API SpringCardReaderUnit : public PCSCReaderUnit
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			SpringCardReaderUnit(const std::string& name);

			/**
			 * \brief Destructor.
			 */
			virtual ~SpringCardReaderUnit();			

			/**
			 * \brief Get the PC/SC reader unit type.
			 * \return The PC/SC reader unit type.
			 */
			virtual PCSCReaderUnitType getPCSCType() const;


		protected:
			
	};
}

#endif

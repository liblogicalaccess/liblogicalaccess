/**
 * \file springcardreaderunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SpringCard Reader unit.
 */

#ifndef LOGICALACCESS_SPRINGCARDREADERUNIT_HPP
#define LOGICALACCESS_SPRINGCARDREADERUNIT_HPP

#include "../pcscreaderunit.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
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

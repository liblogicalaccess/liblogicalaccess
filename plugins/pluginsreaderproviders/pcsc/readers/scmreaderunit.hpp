/**
 * \file scmreaderunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SCM Reader unit.
 */

#ifndef LOGICALACCESS_SCMREADERUNIT_HPP
#define LOGICALACCESS_SCMREADERUNIT_HPP

#include "../pcscreaderunit.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	/**
	 * \brief The SCM reader unit class.
	 */
	class LIBLOGICALACCESS_API SCMReaderUnit : public PCSCReaderUnit
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			SCMReaderUnit(const std::string& name);

			/**
			 * \brief Destructor.
			 */
			virtual ~SCMReaderUnit();			

			/**
			 * \brief Get the PC/SC reader unit type.
			 * \return The PC/SC reader unit type.
			 */
			virtual PCSCReaderUnitType getPCSCType() const;


		protected:
			
	};
}

#endif

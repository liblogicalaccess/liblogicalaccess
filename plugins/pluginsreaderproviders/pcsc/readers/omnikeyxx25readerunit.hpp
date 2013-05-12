/**
 * \file omnikeyxx25readerunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX25 Reader unit.
 */

#ifndef LOGICALACCESS_OMNIKEYXX25READERUNIT_HPP
#define LOGICALACCESS_OMNIKEYXX25READERUNIT_HPP

#include "omnikeyreaderunit.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	/**
	 * \brief The Omnikey XX25 reader unit class.
	 */
	class LIBLOGICALACCESS_API OmnikeyXX25ReaderUnit : public OmnikeyReaderUnit
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			OmnikeyXX25ReaderUnit(const std::string& name);

			/**
			 * \brief Destructor.
			 */
			virtual ~OmnikeyXX25ReaderUnit();			

			/**
			 * \brief Get the PC/SC reader unit type.
			 * \return The PC/SC reader unit type.
			 */
			virtual PCSCReaderUnitType getPCSCType() const;

			/**
			 * \brief Get the card serial number.
			 * \return The card serial number.
			 *
			 * If the card handle is not connected to the card, the call fails.
			 */
			virtual std::vector<unsigned char> getCardSerialNumber();

		protected:
			
	};
}

#endif

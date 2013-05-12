/**
 * \file omnikeyxx27readerunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX27 Reader unit.
 */

#ifndef LOGICALACCESS_OMNIKEYXX27READERUNIT_HPP
#define LOGICALACCESS_OMNIKEYXX27READERUNIT_HPP

#include "omnikeyreaderunit.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	/**
	 * \brief The Omnikey XX27 reader unit class.
	 */
	class LIBLOGICALACCESS_API OmnikeyXX27ReaderUnit : public OmnikeyReaderUnit
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			OmnikeyXX27ReaderUnit(const std::string& name);

			/**
			 * \brief Destructor.
			 */
			virtual ~OmnikeyXX27ReaderUnit();			

			/**
			 * \brief Get the PC/SC reader unit type.
			 * \return The PC/SC reader unit type.
			 */
			virtual PCSCReaderUnitType getPCSCType() const;			

			/**
			 * \brief Get the PC/SC reader/card adapter for a card type.
			 * \param type The card type.
			 * \return The PC/SC reader/card adapter.
			 */
			virtual boost::shared_ptr<ReaderCardAdapter> getReaderCardAdapter(std::string type);


		protected:
			
	};
}

#endif

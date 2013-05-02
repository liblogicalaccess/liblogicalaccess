/**
 * \file OmnikeyReaderUnit.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Omnikey Reader unit.
 */

#ifndef LOGICALACCESS_OMNIKEYREADERUNIT_H
#define LOGICALACCESS_OMNIKEYREADERUNIT_H

#include "../PCSCReaderUnit.h"

#include <string>
#include <vector>
#include <iostream>
#include <map>

namespace LOGICALACCESS
{
	/**
	 * \brief The Omnikey reader unit base class.
	 */
	class LIBLOGICALACCESS_API OmnikeyReaderUnit : public PCSCReaderUnit
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			OmnikeyReaderUnit(const std::string& name);

			/**
			 * \brief Destructor.
			 */
			virtual ~OmnikeyReaderUnit();			

			/**
			 * \brief Get a string hexadecimal representation of the reader serial number
			 * \return The reader serial number or an empty string on error.
			 */
			virtual std::string getReaderSerialNumber();

			/**
			 * \brief Get if the current connection is in secure mode.
			 * \return True if secure mode is currently use, false otherwise.
			 */
			bool getIsSecureConnectionMode();

			/**
			 * \brief Set if the current connection is in secure mode.
			 * \param isSecure The secure mode value.
			 */
			void setIsSecureConnectionMode(bool isSecure);

			/**
			 * \brief Disconnect from the card.
			 */
			virtual void disconnect();

			/**
			 * \brief Is secure connection mode ?
			 * \remarks We must store it in static memory because the connection mode is global for all connection to the reader
			 */
			static std::map<std::string, bool> d_isSecureConnectionMode;

		protected:
						
	};
}

#endif

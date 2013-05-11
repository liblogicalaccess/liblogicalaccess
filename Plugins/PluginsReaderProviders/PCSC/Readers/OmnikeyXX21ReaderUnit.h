/**
 * \file OmnikeyXX21ReaderUnit.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX21 Reader unit.
 */

#ifndef LOGICALACCESS_OMNIKEYXX21READERUNIT_H
#define LOGICALACCESS_OMNIKEYXX21READERUNIT_H

#include "OmnikeyReaderUnit.h"
#include "OmnikeyXX21ReaderUnitConfiguration.h"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	/**
	 * \brief The Omnikey XX21 reader unit class.
	 */
	class LIBLOGICALACCESS_API OmnikeyXX21ReaderUnit : public OmnikeyReaderUnit
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			OmnikeyXX21ReaderUnit(const std::string& name);

			/**
			 * \brief Destructor.
			 */
			virtual ~OmnikeyXX21ReaderUnit();			

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

			/**
			 * \brief Get the Omnikey XX21 reader unit configuration.
			 * \return The Omnikey XX21 reader unit configuration.
			 */
			boost::shared_ptr<OmnikeyXX21ReaderUnitConfiguration> getOmnikeyXX21Configuration() { return boost::dynamic_pointer_cast<OmnikeyXX21ReaderUnitConfiguration>(getPCSCConfiguration()); };

			/**
			 * \brief Change a key in reader memory.
			 * \param keystorage The key storage information.
			 * \param key The key value.
			 */
			virtual void changeReaderKey(boost::shared_ptr<ReaderMemoryKeyStorage> keystorage, const std::vector<unsigned char>& key);

			/**
			 * \brief Get if the inserted card is from ISO 14443-4 A or B.
			 * \param isTypeA Will be set to true if the card is from ISO 14443-4 A, false if not or unknown.
			 * \param isTypeB Will be set to true if the card is from ISO 14443-4 B, false if not or unknown.
			 */
			virtual void getT_CL_ISOType(bool& isTypeA, bool& isTypeB);


		protected:
			
	};
}

#endif

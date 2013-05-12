/**
 * \file twiciso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic ISO7816 commands.
 */

#ifndef LOGICALACCESS_TWICISO7816COMMANDS_HPP
#define LOGICALACCESS_TWICISO7816COMMANDS_HPP

#include "logicalaccess/key.hpp"
#include "twiccommands.hpp"
#include "iso7816iso7816commands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	class TwicProfile;

	/**
	 * \brief The Twic ISO7816 commands base class.
	 */
	class LIBLOGICALACCESS_API TwicISO7816Commands : public ISO7816ISO7816Commands, public TwicCommands
	{
		public:

			/**
			 * \brief Constructor.
			 */
			TwicISO7816Commands();

			/**
			 * \brief Destructor.
			 */
			virtual ~TwicISO7816Commands();

			/**
			 * \brief Select the TWIC application.
			 */
			virtual void selectTWICApplication();			

			/**
			 * \brief Get the Unsigned Cardholder Unique Identifier.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 */
			virtual bool getUnsignedCardholderUniqueIdentifier(void* data, size_t& dataLength);

			/**
			 * \brief Get the TWIC Privacy Key.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 * \remarks Only accessible with the contact chip.
			 */
			virtual bool getTWICPrivacyKey(void* data, size_t& dataLength);

			/**
			 * \brief Get the Cardholder Unique Identifier.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 */
			virtual bool getCardholderUniqueIdentifier(void* data, size_t& dataLength);

			/**
			 * \brief Get the Cardholder Fingerprints.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 */
			virtual bool getCardHolderFingerprints(void* data, size_t& dataLength);

			/**
			 * \brief Get the Security Object.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 */
			virtual bool getSecurityObject(void* data, size_t& dataLength);

			/**
			 * \brief Get TWIC data object.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \param dataObject The data object.
			 * \return True on success, false otherwise.
			 */
			virtual bool getTWICData(void* data, size_t& dataLength, int64_t dataObject);

		protected:
	
	};
}

#endif

/**
 * \file TwicCommands.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic commands.
 */

#ifndef LOGICALACCESS_TWICCOMMANDS_H
#define LOGICALACCESS_TWICCOMMANDS_H

#include "logicalaccess/Cards/AccessInfo.h"
#include "logicalaccess/Cards/Commands.h"
#include "logicalaccess/Cards/Location.h"


namespace logicalaccess
{
	/**
	 * \brief The Twic commands class.
	 */
	class LIBLOGICALACCESS_API TwicCommands : public virtual Commands
	{
		public:

			/**
			 * \brief Select the TWIC application.
			 */
			virtual void selectTWICApplication() = 0;

			/**
			 * \brief Get the Unsigned Cardholder Unique Identifier.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 */
			virtual bool getUnsignedCardholderUniqueIdentifier(void* data, size_t& dataLength) = 0;

			/**
			 * \brief Get the TWIC Privacy Key.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 * \remarks Only accessible with the contact chip.
			 */
			virtual bool getTWICPrivacyKey(void* data, size_t& dataLength) = 0;

			/**
			 * \brief Get the Cardholder Unique Identifier.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 */
			virtual bool getCardholderUniqueIdentifier(void* data, size_t& dataLength) = 0;

			/**
			 * \brief Get the Cardholder Fingerprints.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 */
			virtual bool getCardHolderFingerprints(void* data, size_t& dataLength) = 0;

			/**
			 * \brief Get the Security Object.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 */
			virtual bool getSecurityObject(void* data, size_t& dataLength) = 0;

			/**
			 * \brief Get TWIC data object.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \param dataObject The data object.
			 * \return True on success, false otherwise.
			 */
			virtual bool getTWICData(void* data, size_t& dataLength, int64_t dataObject) = 0;


	protected:

	};
}

#endif

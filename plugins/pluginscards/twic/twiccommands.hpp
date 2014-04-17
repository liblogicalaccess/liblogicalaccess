/**
 * \file twiccommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic commands.
 */

#ifndef LOGICALACCESS_TWICCOMMANDS_HPP
#define LOGICALACCESS_TWICCOMMANDS_HPP

#include "logicalaccess/cards/accessinfo.hpp"
#include "logicalaccess/cards/commands.hpp"
#include "twiclocation.hpp"


namespace logicalaccess
{
	/**
	 * \brief The Twic commands class.
	 */
	class LIBLOGICALACCESS_API TwicCommands : public virtual Commands
	{
		public:

			size_t getMinimumBytesRepresentation(size_t value);

			size_t getMaximumDataObjectLength(int64_t dataObject);

			size_t getMaximumTagLength(int64_t dataObject, unsigned char tag);

			size_t getValueFromBytes(unsigned char* data, size_t datalength);

			/**
			 * \brief Get the current length for a data object.
			 * \param dataObject The data object.
			 * \param withObjectLength Add header that describe the object length to the total number, or not.
			 * \return The current data object length.
			 */
			size_t getDataObjectLength(int64_t dataObject, bool withObjectLength = false);

			bool getTagData(boost::shared_ptr<TwicLocation> location, void* data, size_t datalen, void* datatag, size_t& datataglen);

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
			virtual std::vector<unsigned char>  getUnsignedCardholderUniqueIdentifier() = 0;

			/**
			 * \brief Get the TWIC Privacy Key.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 * \remarks Only accessible with the contact chip.
			 */
			virtual std::vector<unsigned char>  getTWICPrivacyKey() = 0;

			/**
			 * \brief Get the Cardholder Unique Identifier.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 */
			virtual std::vector<unsigned char>  getCardholderUniqueIdentifier() = 0;

			/**
			 * \brief Get the Cardholder Fingerprints.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 */
			virtual std::vector<unsigned char>  getCardHolderFingerprints() = 0;

			/**
			 * \brief Get the Security Object.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \return True on success, false otherwise.
			 */
			virtual std::vector<unsigned char>  getSecurityObject() = 0;

			/**
			 * \brief Get TWIC data object.
			 * \param data The data buffer.
			 * \param dataLength The buffer length.
			 * \param dataObject The data object.
			 * \return True on success, false otherwise.
			 */
			virtual std::vector<unsigned char>  getTWICData(int64_t dataObject) = 0;


	protected:

	};
}

#endif

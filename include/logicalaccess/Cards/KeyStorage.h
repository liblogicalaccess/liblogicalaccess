/**
 * \file KeyStorage.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Key storage description.
 */

#ifndef LOGICALACCESS_KEYSTORAGE_H
#define LOGICALACCESS_KEYSTORAGE_H

#include "logicalaccess/ReaderProviders/ReaderProvider.h"

namespace LOGICALACCESS
{
	/**
	 * \brief The key storage types.
	 */
	typedef enum {
		KST_COMPUTER_MEMORY = 0x00, /* Key stored in computer memory */
		KST_READER_MEMORY = 0x01, /* Key stored in reader memory */
		KST_SAM = 0x02 /* Key stored in SAM */
	} KeyStorageType;

	/**
	 * \brief A Key storage base class. The key storage specify where the key is stored in memory. It can have cryptographic functionalities.
	 */
	class LIBLOGICALACCESS_API KeyStorage : public XmlSerializable, public boost::enable_shared_from_this<KeyStorage>
	{
		public:

			/**
			 * \brief Get the key storage type.
			 * \return The key storage type.
			 */
			virtual KeyStorageType getType() const = 0;

			/**
			 * \brief Get the key storage instance from a key storage type.
			 * \return The key storage instance.
			 */
			static boost::shared_ptr<KeyStorage> getKeyStorageFromType(KeyStorageType kst);

	protected:
			
			
	};
}

#endif /* LOGICALACCESS_KEYSTORAGE_H */


/**
 * \file mifarestidstrcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare STidSTR card.
 */

#ifndef LOGICALACCESS_MIFARESTIDSTRCOMMANDS_HPP
#define LOGICALACCESS_MIFARESTIDSTRCOMMANDS_HPP

#include "mifarecommands.hpp"
#include "../readercardadapters/stidstrreadercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	/**
	 * \brief The Mifare commands class for STidSTR reader.
	 */
	class LIBLOGICALACCESS_API MifareSTidSTRCommands : public MifareCommands
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			MifareSTidSTRCommands();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareSTidSTRCommands();			

			/**
			 * \brief Get the STidSTR reader/card adapter.
			 * \return The STidSTR reader/card adapter.
			 */
			boost::shared_ptr<STidSTRReaderCardAdapter> getSTidSTRReaderCardAdapter() const { return boost::dynamic_pointer_cast<STidSTRReaderCardAdapter>(getReaderCardAdapter()); };			

			/**
			 * \brief Scan the RFID field for Mifare tag.
			 * \return The tag uid if present.
			 */
			std::vector<unsigned char> scanMifare();

			/**
			 * \brief Release the RFID field.
			 */
			void releaseRFIDField();

			/**
			 * \brief Load a key on a given location.
			 * \param location The location.
			 * \param key The key.
			 * \param keytype The mifare key type.
			 */
			virtual void loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifareKeyType keytype);			
			
			/**
			 * \brief Load a key to the reader.
			 * \param keyno The key number.
			 * \param keytype The key type.
			 * \param key The key byte array.
			 * \param keylen The key byte array length.
			 * \param vol Use volatile memory.
			 * \return true on success, false otherwise.
			 */
			virtual bool loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol = true);			

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param blockno The block number.
			 * \param keyno The key number, previously loaded with Mifare::loadKey().
			 * \param keytype The key type.
			 */
			virtual void authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype);

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param blockno The block number.
			 * \param key_storage The key storage used for authentication.
			 * \param keytype The key type.
			 */
			virtual void authenticate(unsigned char blockno, boost::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype);

			/**
			 * \brief Read bytes from the card.
			 * \param blockno The block number.
			 * \param len The count of bytes to read. (0 <= len < 16)
			 * \param buf The buffer in which to place the data.
			 * \param buflen The length of buffer.
			 * \return The count of bytes red.
			 */
			virtual size_t readBinary(unsigned char blockno, size_t len, void* buf, size_t buflen);

			/**
			 * \brief Write bytes to the card.
			 * \param blockno The block number.
			 * \param buf The buffer containing the data.
			 * \param buflen The length of buffer.
			 * \return The count of bytes written.
			 */
			virtual size_t updateBinary(unsigned char blockno, const void* buf, size_t buflen);

	protected:
			/**
			 * \brief Read bytes from the card.
			 * \param keyindex The key index.
			 * \param blockno The block number.
			 * \param len The count of bytes to read. (0 <= len < 16)
			 * \param buf The buffer in which to place the data.
			 * \param buflen The length of buffer.
			 * \return The count of bytes red.
			 */
			virtual size_t readBinaryIndex(unsigned char keyindex, unsigned char blockno, size_t len, void* buf, size_t buflen);

			/**
			 * \brief Write bytes to the card.
			 * \param keyindex The key index.
			 * \param blockno The block number.
			 * \param buf The buffer containing the data.
			 * \param buflen The length of buffer.
			 * \return The count of bytes written.
			 */
			virtual size_t updateBinaryIndex(unsigned char keyindex, unsigned char blockno, const void* buf, size_t buflen);

			/**
			 * \brief Use SKB for read/write operation, or not.
			 */
			bool d_useSKB;

			/**
			 * \brief The SKB key index if SKB use.
			 */
			unsigned char d_skbIndex;

			/**
			 * \brief The last key type used;
			 */
			MifareKeyType d_lastKeyType;
	};	
}

#endif /* LOGICALACCESS_MIFARESTIDSTRCOMMANDS_H */


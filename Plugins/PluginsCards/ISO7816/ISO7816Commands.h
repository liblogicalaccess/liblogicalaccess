/**
 * \file ISO7816Commands.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 commands.
 */

#ifndef LOGICALACCESS_ISO7816COMMANDS_H
#define LOGICALACCESS_ISO7816COMMANDS_H

#include "logicalaccess/Key.h"
#include "ISO7816Location.h"
#include "logicalaccess/Cards/Commands.h"


namespace logicalaccess
{
	/**
	 * \brief The ISO7816 commands class.
	 */
	class LIBLOGICALACCESS_API ISO7816Commands : public virtual Commands
	{
		public:

			/**
			 * \brief Read binary data.
			 * \param data The buffer that will contains data.
			 * \param dataLength The buffer length.
			 * \param offset The read offset.
			 * \param efid The EF identifier to set as current.
			 * \return True on success, false otherwise.
			 */
			virtual bool readBinay(void* data, size_t& dataLength, size_t offset, short efid = 0) = 0;

			/**
			 * \brief Write binary data.
			 * \param data The buffer that contains data to write.
			 * \param dataLength The buffer length.
			 * \param offset The write offset.
			 * \param efid The EF identifier to set as current.
			 * \return True on success, false otherwise.
			 */
			virtual void writeBinay(const void* data, size_t dataLength, size_t offset, short efid = 0) = 0;

			/**
			 * \brief Update binary data.
			 * \param data The buffer that contains data to update.
			 * \param dataLength The buffer length.
			 * \param offset The update offset.
			 * \param efid The EF identifier to set as current.
			 * \return True on success, false otherwise.
			 */
			virtual void updateBinay(const void* data, size_t dataLength, size_t offset, short efid = 0) = 0;

			/**
			 * \brief Erase binary data.
			 * \param offset The erase offset.
			 * \param efid The EF identifier to set as current.
			 * \return True on success, false otherwise.
			 */
			virtual void eraseBinay(size_t offset, short efid = 0) = 0;

			// Don't implement record management yet.

			/**
			 * \brief Get data.
			 * \param data The buffer that will contains data.
			 * \param dataLength The buffer length.
			 * \param dataObject The data object to get.
			 * \return True on success, false otherwise.
			 */
			virtual bool getData(void* data, size_t& dataLength, unsigned short dataObject) = 0;

			/**
			 * \brief Put data.
			 * \param data The buffer that contains data.
			 * \param dataLength The buffer length.
			 * \param dataObject The data object to get.
			 * \return True on success, false otherwise.
			 */
			virtual void putData(const void* data, size_t dataLength, unsigned short dataObject) = 0;

			/**
			 * \brief Select a file by the file identifier.
			 * \param efid The file identifier.
			 * \return True on success, false otherwise.
			 */
			virtual void selectFile(unsigned short efid) = 0;

			/**
			 * \brief Select a file by the DF name.
			 * \param dfname The DF name.
			 * \param dfnamelen The DF name length.
			 * \return True on success, false otherwise.
			 */
			virtual void selectFile(unsigned char* dfname, size_t dfnamelen) = 0;

			// Don't implement security management yet.
	};
}

#endif

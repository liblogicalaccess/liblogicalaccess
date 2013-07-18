/**
 * \file iso7816iso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 commands.
 */

#ifndef LOGICALACCESS_ISO7816ISO7816COMMANDS_HPP
#define LOGICALACCESS_ISO7816ISO7816COMMANDS_HPP

#include "iso7816commands.hpp"
#include "../iso7816readerunit.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "../readercardadapters/iso7816readercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>


namespace logicalaccess
{
	/**
	 * \brief The ISO7816 commands base class.
	 */
	class LIBLOGICALACCESS_API ISO7816ISO7816Commands : public ISO7816Commands
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			ISO7816ISO7816Commands();

			/**
			 * \brief Destructor.
			 */
			virtual ~ISO7816ISO7816Commands();

			/**
			 * \brief Read binary data.
			 * \param data The buffer that will contains data.
			 * \param dataLength The buffer length.
			 * \param offset The read offset.
			 * \param efid The EF identifier to set as current.
			 * \return True on success, false otherwise.
			 */
			virtual bool readBinay(void* data, size_t& dataLength, size_t offset, short efid = 0);

			/**
			 * \brief Write binary data.
			 * \param data The buffer that contains data to write.
			 * \param dataLength The buffer length.
			 * \param offset The write offset.
			 * \param efid The EF identifier to set as current.
			 * \return True on success, false otherwise.
			 */
			virtual void writeBinay(const void* data, size_t dataLength, size_t offset, short efid = 0);

			/**
			 * \brief Update binary data.
			 * \param data The buffer that contains data to update.
			 * \param dataLength The buffer length.
			 * \param offset The update offset.
			 * \param efid The EF identifier to set as current.
			 * \return True on success, false otherwise.
			 */
			virtual void updateBinay(const void* data, size_t dataLength, size_t offset, short efid = 0);

			/**
			 * \brief Erase binary data.
			 * \param offset The erase offset.
			 * \param efid The EF identifier to set as current.
			 * \return True on success, false otherwise.
			 */
			virtual void eraseBinay(size_t offset, short efid = 0);

			/**
			 * \brief Get data.
			 * \param data The buffer that will contains data.
			 * \param dataLength The buffer length.
			 * \param dataObject The data object to get.
			 * \return True on success, false otherwise.
			 */
			virtual bool getData(void* data, size_t& dataLength, unsigned short dataObject);

			/**
			 * \brief Put data.
			 * \param data The buffer that contains data.
			 * \param dataLength The buffer length.
			 * \param dataObject The data object to get.
			 * \return True on success, false otherwise.
			 */
			virtual void putData(const void* data, size_t dataLength, unsigned short dataObject);

			/**
			 * \brief Select a file by the file identifier.
			 * \param efid The file identifier.
			 * \return True on success, false otherwise.
			 */
			virtual void selectFile(unsigned short efid);

			/**
			 * \brief Select a file by the DF name.
			 * \param dfname The DF name.
			 * \param dfnamelen The DF name length.
			 * \return True on success, false otherwise.
			 */
			virtual void selectFile(unsigned char* dfname, size_t dfnamelen);

			/**
			 * \brief Select a file.
			 * \param p1 The parameter 1.
			 * \param p2 The parameter 2.
			 * \param data The data buffer.
			 * \param datalen The buffer length.
			 */
			virtual void selectFile(unsigned char p1, unsigned char p2, unsigned char* data, size_t datalen);
		
			/**
			 * \brief Get the ISO7816 reader/card adapter.
			 * \return The ISO7816 reader/card adapter.
			 */
			boost::shared_ptr<ISO7816ReaderCardAdapter> getISO7816ReaderCardAdapter() { return boost::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(getReaderCardAdapter()); };


			/**
			 * \brief Set the SAM PCSCReaderUnit.
			 * \param t The SAM PCSCReaderUnit.
			 */
			void setSAMReaderUnit(boost::shared_ptr<ISO7816ReaderUnit> t);
			
			/**
			 * \brief get the SAM PCSCReaderUnit.
			 */
			boost::shared_ptr<ISO7816ReaderUnit> getSAMReaderUnit();

			/**
			 * \brief Find the type of SAM from the reader set.
			 */
			std::string AutoSelectSAMType();

		protected:

			/**
			 * \brief Set P1 and P2 parameters.
			 * \param offset The starting offset.
			 * \param efid The EF identifier.
			 * \param p1 The parameter 1.
			 * \param p2 The parameter 2.
			 */
			void setP1P2(size_t offset, short efid, unsigned char& p1, unsigned char& p2);

			/**
			 * \brief The PCSCReaderUnit used for the SAM.
			 */
			boost::shared_ptr<ISO7816ReaderUnit> d_SAM_readerunit;
	};
}

#endif

/**
 * \file WiegandFormat.h
 * \author Arnaud HUMILIER <arnaud.humilier@islog.eu>, Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief WiegandFormat.
 */

#ifndef LOGICALACCESS_WIEGANDFORMAT_H
#define LOGICALACCESS_WIEGANDFORMAT_H	

#include "logicalaccess/Services/AccessControl/Formats/StaticFormat.h"

namespace LOGICALACCESS
{
	/**
	 * \brief A Wiegand format.
	 */
	class LIBLOGICALACCESS_API WiegandFormat : public StaticFormat
	{
		public:
			/**
			 * \brief Constructor.
			 *
			 * Create a Wiegand Format.
			 */
			WiegandFormat();

			/**
			 * \brief Destructor.
			 *
			 * Release the Wiegand Format.
			 */
			virtual ~WiegandFormat();

			/**
			 * \brief Get left parity byte.
			 * \return The left parity.
			 */
			unsigned char getLeftParity() const;

			/**
			 * \brief Get right parity byte.
			 * \return The right parity.
			 */
			unsigned char getRightParity() const;

			/**
			 * \brief Get linear data.
			 * \param data Where to put data
			 * \param dataLengthBytes Length in byte of data
			 */
			virtual void getLinearData(void* data, size_t dataLengthBytes) const;

			/**
			 * \brief Get linear data without parity
			 * \param data Buffer to be modified. Should be allocated.
			 * \param dataLengthBytes Length in byte of data
			 */
			virtual void getLinearDataWithoutParity(void* data, size_t dataLengthBytes) const = 0;

			/**
			 * \brief Set linear data
			 * \param data Where to get data
			 * \param dataLengthBytes Length of data in bytes
			 */
			virtual void setLinearData(const void* data, size_t dataLengthBytes);

			/**
			 * \brief Write linear data without parity into data buffer
			 * \param data Buffer where data will be written.
			 * \param dataLengthBytes Length of data in bytes
			 */
			virtual void setLinearDataWithoutParity(const void* data, size_t dataLengthBytes) = 0;

		protected:

			/**
			 * \brief The left parity length.
			 */
			unsigned int d_leftParityLength;

			/**
			 * \brief The left parity type.
			 */
			ParityType d_leftParityType;


			/**
			 * \brief The right parity length.
			 */
			unsigned int d_rightParityLength;

			/**
			 * \brief The right parity type.
			 */
			ParityType d_rightParityType;
	};	
}

#endif /* LOGICALACCESS_WIEGANDFORMAT_H */

/**
 * \file A3MLGM5600ReaderCardAdapter.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Default A3MLGM5600 reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTA3MLGM5600READERCARDADAPTER_H
#define LOGICALACCESS_DEFAULTA3MLGM5600READERCARDADAPTER_H

#include "logicalaccess/Cards/ReaderCardAdapter.h"
#include "../A3MLGM5600ReaderUnit.h"
#include "logicalaccess/ReaderProviders/ISO14443ReaderCommunication.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace LOGICALACCESS
{	
	/**
	 * \brief A default A3MLGM5600 reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API A3MLGM5600ReaderCardAdapter : public ReaderCardAdapter, public ISO14443ReaderCommunication
	{
		public:

			/**
			 * \brief Constructor.
			 */
			A3MLGM5600ReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~A3MLGM5600ReaderCardAdapter();

			static const unsigned char STX = 0x02; /**< \brief The start value. */
			static const unsigned char ETX = 0x03; /**< \brief The stop value. */
			
			/**
			 * \brief Send a command to the reader without waiting for a response.
			 * \param cmd The command code.
			 * \param data The command data buffer.
			 * \return True if the command was send successfully, false otherwise.
			 */
			bool sendCommandWithoutResponse(unsigned char cmd, const std::vector<unsigned char>& data);

			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout = 3000);

			/**
			 * \brief Send a command to the reader.
			 * \param cmd The command code.
			 * \param data The command data buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(unsigned char cmd, const std::vector<unsigned char>& data, long int timeout = 3000);

			/**
			 * \brief Wait for a command.
			 * \param buf The buffer into which to put the received data.
			 * \param timeout The timeout value, in milliseconds. If timeout is negative, the call never times out.
			 * \return true if a command was received, false otherwise.
			 */
			bool receiveCommand(std::vector<unsigned char>& buf, long int timeout = 2000);
			
			/**
			 * \brief Get the A3MLGM5600 reader unit.
			 * \return The A3MLGM5600 reader unit.
			 */
			boost::shared_ptr<A3MLGM5600ReaderUnit> getA3MLGM5600ReaderUnit() const { return boost::dynamic_pointer_cast<A3MLGM5600ReaderUnit>(getReaderUnit()); };			
		
			/**
			 * \brief Send a REQA command from the PCD to the PICC.
			 * \return The ATQB PICC result.
			 */
			virtual std::vector<unsigned char> requestA();

			/**
			 * \brief Send a RATS command from the PCD to the PICC.
			 * \return The ATS PICC result.
			 */
			virtual std::vector<unsigned char> requestATS();

			/**
			 * \brief Send a HLTB command from the PCD to the PICC.
			 */
			virtual void haltA();

			/**
			 * \brief Manage collision.
			 * \return The chip UID.
			 */
			virtual std::vector<unsigned char> anticollisionA();

			/**
			 * \brief Send a REQB command from the PCD to the PICC.
			 * \param afi The AFI value.
			 * \return The ATQB PICC result.
			 */
			virtual std::vector<unsigned char> requestB(unsigned char afi = 0x00);

			/**
			 * \brief Send a HLTB command from the PCD to the PICC.
			 */
			virtual void haltB();

			/**
			 * \brief Send a attrib command from the PCD to the PICC.
			 */
			virtual void attrib();			

			/**
			 * \brief Manage collision.
			 * \param afi The AFI value.
			 * \return The chip UID.
			 */
			virtual std::vector<unsigned char> anticollisionB(unsigned char afi = 0x00);
			

		protected:

			/**
			 * \brief Calculate command checksum.
			 * \param data The data to calculate checksum
			 * \return The checksum.
			 */
			unsigned char calcBCC(const std::vector<unsigned char>& data);
			
			/**
			 * \brief Handle a command buffer and give the associated data buffer.
			 * \param cmdbuf The command buffer.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleCommandBuffer(const std::vector<unsigned char>& cmdbuf);		

			/**
			 * \brief The current sequence number.
			 */
			unsigned char d_seq;

			/**
			 * \brief The last command response status.
			 */
			unsigned char d_command_status;
	};

}

#endif /* LOGICALACCESS_DEFAULTA3MLGM5600READERCARDADAPTER_H */

 

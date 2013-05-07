/**
 * \file SmartIDReaderCardAdapter.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default SmartID reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTSMARTIDREADERCARDADAPTER_H
#define LOGICALACCESS_DEFAULTSMARTIDREADERCARDADAPTER_H

#include "ReaderCardAdapters/ISO7816ReaderCardAdapter.h"
#include "../SmartIDReaderUnit.h"
#include "logicalaccess/ReaderProviders/ISO14443ReaderCommunication.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace LOGICALACCESS
{	
	/**
	 * \brief A default SmartID reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API SmartIDReaderCardAdapter : public ISO7816ReaderCardAdapter, public ISO14443ReaderCommunication
	{
		public:

			/**
			 * \brief Constructor.
			 */
			SmartIDReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~SmartIDReaderCardAdapter();

			static const unsigned char DLE; /**< \brief The DLE value. */
			static const unsigned char STX; /**< \brief The STX value. */
			static const unsigned char ETX; /**< \brief The ETX value. */
			static const unsigned char NAK; /**< \brief The NAK value. */

			static const unsigned char RET_OK; /**< \brief The RET_OK value. */
			static const unsigned char RET_OK_CHAIN; /**< \brief The RET_OK_CHAIN value. */
			static const unsigned char RET_NOTAG; /**< \brief The RET_NOTAG value. */
			static const unsigned char RET_FORMATERR; /**< \brief The RET_FORMATERR value. */
			static const unsigned char RET_E2PROMERR; /**< \brief The RET_E2PROMERR value. */
			static const unsigned char RET_ENVNOTFOUND; /**< \brief The RET_ENVNOTFOUND value. */
			static const unsigned char RET_NY_IMPLEMENTED; /**< \brief The RET_NY_IMPLEMENTED value. */

			/**
			 * \brief Send an APDU command to the reader.
			 */
			virtual void sendAPDUCommand(const std::vector<unsigned char>& command, unsigned char* result, size_t* resultlen);

			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout = 2000);

			/**
			 * \brief Send a command to the reader.
			 * \param cmd The command code.
			 * \param command The command buffer.
			 * \param timeout The command timeout. Default is 2000 ms.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(unsigned char cmd, const std::vector<unsigned char>& command, long int timeout = 2000);
					
			/**
			 * \brief Get the SmartID reader unit.
			 * \return The SmartID reader unit.
			 */
			boost::shared_ptr<SmartIDReaderUnit> getSmartIDReaderUnit() const { return boost::dynamic_pointer_cast<SmartIDReaderUnit>(getReaderUnit()); };			

			/**
			 * \brief Wait for a command.
			 * \param buf The buffer into which to put the received data.
			 * \param sta The received status code.
			 * \param timeout The timeout value, in milliseconds. If timeout is negative, the call never times out.
			 * \return true if a command was received, false otherwise.
			 */
			bool receiveCommand(std::vector<unsigned char>& buf, unsigned char& sta, long int timeout = 1000);

			/**
			 * \brief Send a halt command.
			 */
			void tclHalt();

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
			 * \brief Handle a command buffer and give the associated data buffer.
			 * \param cmdbuf The command buffer.
			 * \param sta The received status.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleCommandBuffer(const std::vector<unsigned char>& cmdbuf, unsigned char& sta);
	};

}

#endif /* LOGICALACCESS_DEFAULTSMARTIDREADERCARDADAPTER_H */

 

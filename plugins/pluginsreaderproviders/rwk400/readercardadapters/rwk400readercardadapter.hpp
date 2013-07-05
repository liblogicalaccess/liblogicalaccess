/**
 * \file rplethreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Rpleth reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTRWK400READERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTRWK400READERCARDADAPTER_HPP

#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/readerproviders/iso14443readercommunication.hpp"
#include "../rwk400readerunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{	

	typedef enum
	{
		SELECT_CURRENT_KEY = 0x52,
		DIVERSIFY_KEY = 0X52,
		SELECT_CARD = 0XA4,
		PAGE_SELECT = 0XA6,
		TRANSMIT = 0XC2,
		RETREIVE_DATA = 0XC0,
		LED_SOUNDER_CONTROL = 0X70,
		OPEN_COLLECTOR_OUTPUT = 0X71,
		LOAD_KEY = 0XD8,
		ASK_RANDOM = 0X84,
		READ_EEPROM = 0XF2,
		WRITE_EEPROM = 0XF4,
		RESET_RF = 0XF4,
		ENCRYPT_DECRYPT = 0X54,
		RATS = 0xE0
	} RWK400Commands;

	/**
	 * \brief A default Rpleth reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API Rwk400ReaderCardAdapter : public ReaderCardAdapter, public ISO14443ReaderCommunication
	{
		public:

			/**
			 * \brief Constructor.
			 */
			Rwk400ReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~Rwk400ReaderCardAdapter();

			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout = 2000);

			/**
			 * \brief Receive the reader answer.
			 * \param buf The sand command buffer.		 
			 * \param timeout The answer timeout.
			 * \return The answer.
			 */
			bool receiveAnwser(std::vector<unsigned char>& buf, long int timeout);
			
			/**
			 * \brief Get the Rpleth reader unit.
			 * \return The Rpleth reader unit.
			 */
			boost::shared_ptr<Rwk400ReaderUnit> getRwk400ReaderUnit() const { return boost::dynamic_pointer_cast<Rwk400ReaderUnit>(getReaderUnit()); };			
			
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
			 * \brief Handle a answer and give the associated data buffer.
			 * \param ansbuf The answer buffer.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleAnswerBuffer(const std::vector<unsigned char>& ansbuf);

			bool isRWK400Commands (unsigned char source);

			RWK400Commands toRWK400Commands (unsigned char source);

			unsigned char fromRWK400Commands (RWK400Commands source);
	};

}

#endif /* LOGICALACCESS_DEFAULTRWK400READERCARDADAPTER_HPP */

 

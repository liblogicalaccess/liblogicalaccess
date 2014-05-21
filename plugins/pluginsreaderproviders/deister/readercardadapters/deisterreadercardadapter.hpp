/**
 * \file deisterreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Deister reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTDEISTERREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTDEISTERREADERCARDADAPTER_HPP

#include "logicalaccess/cards/readercardadapter.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{	
	/**
	 * \brief A default Deister reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API DeisterReaderCardAdapter : public ReaderCardAdapter
	{
		public:

			/**
			 * \brief Constructor.
			 */
			DeisterReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~DeisterReaderCardAdapter();

			static const unsigned char SHFT; /**< \brief The shift value. */
			static const unsigned char SOM; /**< \brief The start of message value. */
			static const unsigned char SOC; /**< \brief The start of command value. */
			static const unsigned char STOP; /**< \brief The stop value, end of message. */
			static const unsigned char Dummy; /**< \brief The dummy value. */
			static const unsigned char SHFT_SHFT; /**< \brief The shift 00 value to replace SHFT value in message. */
			static const unsigned char SHFT_SOM; /**< \brief The shift 00 value to replace SOM value in message. */
			static const unsigned char SHFT_SOC; /**< \brief The shift 00 value to replace SOC value in message. */
			static const unsigned char SHFT_STOP; /**< \brief The shift 00 value to replace STOP value in message. */

			/**
			 * \brief Adapt the command to send to the reader.
			 * \param command The command to send.
			 * \return The adapted command to send.
			 */
			virtual std::vector<unsigned char> adaptCommand(const std::vector<unsigned char>& command);

			/**
			 * \brief Adapt the asnwer received from the reader.
			 * \param answer The answer received.
			 * \return The adapted answer received.
			 */
			virtual std::vector<unsigned char> adaptAnswer(const std::vector<unsigned char>& answer);

		protected:
			
			/**
			 * \brief Prepare data buffer for device.
			 * \param data The data buffer to prepare.
			 * \return The data buffer ready to send to device.
			 */
			static std::vector<unsigned char> prepareDataForDevice(const std::vector<unsigned char>& data);

			/**
			 * \brief Prepare data buffer from device.
			 * \param data The data buffer to prepare.
			 * \return The data buffer ready to use.
			 */
			static std::vector<unsigned char> prepareDataFromDevice(const std::vector<unsigned char>& data);

			/**
			 * \brief Target device bus address destination.
			 */
			unsigned char d_destination;

			/**
			 * \brief Transmitter bus address source.
			 */
			unsigned char d_source;
	};

}

#endif /* LOGICALACCESS_DEFAULTDEISTERREADERCARDADAPTER_HPP */

 

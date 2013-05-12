/**
 * \file A3MLGM5600LEDBuzzerDisplay.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 LED/Buzzer Display. 
 */

#ifndef D3L_PCSC_RPLETHLEDBUZZERDISPLAY_H
#define D3L_PCSC_RPLETHLEDBUZZERDISPLAY_H

#include "ReaderCardAdapters/RplethReaderCardAdapter.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace logicalaccess
{
	/**
	 * \brief A A3MLGM5600 LED/Buzzer Display class.
	 */
	class LIBLOGICALACCESS_API RplethLEDBuzzerDisplay : public LEDBuzzerDisplay
	{
		public:
			/**
			 * \brief Constructor.
			 */
			RplethLEDBuzzerDisplay();

			/**
			 * \brief Set the green led to a status.
			 * \param status True to show the green led, false otherwise.
			 */
			virtual void setGreenLed(bool status);

			/**
			 * \brief Set the red led to a status.
			 * \param status True to show the red led, false otherwise.
			 */
			virtual void setRedLed(bool status);

			/**
			 * \brief Set the buzzer to a status.
			 * \param status True to play the buzzer, false otherwise.
			 */
			virtual void setBuzzer(bool status);

			/**
			 * \brief Upload led status.
			 * \see setRedLed
			 * \see setGreenLed
			 */
			void setLED();

			boost::shared_ptr<RplethReaderCardAdapter> getRplethReaderCardAdapter() { return boost::dynamic_pointer_cast<RplethReaderCardAdapter>(getReaderCardAdapter()); };

		protected:

			/**
			 * \brief The red led status.
			 */
			bool d_red_led;

			/**
			 * \brief The green led status.
			 */
			bool d_green_led;

			/**
			 * \brief The buzzer status.
			 */
			bool d_buzzer;
	};
}

#endif /* D3L_PCSC_A3MLGM5600LEDBUZZERDISPLAY_H */
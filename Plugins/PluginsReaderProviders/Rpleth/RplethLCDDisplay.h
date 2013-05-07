/**
 * \file RplethLCDDisplay.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Rpleth LCD Display. 
 */

#ifndef D3L_PCSC_RPLETHLCDDISPLAY_H
#define D3L_PCSC_RPLETHLCDDISPLAY_H

#include "logicalaccess/ReaderProviders/LCDDisplay.h"
#include "ReaderCardAdapters/RplethReaderCardAdapter.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"


namespace LOGICALACCESS
{

	/**
	 * \brief A Rpleth LCD Display class.
	 */
	class LIBLOGICALACCESS_API RplethLCDDisplay : public LCDDisplay
	{
		public:

			/**
			 * \brief Constructor.
			 */
			RplethLCDDisplay();

			/**
			 * \brief Write a message on screen.
			 * \param message The message to show.
			 */
			virtual void setMessage(string message);

			/**
			 * \brief Write a message on screen during a time.
			 * \param message The message to show.
			 * \param time The time to show the message
			 */
			virtual void setMessage(string message, int time);

			/**
			 * \brief Set the default time to display a message.
			 * \param time The time to show the message
			 */
			void setDisplayTime(int time);

			/**
			 * \brief Enable or disable the blink.
			 */
			void blink();

			/**
			 * \brief Enable or disable the autoScroll.
			 */
			void autoScroll();

			boost::shared_ptr<RplethReaderCardAdapter> getRplethReaderCardAdapter() { return boost::dynamic_pointer_cast<RplethReaderCardAdapter>(getReaderCardAdapter()); };
	};

}

#endif /* D3L_PCSC_A3MLGM5600LCDDISPLAY_H */


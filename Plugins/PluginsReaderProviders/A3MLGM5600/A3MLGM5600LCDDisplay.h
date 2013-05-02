/**
 * \file A3MLGM5600LCDDisplay.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief A3MLGM5600 LCD Display. 
 */

#ifndef LOGICALACCESS_A3MLGM5600LCDDISPLAY_H
#define LOGICALACCESS_A3MLGM5600LCDDISPLAY_H

#include "logicalaccess/ReaderProviders/LCDDisplay.h"
#include "ReaderCardAdapters/A3MLGM5600ReaderCardAdapter.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"


namespace LOGICALACCESS
{
	/**
	 * \brief A A3MLGM5600 LCD Display class.
	 */
	class LIBLOGICALACCESS_API A3MLGM5600LCDDisplay : public LCDDisplay
	{
		public:

			/**
			 * \brief Constructor.
			 */
			A3MLGM5600LCDDisplay();

			/**
			 * \brief Write a message on screen.
			 * \param message The message to show.
			 */
			virtual void setMessage(std::string message);

			/**
			 * \brief Write a message on screen.
			 * \param rowid The row id for the message.
			 * \param message The message to show.
			 */
			virtual void setMessage(unsigned char rowid, std::string message);

			boost::shared_ptr<A3MLGM5600ReaderCardAdapter> getA3MLGM5600ReaderCardAdapter() { return boost::dynamic_pointer_cast<A3MLGM5600ReaderCardAdapter>(getReaderCardAdapter()); };
	};

}

#endif /* LOGICALACCESS_A3MLGM5600LCDDISPLAY_H */


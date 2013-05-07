/**
 * \file LCDDisplay.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief LCD Display. 
 */

#ifndef LOGICALACCESS_LCDDISPLAY_H
#define LOGICALACCESS_LCDDISPLAY_H

#include "logicalaccess/XmlSerializable.h"

#include "logicalaccess/logs.h"

namespace LOGICALACCESS
{
	class ReaderCardAdapter;

	/**
	 * \brief A LCD Display base class to display message on LCD screen.
	 */
	class LIBLOGICALACCESS_API LCDDisplay
	{
		public:

			/**
			 * \brief Write a message on screen.
			 * \param message The message to show.
			 */
			virtual void setMessage(std::string message) = 0;

			/**
			 * \brief Write a message on screen.
			 * \param rowid The row id for the message.
			 * \param message The message to show.
			 */
			virtual void setMessage(unsigned char rowid, std::string message) = 0;

			/**
			 * \brief Get the reader card adapter.
			 * \return The reader card adapter.
			 */
			boost::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const { return d_readerCardAdapter; };

			/**
			 * \brief Set the reader card adapter.
			 * \param adapter The reader card adapter.
			 */
			void setReaderCardAdapter(boost::shared_ptr<ReaderCardAdapter> adapter) { d_readerCardAdapter = adapter; };


		protected:

			/**
			 * \brief The reader card adapter.
			 */
			boost::shared_ptr<ReaderCardAdapter> d_readerCardAdapter;
	};
}

#endif /* LOGICALACCESS_LCDDISPLAY_H */


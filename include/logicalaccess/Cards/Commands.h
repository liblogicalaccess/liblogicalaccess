/**
 * \file Commands.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Commands.
 */

#ifndef LOGICALACCESS_COMMANDS_H
#define LOGICALACCESS_COMMANDS_H

namespace logicalaccess
{
	/**
	 * \brief The base commands class for all card commands.
	 */
	class LIBLOGICALACCESS_API Commands
	{
		public:

			/**
			 * \brief Get the reader/card adapter.
			 * \return The reader/card adapter.
			 */
			virtual boost::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const { return d_readerCardAdapter; };

			/**
			 * \brief Set the reader/card adapter.
			 * \param adapter The reader/card adapter.
			 */
			virtual void setReaderCardAdapter(boost::shared_ptr<ReaderCardAdapter> adapter) { d_readerCardAdapter = adapter; };

		protected:

			/**
			 * \brief The reader/card adapter.
			 */
			boost::shared_ptr<ReaderCardAdapter> d_readerCardAdapter;
	};
}

#endif

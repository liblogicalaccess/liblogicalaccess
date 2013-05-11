/**
 * \file ReaderCardAdapter.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader/card adapter. 
 */

#ifndef LOGICALACCESS_READERCARDADAPTER_H
#define LOGICALACCESS_READERCARDADAPTER_H

#include "logicalaccess/ReaderProviders/ReaderProvider.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace logicalaccess
{
	/**
	 * \brief A reader/card adapter base class. It provide an abstraction layer between the card and the reader to send chip command.
	 */
	class LIBLOGICALACCESS_API ReaderCardAdapter
	{
		public:

			/**
			 * \brief Get the reader unit.
			 * \return The reader unit.
			 */
			boost::shared_ptr<ReaderUnit> getReaderUnit() const { return d_ReaderUnit.lock(); };

			/**
			 * \brief Set the reader unit.
			 * \param unit The reader unit.
			 */
			void setReaderUnit(boost::weak_ptr<ReaderUnit> unit){ d_ReaderUnit = unit; };

			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return the result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout) = 0;

			/**
			 * \brief Get the last command.
			 * \return The last command.
			 */
			virtual std::vector<unsigned char> getLastCommand() { return d_lastCommand; };

			/**
			 * \brief Get the last command result.
			 * \return The last command result.
			 */
			virtual std::vector<unsigned char> getLastResult() { return d_lastResult; };

		protected:
			
			/**
			 * \brief The reader unit.
			 */
			boost::weak_ptr<ReaderUnit> d_ReaderUnit;

			/**
			 * \brief The last result.
			 */
			std::vector<unsigned char> d_lastResult;

			/**
			 * \brief The last command.
			 */
			std::vector<unsigned char> d_lastCommand;
	};

}

#endif /* LOGICALACCESS_READERCARDADAPTER_H */


/**
 * \file PCSCReaderCardAdapter.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default PC/SC reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTPCSCREADERCARDADAPTER_H
#define LOGICALACCESS_DEFAULTPCSCREADERCARDADAPTER_H

#include "ReaderCardAdapters/ISO7816ReaderCardAdapter.h"
#include "../PCSCReaderProvider.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace LOGICALACCESS
{
	/**
	 * \brief A default PC/SC reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API PCSCReaderCardAdapter : public ISO7816ReaderCardAdapter
	{
		public:

			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer.
			 * \param timeout The timeout.
			 * \return the result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout);

			/**
			 * \brief Get the PC/SC reader unit.
			 * \return The PC/SC reader unit.
			 */
			boost::shared_ptr<PCSCReaderUnit> getPCSCReaderUnit() const { return boost::dynamic_pointer_cast<PCSCReaderUnit>(getReaderUnit()); };

			/**
			 * \brief Check the card error and throw exception if needed.
			 * \param errorFlag The error flag.
			 */
			virtual void CheckCardError(unsigned int errorFlag) const;

			/**
			 * \brief Check the command result and throw exception if needed.
			 * \param data The result buffer.
			 * \param datalen The result buffer length.
			 */
			virtual void CheckResult(const void* data, size_t datalen) const;

		protected:
			
			
	};

}

#endif /* LOGICALACCESS_DEFAULTPCSCREADERCARDADAPTER_H */


/**
 * \file SCIELReaderCardAdapter.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Default SCIEL reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTSCIELREADERCARDADAPTER_H
#define LOGICALACCESS_DEFAULTSCIELREADERCARDADAPTER_H

#include "logicalaccess/Cards/ReaderCardAdapter.h"
#include "../SCIELReaderUnit.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace LOGICALACCESS
{	
	/**
	 * \brief A default SCIEL reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API SCIELReaderCardAdapter : public ReaderCardAdapter
	{
		public:

			/**
			 * \brief Constructor.
			 */
			SCIELReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~SCIELReaderCardAdapter();

			static const unsigned char STX; /**< \brief The STX value. */
			static const unsigned char ETX; /**< \brief The ETX value. */
			
			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout = 2000);

			/**
			 * \brief Wait for a command.
			 * \param buf The buffer into which to put the received data.
			 * \param timeout The timeout value, in milliseconds. If timeout is negative, the call never times out.
			 * \return true if a command was received, false otherwise.
			 */
			bool receiveCommand(std::vector<unsigned char>& buf, long int timeout = 2000);

			/**
			 * \brief Wait for receiving all the tags list.
			 * \param command The command requesting the tags list.
			 * \param timeout The timeout value, in milliseconds. If timeout is negative, the call never times out.
			 * \return The list of tags detected by the reader.
			 */
			std::list<std::vector<unsigned char>> receiveTagsListCommand(const std::vector<unsigned char>& command, long int timeout = 2000);
			
			/**
			 * \brief Get the SCIEL reader unit.
			 * \return The SCIEL reader unit.
			 */
			boost::shared_ptr<SCIELReaderUnit> getSCIELReaderUnit() const { return boost::dynamic_pointer_cast<SCIELReaderUnit>(getReaderUnit()); };			
			

		protected:
			
			/**
			 * \brief Handle a command buffer and give the associated data buffer.
			 * \param cmdbuf The command buffer.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleCommandBuffer(const std::vector<unsigned char>& cmdbuf);

			/**
			 * \brief Trashed data from last command response.
			 */
			std::vector<unsigned char> d_trashedData;
	};

}

#endif /* LOGICALACCESS_DEFAULTSCIELREADERCARDADAPTER_H */

 

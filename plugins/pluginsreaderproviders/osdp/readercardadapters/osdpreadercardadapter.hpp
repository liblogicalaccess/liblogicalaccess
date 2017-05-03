/**
 * \file osdpreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader/card adapter. 
 */

#ifndef LOGICALACCESS_OSDPREADERCARDADAPTER_HPP
#define LOGICALACCESS_OSDPREADERCARDADAPTER_HPP

#include "logicalaccess/readerproviders/datatransport.hpp"
#include "iso7816/readercardadapters/iso7816readercardadapter.hpp"
#include "iso7816/commands/desfireiso7816resultchecker.hpp"
#include "../osdpcommands.hpp"

namespace logicalaccess
{
	/**
	 * \brief A reader/card adapter for Transparent Smart Card Interface OSDP.
	 */
	class LIBLOGICALACCESS_API OSDPReaderCardAdapter : public ISO7816ReaderCardAdapter
	{
		public:

			OSDPReaderCardAdapter(std::shared_ptr<OSDPCommands> command, unsigned char address, std::shared_ptr<DESFireISO7816ResultChecker> resultChecker);

			~OSDPReaderCardAdapter();

			std::shared_ptr<DataTransport> getDataTransport() const { return d_dataTransport; };

			void setDataTransport(std::shared_ptr<DataTransport> dataTransport) { d_dataTransport = dataTransport; };

			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long timeout = 3000);

		protected:
			
			std::shared_ptr<DataTransport> d_dataTransport;

			std::shared_ptr<OSDPCommands> m_commands;

			std::shared_ptr<DESFireISO7816ResultChecker> d_resultChecker;

			unsigned char m_address;
	};

}

#endif /* LOGICALACCESS_OSDPREADERCARDADAPTER_HPP */


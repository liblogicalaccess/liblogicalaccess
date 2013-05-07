/**
 * \file GunneboReaderCardAdapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Gunnebo reader/card adapter.
 */

#include "GunneboReaderCardAdapter.h"

namespace LOGICALACCESS
{			
	const unsigned char GunneboReaderCardAdapter::STX = 0x02;
	const unsigned char GunneboReaderCardAdapter::ETX = 0x03;

	GunneboReaderCardAdapter::GunneboReaderCardAdapter()
		: ReaderCardAdapter()
	{
	}

	GunneboReaderCardAdapter::~GunneboReaderCardAdapter()
	{
	}

	bool GunneboReaderCardAdapter::sendCommandWithoutResponse(const std::vector<unsigned char>& command)
	{
		COM_("Sending command command %s command size {%d}...", BufferHelper::getHex(command).c_str(), command.size());
		
		bool ret = false;
		d_lastCommand = command;
		d_lastResult.clear();

		if (command.size() > 0)
		{
			getGunneboReaderUnit()->connectToReader();

			boost::shared_ptr<SerialPortXml> port = getGunneboReaderUnit()->getSerialPort();
			port->getSerialPort()->write(command);

			ret = true;
		}

		return ret;
	}

	std::vector<unsigned char> GunneboReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		COM_("Sending command %s command size {%d} timeout {%d}...", BufferHelper::getHex(command).c_str(), command.size(), timeout);

		std::vector<unsigned char> r;
		if (sendCommandWithoutResponse(command))
		{
			if (!receiveCommand(r, timeout))
			{
				r.clear();
				THROW_EXCEPTION_WITH_LOG(CardException, "No response received");
			}
			else
			{
				COM_("Response received successfully ! Reponse: %s size {%d}", BufferHelper::getHex(r).c_str(), r.size());
			}

			d_lastResult = r;
		}

		return r;
	}

	std::vector<unsigned char> GunneboReaderCardAdapter::handleCommandBuffer(const std::vector<unsigned char>& cmdbuf)
	{
		COM_("Processing the received command buffer %s command size {%d}...", BufferHelper::getHex(cmdbuf).c_str(), cmdbuf.size());

		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() >= 3, std::invalid_argument, "A valid command buffer size must be at least 3 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf[0] == STX, std::invalid_argument, "The supplied command buffer is not valid (bad STX)");
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf[cmdbuf.size()-2] == ETX, std::invalid_argument, "The supplied command buffer is not valid (bad ETX)");
		
		// Gunnebo ID is like that: [STX - 1 byte][id - x bytes][ETX - 1 byte][checksum - 1 byte]
		// We don't calculate the checksum ! Who cares ?!? :)

		return std::vector<unsigned char>(cmdbuf.begin() + 1, cmdbuf.end() - 2);
	}

	bool GunneboReaderCardAdapter::receiveCommand(std::vector<unsigned char>& buf, long int timeout)
	{
		COM_("Beginning receiving response... Timeout configured {%d}", timeout);

		std::vector<unsigned char> res;
		boost::shared_ptr<SerialPortXml> port = getGunneboReaderUnit()->getSerialPort();
		bool timeoutOccured = true;

		boost::posix_time::time_duration ptimeout = boost::posix_time::milliseconds(timeout);
		while (port->getSerialPort()->select(ptimeout))
		{
			std::vector<unsigned char> tmpbuf;
			timeoutOccured = false;

			while (port->getSerialPort()->read(tmpbuf, 256) > 0)
			{
				COM_("Data read on the serial port %s", BufferHelper::getHex(tmpbuf).c_str());

				res.insert(res.end(), tmpbuf.begin(), tmpbuf.end());
				tmpbuf.clear();

				try
				{
					buf = handleCommandBuffer(res);
					return true;
				}
				catch(LibLOGICALACCESSException& e)
				{
					COM_("Exception {%s}", e.what());
					throw;
				}
				catch (std::invalid_argument& e)
				{
					COM_("Exception {%s}", e.what());
				}
			}
		}

		if (timeoutOccured)
		{
			COM_SIMPLE_("Timeout reached !");
		}
		else
		{
			COM_SIMPLE_("Reponse command not received !");
		}
		return false;
	}	
}

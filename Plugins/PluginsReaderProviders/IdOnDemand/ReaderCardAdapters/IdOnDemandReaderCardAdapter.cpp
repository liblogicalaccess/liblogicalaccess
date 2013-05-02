/**
 * \file IdOnDemandReaderCardAdapter.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief IdOnDemand reader/card adapter.
 */

#include "IdOnDemandReaderCardAdapter.h"

namespace LOGICALACCESS
{		
	IdOnDemandReaderCardAdapter::IdOnDemandReaderCardAdapter()
		: ReaderCardAdapter()
	{
	}

	IdOnDemandReaderCardAdapter::~IdOnDemandReaderCardAdapter()
	{
	}

	bool IdOnDemandReaderCardAdapter::sendCommandWithoutResponse(const std::vector<unsigned char>& command)
	{
		COM_("Sending command with command %s command size {%d} {%s}...", BufferHelper::getHex(command).c_str(), command.size(), BufferHelper::getStdString(command).c_str());
		std::vector<unsigned char> cmd = command;
		cmd.push_back(0x0d);
		d_lastCommand = command;
		d_lastResult.clear();

		getIdOnDemandReaderUnit()->connectToReader();

		boost::shared_ptr<SerialPortXml> port = getIdOnDemandReaderUnit()->getSerialPort();
		port->getSerialPort()->write(cmd);		

		return true;
	}

	std::vector<unsigned char> IdOnDemandReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		COM_("Sending command %s command size {%d} timeout {%d} {%s}...", BufferHelper::getHex(command).c_str(), command.size(), timeout, BufferHelper::getStdString(command).c_str());

		std::vector<unsigned char> r;
		if (sendCommandWithoutResponse(command))
		{
#ifdef _WINDOWS
				Sleep(100);
#elif defined(LINUX)
				usleep(100000);
#endif
			if (!receiveCommand(r, timeout))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, "No response received");
			}

			d_lastResult = r;
		}

		return r;
	}

	std::vector<unsigned char> IdOnDemandReaderCardAdapter::handleCommandBuffer(const std::vector<unsigned char>& cmdbuf)
	{
		COM_("Processing the received command buffer %s command size {%d} {%s}...", BufferHelper::getHex(cmdbuf).c_str(), cmdbuf.size(), BufferHelper::getStdString(cmdbuf).c_str());

		std::vector<unsigned char> ret;
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() >= 2, std::invalid_argument, "A valid command buffer size must be at least 2 bytes long");
		
		std::string strcmdbuf = BufferHelper::getStdString(cmdbuf);

		EXCEPTION_ASSERT_WITH_LOG(strcmdbuf.find("ERROR ") == string::npos, CardException, "ERROR code returned.");
		EXCEPTION_ASSERT_WITH_LOG(strcmdbuf.find("OK ") != string::npos, std::invalid_argument, "The command doesn't return OK code.");
		
		return ret;
	}

	bool IdOnDemandReaderCardAdapter::receiveCommand(std::vector<unsigned char>& buf, long int timeout)
	{		
		COM_("Beginning receiving response... Timeout configured {%d}", timeout);
		std::vector<unsigned char> res;
		boost::shared_ptr<SerialPortXml> port = getIdOnDemandReaderUnit()->getSerialPort();
		bool timeoutOccured = true;

		boost::posix_time::time_duration ptimeout = boost::posix_time::milliseconds(timeout);
		while (port->getSerialPort()->select(ptimeout))
		{
			timeoutOccured = false;
			std::vector<unsigned char> tmpbuf;

			while (port->getSerialPort()->read(tmpbuf, 256) > 0)
			{
				COM_("Data read on the serial port %s {%s}", BufferHelper::getHex(tmpbuf).c_str(), BufferHelper::getStdString(tmpbuf).c_str());
				res.insert(res.end(), tmpbuf.begin(), tmpbuf.end());
				tmpbuf.clear();

				try
				{
					buf = handleCommandBuffer(res);
					return true;
				}
				catch(CardException& e)
				{
					COM_("Exception {%s}", e.what());
					throw e;
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

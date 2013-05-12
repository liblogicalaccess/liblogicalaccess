/**
 * \file scielreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SCIEL reader/card adapter.
 */

#include "scielreadercardadapter.hpp"


namespace logicalaccess
{	
	const unsigned char SCIELReaderCardAdapter::STX = 0x5b;	// [
	const unsigned char SCIELReaderCardAdapter::ETX = 0x5d;	// ]

	SCIELReaderCardAdapter::SCIELReaderCardAdapter()
		: ReaderCardAdapter()
	{
	}

	SCIELReaderCardAdapter::~SCIELReaderCardAdapter()
	{
	}

	std::vector<unsigned char> SCIELReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		COM_("Sending command %s command size {%d} timeout {%d}...", BufferHelper::getHex(command).c_str(), command.size(), timeout);

		std::vector<unsigned char> r;
		std::vector<unsigned char> cmd;
		cmd.push_back(STX);
		cmd.insert(cmd.end(), command.begin(), command.end());
		cmd.push_back(ETX);
		d_lastCommand = cmd;
		d_trashedData.clear();
		d_lastResult.clear();

		if (command.size() > 0)
		{
			getSCIELReaderUnit()->connectToReader();

			bool success = false;
			for (int i = 0; i < 2 && !success; ++i)
			{
				boost::shared_ptr<SerialPortXml> port = getSCIELReaderUnit()->getSerialPort();
				port->getSerialPort()->write(cmd);

				for (int j = 0; j < 2 && !success; ++j)
				{
					success = receiveCommand(r, timeout);
				}
			}

			if (!success)
			{
				THROW_EXCEPTION_WITH_LOG(CardException, "No response received");
			}

			d_lastResult = r;
		}

		return r;
	}

	std::vector<unsigned char> SCIELReaderCardAdapter::handleCommandBuffer(const std::vector<unsigned char>& cmdbuf)
	{
		COM_("Processing the received command buffer %s command size {%d}...", BufferHelper::getHex(cmdbuf).c_str(), cmdbuf.size());
		std::vector<unsigned char> buf = cmdbuf;

		// Remove CR/LF (some response have it ?!)
		while (buf.size() >= 2 && buf[0] == 0x0d)
		{
			int cuti = 1;
			if (buf[1] == 0x0a)
			{
				cuti++;
			}

			buf = std::vector<unsigned char>(buf.begin() + cuti, buf.end());
		}

		EXCEPTION_ASSERT_WITH_LOG(buf.size() >= 2, LibLOGICALACCESSException, "A valid command buffer size must be at least 2 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(buf[0] == STX, LibLOGICALACCESSException, "The supplied command buffer is not valid");
		
		std::vector<unsigned char> data;

		int i = 1;
		while(buf.size() > 0 && buf[i] != ETX)
		{
			unsigned char c = buf[i++];
			data.push_back(c);
		}
		buf = std::vector<unsigned char>(buf.begin() + i, buf.end());

		EXCEPTION_ASSERT_WITH_LOG(buf.size() > 0 && buf[0] == ETX, LibLOGICALACCESSException, "Missing end of command message");

		buf = std::vector<unsigned char>(buf.begin() + 1, buf.end());
		d_trashedData = buf;

		COM_("Returning processed data %s...", BufferHelper::getHex(data).c_str());
		COM_("	-> Actual trashed data %s...", BufferHelper::getHex(d_trashedData).c_str());
		return data;
	}

	bool SCIELReaderCardAdapter::receiveCommand(std::vector<unsigned char>& buf, long int timeout)
	{
		COM_("Beginning receiving response... Timeout configured {%d}", timeout);

		std::vector<unsigned char> res;
		boost::shared_ptr<SerialPortXml> port = getSCIELReaderUnit()->getSerialPort();

		boost::posix_time::time_duration ptimeout = boost::posix_time::milliseconds(timeout);
		bool timeoutOccured = true;

		while (port->getSerialPort()->select(ptimeout))
		{
			timeoutOccured = false;
			std::vector<unsigned char> tmpbuf(256);

			while (port->getSerialPort()->read(tmpbuf, 256) > 0)
			{
				COM_("Data read on the serial port %s", BufferHelper::getHex(tmpbuf).c_str());
				res.insert(res.end(), d_trashedData.begin(), d_trashedData.end());
				d_trashedData.clear();
				res.insert(res.end(), tmpbuf.begin(), tmpbuf.end());

				try
				{
					buf = handleCommandBuffer(res);
					return true;
				}
				catch(LibLOGICALACCESSException& e)
				{
					COM_("LibLOGICALACCESSException {%s}", e.what());
					/*throw;*/
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

	std::list<std::vector<unsigned char> > SCIELReaderCardAdapter::receiveTagsListCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		COM_("Beginning receiving tag(s) identifiers whole list... Timeout configured {%d}", timeout);

		std::list<std::vector<unsigned char>> tagsList;

		if (command.size() > 0)
		{
			getSCIELReaderUnit()->connectToReader();
			boost::shared_ptr<SerialPortXml> port = getSCIELReaderUnit()->getSerialPort();

			COM_("Sending command %s command size {%d}...", BufferHelper::getHex(command).c_str(), command.size());

			std::vector<unsigned char> cmd;
			cmd.push_back(STX);
			cmd.insert(cmd.end(), command.begin(), command.end());
			cmd.push_back(ETX);

			port->getSerialPort()->write(cmd);

			boost::posix_time::time_duration ptimeout = boost::posix_time::milliseconds(timeout);
			bool timeoutOccured = true;
			bool done = false;
			d_trashedData.clear();

			while (!done && port->getSerialPort()->select(ptimeout))
			{
				COM_SIMPLE_("Responses detected on serial port!");

				timeoutOccured = false;
				std::vector<unsigned char> tmpbuf;

				while ((port->getSerialPort()->read(tmpbuf, 256) > 0) || d_trashedData.size() > 0)
				{
					COM_("Data read on the serial port %s", BufferHelper::getHex(tmpbuf).c_str());
					COM_("Trash data that was backed-up %s", BufferHelper::getHex(d_trashedData).c_str());

					std::vector<unsigned char> res;
					res.insert(res.end(), d_trashedData.begin(), d_trashedData.end());
					d_trashedData.clear();
					res.insert(res.end(), tmpbuf.begin(), tmpbuf.end());

					try
					{
						std::vector<unsigned char> buf = handleCommandBuffer(res);

						// When whole list is sent, the reader send [5b nb_tags id_reader 5d]
						if (buf.size() == 2)
						{
							COM_SIMPLE_("Whole list has been received successfully!");
							done = true;
							break;
						}
						else
						{
							tagsList.push_back(buf);
						}
					}
					catch(LibLOGICALACCESSException& e)
					{
						COM_("LibLOGICALACCESSException {%s}", e.what());
						throw;
					}
					catch (std::invalid_argument& e)
					{
						COM_("Exception {%s}", e.what());
						d_trashedData = res;
						// We break to wait for the select
						break;
					}

					tmpbuf.clear();
				}
			}

			if (timeoutOccured)
			{
				COM_SIMPLE_("Timeout reached !");
			}
			else
			{
				COM_SIMPLE_("Response command not received !");
			}
		}

		return tagsList;
	}
}

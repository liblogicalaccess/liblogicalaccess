/**
 * \file rplethdatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth data transport.
 */

#include "rplethdatatransport.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "rplethreaderunit.hpp"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ctime>

namespace logicalaccess
{
	RplethDataTransport::RplethDataTransport()
		: TcpDataTransport()
	{
		
	}

	RplethDataTransport::~RplethDataTransport()
	{

	}

	void RplethDataTransport::send(const std::vector<unsigned char>& data)
	{
		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>(Device::HID));
		cmd.push_back(static_cast<unsigned char>(HidCommand::COM));
		cmd.push_back(static_cast<unsigned char>(data.size()));
		cmd.insert (cmd.end(), data.begin(), data.end());

		sendll(cmd);
	}

	void RplethDataTransport::sendll(const std::vector<unsigned char>& data)
	{
		std::vector<unsigned char> cmd;
		cmd.insert (cmd.end(), data.begin(), data.end());
		cmd.push_back (calcChecksum(cmd));
		TcpDataTransport::send(cmd);
		d_trashedData.clear();
	}

	unsigned char RplethDataTransport::calcChecksum(const std::vector<unsigned char>& data)
	{
		unsigned char bcc = 0x00;

		for (unsigned int i = 0; i < data.size(); ++i)
		{
			bcc ^= data[i];
		}

		return bcc;
	}

	std::vector<unsigned char> RplethDataTransport::receive(long int timeout)
	{
		// Force timeout minimum
		if (timeout < 2000)
		{
			timeout = 2000;
		}
		INFO_("Receiving... %d", timeout);
		std::vector<unsigned char> buf;
		if (d_trashedData.size() > 0)
		{
			COM_("Adding existing trashed data: %s", BufferHelper::getHex(d_trashedData).c_str());
			buf = d_trashedData;
		}
	/*#ifdef _WINDOWS
		Sleep(200);
	#elif defined(__unix__)
		usleep(200000);
	#endif*/
		
		std::clock_t begin;
		std::vector<unsigned char> ansbuf;
		std::clock_t end;
		long diff = 0;
		size_t exceptedlen = 0;
		while (timeout > 0 && exceptedlen == 0)
		{
			ansbuf.clear();
			begin = std::clock();
			ansbuf = TcpDataTransport::receive(timeout);
			COM_("Answer from reader %s", BufferHelper::getHex(ansbuf).c_str());
			end = std::clock();
			diff = end - begin;
			buf.insert(buf.end(), ansbuf.begin(), ansbuf.end());
			if (buf.size() >= 4)
			{
				exceptedlen = 4 + buf[3] + 1;
				if (buf.size() < exceptedlen)
				{
					exceptedlen = 0;
				}
			}
			if (diff >= 0 && diff < timeout)
			{
				timeout -= diff;
			}
			else
			{
				timeout = 0;
			}
			//EXCEPTION_ASSERT_WITH_LOG(buf.size() >= 4, std::invalid_argument, "A valid answer buffer size must be at least 4 bytes long");
		}

		if (buf.size() > exceptedlen)
		{
			d_trashedData.clear();
			d_trashedData.insert(d_trashedData.end(), buf.begin()+exceptedlen, buf.end());

			buf.erase(buf.begin()+exceptedlen, buf.end());
			COM_("	-> Actual trashed data %s...", BufferHelper::getHex(d_trashedData).c_str());
		}
		else if (buf.size() < exceptedlen)
		{
			std::cout << "Buf size : " << buf.size()<< "exceptedlen" << exceptedlen << std::endl;
			d_trashedData.clear();
			d_trashedData.insert(d_trashedData.end(), buf.begin(), buf.end());
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The answer buffer doesn't match the excepted data length.");
		}
		else
		{
			d_trashedData.clear();
		}

		EXCEPTION_ASSERT_WITH_LOG(buf[0] != 0x01, std::invalid_argument, "The supplied answer buffer get the state : Command failure");
		EXCEPTION_ASSERT_WITH_LOG(buf[0] != 0x02, std::invalid_argument, "The supplied answer buffer get the state : Bad checksum in command");
		EXCEPTION_ASSERT_WITH_LOG(buf[0] != 0x03, LibLogicalAccessException, "The supplied answer buffer get the state : Timeout");
		EXCEPTION_ASSERT_WITH_LOG(buf[0] != 0x04, std::invalid_argument, "The supplied answer buffer get the state : Bad size of command");
		EXCEPTION_ASSERT_WITH_LOG(buf[0] != 0x05, std::invalid_argument, "The supplied answer buffer get the state : Bad device in command");
		EXCEPTION_ASSERT_WITH_LOG(buf[0] == 0x00, std::invalid_argument, "The supplied answer buffer is corrupted");

		std::vector<unsigned char> res;
		std::vector<unsigned char> bufnoc = std::vector<unsigned char>(buf.begin(), buf.end() - 1);
		unsigned char checksum_receive = buf[buf.size()-1];
		EXCEPTION_ASSERT_WITH_LOG(calcChecksum(bufnoc) == checksum_receive, std::invalid_argument, "The supplied answer buffer get the state : Bad checksum in answer");
		if (bufnoc.size() > 4)
		{
			res = std::vector<unsigned char> (bufnoc.begin() + 4, bufnoc.end());
		}

		return res;
	}

	std::string RplethDataTransport::getDefaultXmlNodeName() const
	{
		return "RplethDataTransport";
	}
}
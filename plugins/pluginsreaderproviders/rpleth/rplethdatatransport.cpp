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
		d_buffer.clear();
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

	void RplethDataTransport::sendPing()
	{
		std::vector<unsigned char> data;

		data.push_back(Device::RPLETH);
		data.push_back(0x0A);
		data.push_back(0x00);
		data.push_back(calcChecksum(data));

		TcpDataTransport::send(data);
	}

	std::vector<unsigned char> RplethDataTransport::receive(long int timeout)
	{
		std::vector<unsigned char> ret, buf;
		clock_t begin, diff;
		begin = std::clock();
		if (getSocket()->available() || d_buffer.size() < 5)
			buf = TcpDataTransport::receive(timeout);

		if (d_buffer.size() < 1024)
			d_buffer.insert(d_buffer.end(), buf.begin(), buf.end());
		if (d_buffer.size() > 4)
		{
			unsigned int exceptedlen = 4 + d_buffer[3] + 1;
			if (d_buffer.size() >= exceptedlen)
			{
				buf.clear();
				buf.insert(buf.begin(), d_buffer.begin(), d_buffer.begin() +  exceptedlen);
				d_buffer.erase(d_buffer.begin(), d_buffer.begin() +  exceptedlen);

				std::vector<unsigned char> bufnoc = std::vector<unsigned char>(buf.begin(), buf.end() - 1);
				unsigned char checksum_receive = buf[buf.size() - 1];
				EXCEPTION_ASSERT_WITH_LOG(calcChecksum(bufnoc) == checksum_receive, std::invalid_argument, "The supplied answer buffer get the state : Bad checksum in answer");

				ret.insert(ret.begin(), bufnoc.begin() + 4, bufnoc.end());
			}
		}
		diff = (begin + timeout) - std::clock();
		if (d_buffer.size() < 5 && ret.size() == 0 && diff > 0)
		{ 
			while (getSocket()->available() == 0 && diff > 0)
			{
			#ifdef _WINDOWS
				Sleep(250);
			#elif defined(__unix__)
				usleep(250000);
			#endif
				diff -= 250;
			}
		}
		return ret;
	}

	std::string RplethDataTransport::getDefaultXmlNodeName() const
	{
		return "RplethDataTransport";
	}
}
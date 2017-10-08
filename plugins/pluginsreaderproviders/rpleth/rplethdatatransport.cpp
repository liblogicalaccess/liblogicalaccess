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
#include <chrono>
#include <logicalaccess/logs.hpp>
#include "logicalaccess/settings.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    RplethDataTransport::RplethDataTransport()
        : TcpDataTransport()
    {
    }

    RplethDataTransport::~RplethDataTransport()
    {
    }

    void RplethDataTransport::send(const ByteVector& data)
    {
        ByteVector cmd;
        cmd.push_back(static_cast<unsigned char>(HID));
        cmd.push_back(static_cast<unsigned char>(COM));
        cmd.push_back(static_cast<unsigned char>(data.size()));
        cmd.insert(cmd.end(), data.begin(), data.end());

        sendll(cmd);
    }

    void RplethDataTransport::sendll(const ByteVector& data)
    {
        ByteVector cmd;
        cmd.insert(cmd.end(), data.begin(), data.end());
        cmd.push_back(calcChecksum(cmd));
        TcpDataTransport::send(cmd);
        d_buffer.clear();
    }

    unsigned char RplethDataTransport::calcChecksum(const ByteVector& data)
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
        ByteVector data;

        data.push_back(RPLETH);
        data.push_back(0x0A);
        data.push_back(0x00);
        data.push_back(calcChecksum(data));

        TcpDataTransport::send(data);
    }

    ByteVector RplethDataTransport::receive(long int timeout)
    {
        ByteVector ret, buf;
        if (timeout == -1)
            timeout = Settings::getInstance()->DataTransportTimeout;
		std::chrono::steady_clock::time_point const clock_timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);

		do
        {
            buf.clear();
            if (d_buffer.size() < 5 || (d_buffer.size() >= 4 && d_buffer.size() < (unsigned int)(4 + d_buffer[3] + 1)))
                buf = TcpDataTransport::receive(timeout);

            if (d_buffer.size() < 8192)
            {
                d_buffer.insert(d_buffer.end(), buf.begin(), buf.end());
                buf.clear();
            }
			else
			{
				d_buffer.clear();
				buf.clear();
			}

            if (d_buffer.size() >= 4)
            {
                unsigned int exceptedlen = 4 + d_buffer[3] + 1;
                if (d_buffer.size() >= exceptedlen)
                {
                    buf.clear();
                    buf.insert(buf.begin(), d_buffer.begin(), d_buffer.begin() + exceptedlen);
                    d_buffer.erase(d_buffer.begin(), d_buffer.begin() + exceptedlen);

                    EXCEPTION_ASSERT_WITH_LOG(buf[0] != 0x01, std::invalid_argument, "The supplied answer buffer get the state : Command failure");
                    EXCEPTION_ASSERT_WITH_LOG(buf[0] != 0x02, std::invalid_argument, "The supplied answer buffer get the state : Bad checksum in command");
                    EXCEPTION_ASSERT_WITH_LOG(buf[0] != 0x03, LibLogicalAccessException, "The supplied answer buffer get the state : Timeout");
                    EXCEPTION_ASSERT_WITH_LOG(buf[0] != 0x04, std::invalid_argument, "The supplied answer buffer get the state : Bad size of command");
                    EXCEPTION_ASSERT_WITH_LOG(buf[0] != 0x05, std::invalid_argument, "The supplied answer buffer get the state : Bad device in command");
                    EXCEPTION_ASSERT_WITH_LOG(buf[0] == 0x00, std::invalid_argument, "The supplied answer buffer is corrupted");

                    ByteVector bufnoc = ByteVector(buf.begin(), buf.end() - 1);
                    unsigned char checksum_receive = buf[buf.size() - 1];
                    EXCEPTION_ASSERT_WITH_LOG(calcChecksum(bufnoc) == checksum_receive, std::invalid_argument, "The supplied answer buffer get the state : Bad checksum in answer");

                    ret.insert(ret.begin(), bufnoc.begin() + 4, bufnoc.end());

                    if (ret.size() != 0 && buf[1] == HID && buf[2] == BADGE)
                    {
                        //save the badge
                        if (d_badges.size() <= 10)
                            d_badges.push_back(ret);
                    }

                    break; //We have a correct answer
                }
            }
		} while (ret.size() == 0 && std::chrono::steady_clock::now() < clock_timeout);
        return ret;
    }

    std::string RplethDataTransport::getDefaultXmlNodeName() const
    {
        return "RplethDataTransport";
    }

    ByteVector RplethDataTransport::sendCommand(const ByteVector& command, long int timeout)
    {
        LOG(LogLevel::COMS) << "Sending command " << BufferHelper::getHex(command) << " command size {" << command.size() << "} timeout {" << timeout << "}...";

        if (timeout == -1)
            timeout = Settings::getInstance()->DataTransportTimeout;

	    d_lastCommand = command;
        d_lastResult.clear();

        if (command.size() > 0)
            send(command);

        ByteVector res = receive(timeout);
        d_lastResult = res;
        LOG(LogLevel::COMS) << "Response received successfully ! Reponse: " << BufferHelper::getHex(res) << " size {" << res.size() << "}";

        return res;
    }
}
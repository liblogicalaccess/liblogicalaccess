/**
 * \file elatecreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Elatec reader/card adapter.
 */

#include "elatecreadercardadapter.hpp"

namespace logicalaccess
{
    ElatecReaderCardAdapter::ElatecReaderCardAdapter()
        : ReaderCardAdapter()
    {
        d_last_cmdcode = 0x00;
    }

    ElatecReaderCardAdapter::~ElatecReaderCardAdapter()
    {
    }

    unsigned char ElatecReaderCardAdapter::calcChecksum(const ByteVector& buf)
    {
        unsigned char checksum = 0x00;

        for (size_t i = 0; i < buf.size(); ++i)
        {
            checksum = checksum ^ buf[i];
        }

        return checksum;
    }

    ByteVector ElatecReaderCardAdapter::adaptCommand(const ByteVector& command)
    {
        EXCEPTION_ASSERT_WITH_LOG(command.size() >= 1, std::invalid_argument, "A valid command buffer size must be at least 1 byte long");

        ByteVector cmd;
        cmd.push_back(static_cast<unsigned char>(command.size() + 3));
        cmd.push_back(0xFF);	// all readers
        cmd.push_back(command[0]);
        cmd.insert(cmd.end(), command.begin() + 1, command.end());
        cmd.push_back(calcChecksum(cmd));
        d_last_cmdcode = command[0];

        return cmd;
    }

    ByteVector ElatecReaderCardAdapter::sendCommand(unsigned char cmdcode, const ByteVector& command, long int timeout)
    {
        ByteVector cmd = command;
        cmd.insert(cmd.begin(), cmdcode);
        return ReaderCardAdapter::sendCommand(cmd, timeout);
    }

    ByteVector ElatecReaderCardAdapter::adaptAnswer(const ByteVector& answer)
    {
	    EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 5, std::invalid_argument, "No valid buffer recieved.");

        unsigned char buflength = answer[0];
        EXCEPTION_ASSERT_WITH_LOG(answer[2] == d_last_cmdcode, std::invalid_argument, "The request type doesn't match the command sent by the host");

        unsigned char status = answer[3];
        // Don't throw exception for "no card" when getting card list
        if (d_last_cmdcode != 0x11 && (status != 0x01))
        {
            EXCEPTION_ASSERT_WITH_LOG(status == 0x00, std::invalid_argument, "Elatec error");
        }

        unsigned char checksum = answer[buflength - 1];
        EXCEPTION_ASSERT_WITH_LOG(calcChecksum(ByteVector(answer.begin(), answer.begin() + buflength - 1)) == checksum, std::invalid_argument, "Wrong checksum");

        ByteVector ret = ByteVector(answer.begin() + 4, answer.end() - 1);

        return ret;
    }
}
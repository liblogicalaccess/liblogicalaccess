/**
 * \file osdpreaderprovider.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief Reader Provider OSDP.
 */

#include "osdpchannel.hpp"
#include "logicalaccess/crypto/tomcrypt.h"
#include "logicalaccess/bufferhelper.hpp"
#include <openssl/rand.h>
#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
OSDPChannel::OSDPChannel()
    : isSCB(false)
    , m_address(0)
    , m_sequenceNumber(0)
    , m_data(ByteVector())
    , m_reply_type(NOCMD)
    , m_securityBlockType(NOSCS)
    , m_securityBlockData(ByteVector())
{
}

void OSDPChannel::unPackage(ByteVector result)
{
    EXCEPTION_ASSERT_WITH_LOG(result.size() >= 6, std::invalid_argument,
                              "A valid buffer size must be at least 6 bytes long");
    unsigned char index = 0;
    EXCEPTION_ASSERT_WITH_LOG(result[index] == 0x53, std::invalid_argument,
                              "Invalid SOM Received.");
    ++index;
    EXCEPTION_ASSERT_WITH_LOG(result[index] == (getAddress() | 0x80),
                              std::invalid_argument, "Invalid Address Received.");
    ++index;

    short packetLength = ((result[index + 1] << 8) + result[index]);
    EXCEPTION_ASSERT_WITH_LOG(result.size() >= static_cast<unsigned int>(packetLength),
                              std::invalid_argument,
                              "PacketLength and recieved are not the same.");

    index += 2;
    if ((result[index] & 0x04) >> 2) // isCRC
    {
        unsigned char first = 0, last = 0;
        ComputeCrcCCITT(0x1D0F, &result[0], packetLength - 2, &first, &last);
        EXCEPTION_ASSERT_WITH_LOG(result[packetLength - 2] == first &&
                                      result[packetLength - 1] == last,
                                  std::invalid_argument, "Invalid SOM Received.");
    }

    setSequenceNumber(result[index] & 0x03);
    isSCB = (result[index++] & 0x08) >> 3 ? true : false;
    if (isSCB)
    {
        unsigned char length = result[index++];
        setSecurityBlockType(static_cast<OSDPSecureChannelType>(result[index++]));

        ByteVector securityBlock(result.begin() + index,
                                 result.begin() + index + length - 2);
        setSecurityBlockData(securityBlock);
        index += static_cast<unsigned char>(securityBlock.size());
    }

    setCommandsType(static_cast<OSDPCommandsType>(result[index++]));
    char dataLength = packetLength - index - 0x02;
    if (isSCB && getSecurityBlockType() >= SCS_15 && getSecurityBlockType() <= SCS_18)
    {
        dataLength -= 0x04;
        getSecureChannel()->verifyMAC(ByteVector(result.begin(), result.end() - 2));
    }
    if (dataLength > 0)
    {
        ByteVector data(result.begin() + index, result.begin() + index + dataLength);
        if (isSCB &&
            (getSecurityBlockType() == SCS_17 || getSecurityBlockType() == SCS_18))
        {
            data = getSecureChannel()->decryptData(data, getSecureChannel()->getCMAC());
            LOG(LogLevel::INFOS) << "OSDP Answer: " << BufferHelper::getHex(data);
        }
        setData(data);
    }
    else
        setData(ByteVector());
}

ByteVector OSDPChannel::createPackage()
{
    ByteVector cmd;

    cmd.push_back(0x53); // OSDP_SOM
    cmd.push_back(getAddress());
    cmd.push_back(0x00);                             // packetLength
    cmd.push_back(0x00);                             // packetLength
    unsigned char CTRL = getSequenceNumber() | 0x04; // isCRC
    if (isSCB)
        CTRL |= 0x08;
    cmd.push_back(CTRL);

    if (isSCB)
    {
        ByteVector &secureData = getSecurityBlockData();
        cmd.push_back(static_cast<unsigned char>(secureData.size()));
        cmd.push_back(getSecurityBlockType());
        cmd.insert(cmd.end(), secureData.begin(),
                   secureData.begin() + static_cast<unsigned int>(secureData.size()) - 2);
    }

    cmd.push_back(static_cast<unsigned char>(getCommandsType()));

    ByteVector &data = getData();
    if (isSCB && (getSecurityBlockType() == SCS_17 || getSecurityBlockType() == SCS_18))
    {
        data = getSecureChannel()->encryptData(data, getSecureChannel()->getRMAC());
    }

    cmd.insert(cmd.end(), data.begin(), data.end());

    if (isSCB && getSecurityBlockType() >= SCS_15 && getSecurityBlockType() <= SCS_18)
    {
        short packetLength = static_cast<short>(cmd.size()) + 0x06;
        cmd[2]             = static_cast<unsigned char>(packetLength & 0x00ff);
        cmd[3]             = static_cast<unsigned char>(packetLength >> 8);

        ByteVector mac = getSecureChannel()->computePacketMAC(cmd);
        cmd.insert(cmd.end(), mac.begin(), mac.end());
    }
    else
    {
        short packetLength = static_cast<short>(cmd.size()) + 0x02;
        cmd[2]             = static_cast<unsigned char>(packetLength & 0x00ff);
        cmd[3]             = static_cast<unsigned char>(packetLength >> 8);
    }
    unsigned char first = 0, last = 0;
    ComputeCrcCCITT(0x1D0F, &cmd[0], cmd.size(), &first, &last);
    cmd.push_back(first);
    cmd.push_back(last);


    return cmd;
}
}

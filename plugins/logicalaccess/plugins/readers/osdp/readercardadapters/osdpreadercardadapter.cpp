/**
 * \file osdpreadercardadapter.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief OSDP reader card adapter.
 */

#include <logicalaccess/plugins/readers/osdp/readercardadapters/osdpreadercardadapter.hpp>
#include <logicalaccess/bufferhelper.hpp>

namespace logicalaccess
{
OSDPReaderCardAdapter::OSDPReaderCardAdapter(
    std::shared_ptr<OSDPCommands> command, unsigned char address,
    std::shared_ptr<DESFireISO7816ResultChecker> resultChecker)
    : ISO7816ReaderCardAdapter()
    , m_commands(command)
    , d_resultChecker(resultChecker)
    , m_address(address)
{
}


ByteVector OSDPReaderCardAdapter::sendCommand(const ByteVector &command, long /*timeout*/)
{
    ByteVector osdpCommand;
    std::shared_ptr<OSDPChannel> channel = m_commands->getChannel();
    if (channel->isSCB)
    {
        channel->setSecurityBlockData(ByteVector(2));
        channel->setSecurityBlockType(SCS_17); // Enable MAC and Data Security
    }

    channel->setCommandsType(OSDP_XWR);
    osdpCommand.push_back(0x01); // XRW_PROFILE 0x01
    osdpCommand.push_back(0x01); // XRW_PCMND 0x01
    osdpCommand.push_back(m_address);
    osdpCommand.insert(osdpCommand.end(), command.begin(), command.end());
    LOG(LogLevel::INFOS) << "OSDP Cmd: " << BufferHelper::getHex(osdpCommand);
    channel->setData(osdpCommand);
    // Transparent Content Send Request
    std::shared_ptr<OSDPChannel> result = m_commands->transmit();

    if (result->getCommandsType() == OSDP_NAK)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "NAK: Impossible to send Transparent Content");

    if (result->getCommandsType() == OSDP_ACK)
    {
        LOG(LogLevel::INFOS) << "OSDP: No XRD receive, try to send poll";
        result = m_commands->poll();
    }

    ByteVector &data = result->getData();

    if (result->getCommandsType() != OSDP_XRD ||
        (result->getCommandsType() == OSDP_XRD &&
         data[0x01] != 0x02)) // is Not APDU answer - osdp_PR01SCREP = 0x02
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Impossible to send Transparent Content");

    ByteVector res(data.begin() + 4, data.end());

    if (res.size() > 0 && d_resultChecker)
    {
        d_resultChecker->CheckResult(&res[0], res.size());
    }
    return res;
}

OSDPReaderCardAdapter::~OSDPReaderCardAdapter()
{
    // Restore Profile 0x00 command
    /*std::shared_ptr<OSDPChannel> result = m_commands->setProfile(0x00);
    if (result->getCommandsType() != OSDPCommandsType::ACK)
            LOG(LogLevel::ERRORS) << "Impossible to restore Profile 0x00";*/
}

std::shared_ptr<OSDPCommands> OSDPReaderCardAdapter::getOSDPCommands() const
{
    return m_commands;
}

unsigned char OSDPReaderCardAdapter::getRS485Address() const
{
    return m_address;
}
}
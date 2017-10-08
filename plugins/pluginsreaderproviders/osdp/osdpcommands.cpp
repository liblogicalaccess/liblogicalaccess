/**
 * \file osdpreaderprovider.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief Reader Provider OSDP.
 */

#include "osdpcommands.hpp"
#include "logicalaccess/logs.hpp"
#include "logicalaccess/crypto/tomcrypt.h"
#include <openssl/rand.h>
#include <thread>
#include <ctime>

namespace logicalaccess
{
	void OSDPCommands::initCommands(unsigned char address)
	{
		m_channel.reset(new OSDPChannel());
		m_channel->setAddress(address);
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::poll() const
	{
		m_channel->setData(ByteVector());
		m_channel->setCommandsType(POLL);

		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(ByteVector(2));
			m_channel->setSecurityBlockType(SCS_17); //Enable MAC and Data Security
		}

		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::challenge() const
	{
		m_channel->setCommandsType(CHLNG);
		m_channel->isSCB = true;

		RAND_seed(m_channel.get(), sizeof(*(m_channel.get())));
		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient entropy source");
		ByteVector rnda(8);
		if (RAND_bytes(&rnda[0], static_cast<int>(rnda.size())) != 1)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
		}

		m_channel->setSecurityBlockData(ByteVector(3));
		m_channel->setSecurityBlockType(SCS_11);

		m_channel->setData(rnda);

		transmit();

		ByteVector data = m_channel->getData();
		if (data.size() != 32)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Challenge answer is too small.");
		ByteVector uid(data.begin(), data.begin() + 8);
		ByteVector PDChallenge(data.begin() + 8, data.begin() + 16);
		ByteVector PDCryptogram(data.begin() + 16, data.begin() + 32);
		std::shared_ptr<OSDPSecureChannel> mySecureChannel(new OSDPSecureChannel(uid, PDChallenge, PDCryptogram, rnda));
		m_channel->setSecureChannel(mySecureChannel);

		if (m_channel->getSecurityBlockData()[0] == 0x00)
			m_channel->getSecureChannel()->isSCBK_D = true;
		else
			m_channel->getSecureChannel()->isSCBK_D = false;

		return m_channel;
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::sCrypt() const
	{
		m_channel->setCommandsType(OSCRYPT);
		m_channel->setData(m_channel->getSecureChannel()->getCPCryptogram());

		m_channel->setSecurityBlockType(SCS_13);

		ByteVector blockData(3);
		if (m_channel->getSecureChannel()->isSCBK_D)
			blockData[0] = 0x00;
		else
			blockData[0] = 0x01;
		m_channel->setSecurityBlockData(blockData);

		transmit();

		ByteVector data = m_channel->getData();
		if (data.size() != 16)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "RMAC is too small.");
		m_channel->getSecureChannel()->setRMAC(data);

		return m_channel;
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::led(s_led_cmd& led) const
	{
		ByteVector ledConfig(14);

		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(ByteVector(2));
			m_channel->setSecurityBlockType(SCS_17); //Enable MAC and Data Security
		}

		m_channel->setCommandsType(LED);
		memcpy(&ledConfig[0], &led, sizeof(s_led_cmd));
		m_channel->setData(ledConfig);
		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::buz(s_buz_cmd& led) const
	{
		ByteVector buzConfig(14);

		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(ByteVector(2));
			m_channel->setSecurityBlockType(SCS_17); //Enable MAC and Data Security
		}

		m_channel->setCommandsType(BUZ);
		memcpy(&buzConfig[0], &led, sizeof(s_buz_cmd));
		m_channel->setData(buzConfig);
		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::getProfile() const
	{
		ByteVector osdpCommand;
		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(ByteVector(2));
			m_channel->setSecurityBlockType(SCS_17); //Enable MAC and Data Security
		}

		m_channel->setCommandsType(XWR);
		osdpCommand.push_back(0x00); //XRW_PROFILE 0x00
		osdpCommand.push_back(0x01); //XRW_PCMND 0x01 Get Profile Setting
		m_channel->setData(osdpCommand);
		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::setProfile(unsigned char profile) const
	{
		ByteVector osdpCommand;
		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(ByteVector(2));
			m_channel->setSecurityBlockType(SCS_17); //Enable MAC and Data Security
		}

		m_channel->setCommandsType(XWR);
		osdpCommand.push_back(0x00); //XRW_PROFILE 0x00
		osdpCommand.push_back(0x02); //XRW_PCMND 0x01 Set Profile Setting
		osdpCommand.push_back(profile); // Set profile
		m_channel->setData(osdpCommand);
		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::disconnectFromSmartcard() const
	{
		ByteVector osdpCommand;
		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(ByteVector(2));
			m_channel->setSecurityBlockType(SCS_17); //Enable MAC and Data Security
		}

		m_channel->setCommandsType(XWR);
		osdpCommand.push_back(0x01); //XRW_PROFILE 0x01
		osdpCommand.push_back(0x02); //XRW_PCMND 0x02 Connection Done
		osdpCommand.push_back(m_channel->getAddress()); // Set profile
		m_channel->setData(osdpCommand);
		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::transmit() const
	{
		const clock_t begin_time = clock();

		do
		{
			ByteVector result = getReaderCardAdapter()->sendCommand(m_channel->createPackage());
			m_channel->unPackage(result);
			if (m_channel->getCommandsType() == BUSY)
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		while (m_channel->getCommandsType() == BUSY && (clock () - begin_time) /  CLOCKS_PER_SEC < 2); // 3sec

		m_channel->setSequenceNumber(m_channel->getSequenceNumber() + 1);
		if (m_channel->getSequenceNumber() > 3) 
			m_channel->setSequenceNumber(1);
		return m_channel;
	}
}

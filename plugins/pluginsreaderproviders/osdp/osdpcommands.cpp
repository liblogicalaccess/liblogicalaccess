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

	std::shared_ptr<OSDPChannel> OSDPCommands::poll()
	{
		m_channel->setData(std::vector<unsigned char>());
		m_channel->setCommandsType(OSDPCommandsType::POLL);

		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(std::vector<unsigned char>(2));
			m_channel->setSecurityBlockType(OSDPSecureChannelType::SCS_17); //Enable MAC and Data Security
		}

		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::challenge()
	{
		m_channel->setCommandsType(OSDPCommandsType::CHLNG);
		m_channel->isSCB = true;

		RAND_seed(m_channel.get(), sizeof(*(m_channel.get())));
		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient entropy source");
		std::vector<unsigned char> rnda(8);
		if (RAND_bytes(&rnda[0], static_cast<int>(rnda.size())) != 1)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
		}

		m_channel->setSecurityBlockData(std::vector<unsigned char>(3));
		m_channel->setSecurityBlockType(OSDPSecureChannelType::SCS_11);

		m_channel->setData(rnda);

		transmit();

		std::vector<unsigned char> data = m_channel->getData();
		if (data.size() != 32)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Challenge answer is too small.");
		std::vector<unsigned char> uid(data.begin(), data.begin() + 8);
		std::vector<unsigned char> PDChallenge(data.begin() + 8, data.begin() + 16);
		std::vector<unsigned char> PDCryptogram(data.begin() + 16, data.begin() + 32);
		std::shared_ptr<OSDPSecureChannel> mySecureChannel(new OSDPSecureChannel(uid, PDChallenge, PDCryptogram, rnda));
		m_channel->setSecureChannel(mySecureChannel);

		if (m_channel->getSecurityBlockData()[0] == 0x00)
			m_channel->getSecureChannel()->isSCBK_D = true;
		else
			m_channel->getSecureChannel()->isSCBK_D = false;

		return m_channel;
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::sCrypt()
	{
		m_channel->setCommandsType(OSDPCommandsType::OSCRYPT);
		m_channel->setData(m_channel->getSecureChannel()->getCPCryptogram());

		m_channel->setSecurityBlockType(OSDPSecureChannelType::SCS_13);

		std::vector<unsigned char> blockData(3);
		if (m_channel->getSecureChannel()->isSCBK_D)
			blockData[0] = 0x00;
		else
			blockData[0] = 0x01;
		m_channel->setSecurityBlockData(blockData);

		transmit();

		std::vector<unsigned char> data = m_channel->getData();
		if (data.size() != 16)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "RMAC is too small.");
		m_channel->getSecureChannel()->setRMAC(data);

		return m_channel;
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::led(s_led_cmd& led)
	{
		std::vector<unsigned char> ledConfig(14);

		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(std::vector<unsigned char>(2));
			m_channel->setSecurityBlockType(OSDPSecureChannelType::SCS_17); //Enable MAC and Data Security
		}

		m_channel->setCommandsType(OSDPCommandsType::LED);
		memcpy(&ledConfig[0], &led, sizeof(s_led_cmd));
		m_channel->setData(ledConfig);
		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::buz(s_buz_cmd& led)
	{
		std::vector<unsigned char> buzConfig(14);

		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(std::vector<unsigned char>(2));
			m_channel->setSecurityBlockType(OSDPSecureChannelType::SCS_17); //Enable MAC and Data Security
		}

		m_channel->setCommandsType(OSDPCommandsType::BUZ);
		memcpy(&buzConfig[0], &led, sizeof(s_buz_cmd));
		m_channel->setData(buzConfig);
		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::getProfile()
	{
		std::vector<unsigned char> osdpCommand;
		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(std::vector<unsigned char>(2));
			m_channel->setSecurityBlockType(OSDPSecureChannelType::SCS_17); //Enable MAC and Data Security
		}

		m_channel->setCommandsType(OSDPCommandsType::XWR);
		osdpCommand.push_back(0x00); //XRW_PROFILE 0x00
		osdpCommand.push_back(0x01); //XRW_PCMND 0x01 Get Profile Setting
		m_channel->setData(osdpCommand);
		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::setProfile(unsigned char profile)
	{
		std::vector<unsigned char> osdpCommand;
		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(std::vector<unsigned char>(2));
			m_channel->setSecurityBlockType(OSDPSecureChannelType::SCS_17); //Enable MAC and Data Security
		}

		m_channel->setCommandsType(OSDPCommandsType::XWR);
		osdpCommand.push_back(0x00); //XRW_PROFILE 0x00
		osdpCommand.push_back(0x02); //XRW_PCMND 0x01 Set Profile Setting
		osdpCommand.push_back(profile); // Set profile
		m_channel->setData(osdpCommand);
		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::disconnectFromSmartcard()
	{
		std::vector<unsigned char> osdpCommand;
		if (m_channel->isSCB)
		{
			m_channel->setSecurityBlockData(std::vector<unsigned char>(2));
			m_channel->setSecurityBlockType(OSDPSecureChannelType::SCS_17); //Enable MAC and Data Security
		}

		m_channel->setCommandsType(OSDPCommandsType::XWR);
		osdpCommand.push_back(0x01); //XRW_PROFILE 0x01
		osdpCommand.push_back(0x02); //XRW_PCMND 0x02 Connection Done
		osdpCommand.push_back(m_channel->getAddress()); // Set profile
		m_channel->setData(osdpCommand);
		return transmit();
	}

	std::shared_ptr<OSDPChannel> OSDPCommands::transmit()
	{
		const clock_t begin_time = std::clock();

		do
		{
			std::vector<unsigned char> result = getReaderCardAdapter()->sendCommand(m_channel->createPackage());
			m_channel->unPackage(result);
			if (m_channel->getCommandsType() == OSDPCommandsType::BUSY)
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		while (m_channel->getCommandsType() == OSDPCommandsType::BUSY && (std::clock () - begin_time) /  CLOCKS_PER_SEC < 2); // 3sec

		m_channel->setSequenceNumber(m_channel->getSequenceNumber() + 1);
		if (m_channel->getSequenceNumber() > 3) 
			m_channel->setSequenceNumber(1);
		return m_channel;
	}
}

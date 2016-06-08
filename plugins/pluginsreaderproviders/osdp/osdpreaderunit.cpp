/**
 * \file stidstrreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR reader unit.
 */

#include "osdpreaderunit.hpp"
#include "osdpreaderprovider.hpp"


#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"

#include <boost/filesystem.hpp>
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "readercardadapters/osdpdatatransport.hpp"
#include "commands/desfireev1iso7816commands.hpp"
#include "readercardadapters/osdpreadercardadapter.hpp"
#include "desfireprofile.hpp"
#include "commands/desfireiso7816resultchecker.hpp"

#include "osdpcommands.hpp"

namespace logicalaccess
{
	OSDPReaderUnit::OSDPReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new OSDPReaderUnitConfiguration());

		setDataTransport(std::shared_ptr<OSDPDataTransport>(new OSDPDataTransport()));

		std::shared_ptr<ReaderCardAdapter> rca(new ReaderCardAdapter());
		rca->setDataTransport(getDataTransport());

		m_commands.reset(new OSDPCommands());
		m_commands->setReaderCardAdapter(rca);

		d_card_type = CHIP_UNKNOWN;

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/OSDPReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
		}
		catch (...) { }
	}

	OSDPReaderUnit::~OSDPReaderUnit()
	{
		disconnectFromReader();
	}

	std::string OSDPReaderUnit::getName() const
	{
		return getDataTransport()->getName();
	}

	std::string OSDPReaderUnit::getConnectedName()
	{
		return getName();
	}

	void OSDPReaderUnit::setCardType(std::string cardType)
	{
		LOG(LogLevel::INFOS) << "Setting card type {" << cardType << "}";
		d_card_type = cardType;
	}

	std::shared_ptr<Chip> OSDPReaderUnit::createChip(std::string type)
	{
		std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);
		setcryptocontext setcryptocontextfct;

		if (chip)
		{
			LOG(LogLevel::INFOS) << "Chip created successfully !";
			std::shared_ptr<ReaderCardAdapter> rca;
			std::shared_ptr<Commands> commands;

			if (type == "DESFire" || type == "DESFireEV1")
			{
				LOG(LogLevel::INFOS) << "Mifare DESFire EV1 Chip created";
				rca.reset(new OSDPReaderCardAdapter(m_commands, getOSDPConfiguration()->getRS485Address(), std::shared_ptr<DESFireISO7816ResultChecker>(new DESFireISO7816ResultChecker())));
				commands = LibraryManager::getInstance()->getCommands("DESFireEV1ISO7816");
				*(void**)(&setcryptocontextfct) = LibraryManager::getInstance()->getFctFromName("setCryptoContextDESFireEV1ISO7816Commands", LibraryManager::READERS_TYPE);
				setcryptocontextfct(&commands, &chip);
			}
			else
				return chip;

			rca->setDataTransport(getDataTransport());
			if (commands)
			{
				commands->setReaderCardAdapter(rca);
				commands->setChip(chip);
				chip->setCommands(commands);
			}
		}
		return chip;
	}

	bool OSDPReaderUnit::waitInsertion(unsigned int maxwait)
	{
		unsigned int currentWait = 0;
		bool inserted = false;

		if (m_currencard.size() > 0)
		{
			inserted = true;
		}
		else
		{
			do
			{
				std::shared_ptr<OSDPChannel> poll = m_commands->poll();

				if (poll->getCommandsType() == OSDPCommandsType::RAW)
				{
					m_currencard = poll->getData();
					LOG(LogLevel::INFOS) << "ATQA card type detected:" << BufferHelper::getHex(m_currencard) << "}";

					inserted = true;
				}
				else
				{
					if (!inserted)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
						currentWait += 100;
					}
				}
			} while (!inserted && (maxwait == 0 || currentWait < maxwait));
		}

		if (inserted)
		{
			s_led_cmd	osdp_LED_cmd = { 0,0,TemporaryControleCode::SetTemporaryState,5,5, OSDPColor::Green, OSDPColor::Black,30,0,
				PermanentControlCode::SetPermanentState,1,0,OSDPColor::Red,OSDPColor::Black };
			m_commands->led(osdp_LED_cmd);

			s_buz_cmd	osdp_BUZ_cmd = { 0,2,1,1,3 };
			m_commands->buz(osdp_BUZ_cmd);

			d_insertedChip = createChip(d_card_type);
		}

		return inserted;
	}

	bool OSDPReaderUnit::waitRemoval(unsigned int maxwait)
	{
		unsigned int currentWait = 0;
		bool removed = false;

		do
		{
			std::shared_ptr<OSDPChannel> poll = m_commands->poll();

			if (poll->getCommandsType() == OSDPCommandsType::RAW)
			{
				std::vector<unsigned char> currentcard = poll->getData();
				LOG(LogLevel::INFOS) << "ATQA card type detected:" << BufferHelper::getHex(currentcard) << "}";

				if (currentcard == m_currencard)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					currentWait += 100;
				}
				else
					removed = true;
			}
			/*else
				removed = true;*/

			std::cout << poll->getCommandsType();
		} while (!removed && (maxwait == 0 || currentWait < maxwait));

		if (d_insertedChip && d_insertedChip->getCommands())
		{
			d_insertedChip->getCommands()->setReaderCardAdapter(std::shared_ptr<ReaderCardAdapter>());
		}
		return removed;
	}

	bool OSDPReaderUnit::connect()
	{
		//TODO
		return true;
	}

	void OSDPReaderUnit::disconnect()
	{
		std::shared_ptr<OSDPChannel> result = m_commands->disconnectFromSmartcard();
	}

	void OSDPReaderUnit::checkPDAuthentication(std::shared_ptr<OSDPChannel> challenge)
	{
		std::vector<unsigned char> cryptogramInput;

		std::shared_ptr<OSDPReaderUnitConfiguration> config = getOSDPConfiguration();
		challenge->getSecureChannel()->deriveKey(config->getSCBKKey(), config->getSCBKDKey());
		challenge->getSecureChannel()->computeAuthenticationData();
	}

	bool OSDPReaderUnit::connectToReader()
	{
		bool ret = getDataTransport()->connect();
		if (ret)
		{
			m_commands->initCommands(getOSDPConfiguration()->getRS485Address());

			//Test if can read
			m_commands->poll();
			
			//Start Secure Channel
			std::shared_ptr<OSDPChannel> challenge = m_commands->challenge();
			checkPDAuthentication(challenge);
			std::shared_ptr<OSDPChannel> crypt = m_commands->sCrypt();
			//Successful Authentication
			crypt->isSCB = true;

			std::shared_ptr<OSDPChannel> result = m_commands->getProfile();
			if (result->getCommandsType() == OSDPCommandsType::XRD)
			{
				result = m_commands->setProfile(0x00);

				if (result->getCommandsType() != OSDPCommandsType::ACK)
					THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Impossible to set Profile 0x00");
			}
		}

		return ret;
	}

	void OSDPReaderUnit::disconnectFromReader()
	{
		getDataTransport()->disconnect();
	}

	std::shared_ptr<Chip> OSDPReaderUnit::getSingleChip()
	{
		std::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<std::shared_ptr<Chip> > OSDPReaderUnit::getChipList()
	{
		std::vector<std::shared_ptr<Chip> > chipList;
		std::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	std::string OSDPReaderUnit::getReaderSerialNumber()
	{
		return std::string();
	}

	bool OSDPReaderUnit::isConnected()
	{
		return true;
	}
	
	void OSDPReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		ReaderUnit::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void OSDPReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		ReaderUnit::unSerialize(node);
	}
}

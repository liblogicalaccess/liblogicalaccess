/**
 * \file rwk400readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rwk400 reader unit.
 */

#include "rwk400readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"

#include "logicalaccess/bufferhelper.hpp"
#include "rwk400readerprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/rwk400readercardadapter.hpp"
#include "commands/desfirerwk400commands.hpp"
#include <boost/filesystem.hpp>
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

namespace logicalaccess
{

	Rwk400ReaderUnit::Rwk400ReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new Rwk400ReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<Rwk400ReaderCardAdapter> (new Rwk400ReaderCardAdapter()));
		boost::shared_ptr<SerialPortDataTransport> dataTransport(new SerialPortDataTransport());
		setDataTransport(dataTransport);
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/Rwk400ReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	Rwk400ReaderUnit::~Rwk400ReaderUnit()
	{
		disconnectFromReader();
	}

	std::string Rwk400ReaderUnit::getName() const
	{
		return getDataTransport()->getName();
	}

	std::string Rwk400ReaderUnit::getConnectedName()
	{
		return getName();
	}

	void Rwk400ReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
		d_readerCommunication = getReaderCommunication(cardType);
		boost::dynamic_pointer_cast<ReaderCardAdapter>(d_readerCommunication)->setDataTransport(getDataTransport());
	}

	boost::shared_ptr<ReaderCommunication> Rwk400ReaderUnit::getReaderCommunication(std::string /*cardType*/)
	{
		boost::shared_ptr<ReaderCommunication> ret;

		ret.reset(new Rwk400ReaderCardAdapter());
		return ret;
	}

	bool Rwk400ReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool inserted = false;
		boost::shared_ptr<Chip> chip = getChipInAir(maxwait);
		if (chip)
		{
			d_insertedChip = chip;
			inserted = true;
		}

		return inserted;
	}

	bool Rwk400ReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;
		if (d_insertedChip)
		{
			while (!removed && ((currentWait < maxwait) || maxwait == 0))
			{
				boost::shared_ptr<Chip> chip = getChipInAir(250);
				if (chip)
				{
					if (chip->getChipIdentifier() != d_insertedChip->getChipIdentifier())
					{
						d_insertedChip.reset();
						removed = true;
					}
					else
					{
		#ifdef _WINDOWS
						Sleep(100);
		#elif defined(__unix__)
						usleep(100000);
		#endif
						currentWait += 250;
					}

				}
				else
				{
					d_insertedChip.reset();
					removed = true;
				}
			}
		}
		return removed;
	}

	bool Rwk400ReaderUnit::connect()
	{
		COM_SIMPLE_("connect to the chip");
		bool ret = true;
		return ret;
	}

	void Rwk400ReaderUnit::disconnect()
	{
	}

	std::vector<unsigned char> Rwk400ReaderUnit::getCardSerialNumber()
	{
		std::vector<unsigned char> csn = selectCard ();
		if (csn.size () > 0)
			csn.erase (csn.begin());
		return csn;
	}

	boost::shared_ptr<Chip> Rwk400ReaderUnit::createChip(std::string type)
	{
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<Commands> commands;

			if (type == "DESFire")
			{
				rca = getDefaultRwk400ReaderCardAdapter();
				commands.reset(new DesfireRwk400Commands());
				boost::dynamic_pointer_cast<DesfireRwk400Commands>(commands)->getCrypto().setCryptoContext(boost::dynamic_pointer_cast<DESFireProfile>(chip->getProfile()), chip->getChipIdentifier());
			}
			else if (type == "GenericTag")
			{
				rca = getDefaultRwk400ReaderCardAdapter();
			}

			if (rca)
			{
				rca->setDataTransport(getDataTransport());
				if (commands)
				{
					commands->setReaderCardAdapter(rca);
					commands->setChip(chip);
					chip->setCommands(commands);
				}
			}
		}
		return chip;
	}

	boost::shared_ptr<Chip> Rwk400ReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > Rwk400ReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<Rwk400ReaderCardAdapter> Rwk400ReaderUnit::getDefaultRwk400ReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		if (!adapter->getDataTransport())
			adapter->setDataTransport(getDataTransport());
		return boost::dynamic_pointer_cast<Rwk400ReaderCardAdapter>(adapter);
	}

	bool Rwk400ReaderUnit::isConnected()
	{
		return bool(d_insertedChip);
	}

	bool Rwk400ReaderUnit::connectToReader()
	{
		boost::shared_ptr<DataTransport> dataTransport = getDataTransport();
		if (!dataTransport->getReaderUnit())
		{
			dataTransport->setReaderUnit(shared_from_this());
			setDataTransport(dataTransport);
		}
		return getDataTransport()->connect();
	}

	void Rwk400ReaderUnit::disconnectFromReader()
	{
		getDataTransport()->disconnect();
	}

	std::vector<unsigned char> Rwk400ReaderUnit::getPingCommand() const
	{
		std::vector<unsigned char> cmd;

		cmd.push_back(0x80);							// CLA
		cmd.push_back(RWK400Commands::READ_EEPROM);		// INS
		cmd.push_back(0x00);							// P1
		cmd.push_back(0x00);							// P2
		cmd.push_back(0x01);							// P3

		return cmd;
	}

	std::string Rwk400ReaderUnit::getReaderSerialNumber()
	{
		return "";
	}

	void Rwk400ReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		ReaderUnit::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void Rwk400ReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		ReaderUnit::unSerialize(node);
	}

	boost::shared_ptr<Rwk400ReaderProvider> Rwk400ReaderUnit::getRwk400ReaderProvider() const
	{
		return boost::dynamic_pointer_cast<Rwk400ReaderProvider>(getReaderProvider());
	}

	void Rwk400ReaderUnit::resetRF(int /*offtime*/)
	{
		std::vector<unsigned char> command;
		command.push_back(0x80);							// CLA
		command.push_back(RWK400Commands::RESET_RF);		// INS
		command.push_back(0x40);							// P1
		command.push_back(0x00);							// P2
		command.push_back(0x00);							// P3
		try
		{
			getDefaultRwk400ReaderCardAdapter()->sendCommand(command);
		}
		catch(std::exception&)
		{

		}
	}

	std::vector<unsigned char> Rwk400ReaderUnit::selectCard()
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(0x80);							// CLA
		command.push_back(RWK400Commands::SELECT_CARD);		// INS
		command.push_back(0x00);							// P1
		command.push_back(0x0A);							// P2
		command.push_back(0x09);							// P3
		try
		{
			answer = getDefaultRwk400ReaderCardAdapter()->sendCommand(command);
		}
		catch(std::exception&)
		{

		}
		return answer;
	}

	boost::shared_ptr<Chip> Rwk400ReaderUnit::getChipInAir(unsigned int maxwait)
	{
		INFO_SIMPLE_("Starting get chip in air...");

		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> buf;
		unsigned int currentWait = 0;
		while (!chip && (maxwait == 0 || currentWait < maxwait))
		{
			buf = selectCard ();
			COM_("Select Card : %s",BufferHelper::getHex (buf).c_str());
			if (buf.size () > 1)
			{
				if (buf[0] == RWK400_MAFARE_DESFIRE_CHIP)
				{
					if (rats ())
					{
						chip = createChip ("DESFire");
						buf.erase (buf.begin());
						chip->setChipIdentifier(buf);
					}
				}
			}
			if (!chip)
			{
	#ifdef _WINDOWS
				Sleep(250);
	#elif defined(__unix__)
				usleep(250000);
	#endif
				currentWait += 250;
			}
		}
		return chip;
	}

	bool Rwk400ReaderUnit::rats()
	{
		bool res = true;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back (0x80);						// CLA
		command.push_back (RWK400Commands::TRANSMIT);	// INS
		command.push_back (0xF7);						// P1
		command.push_back (0x06);						// P2
		command.push_back (0x02);						// P3
		command.push_back (0xE0);						// RATS
		command.push_back (0x80);						// P1
		try
		{
			answer = getDefaultRwk400ReaderCardAdapter()->sendCommand(command);
		}
		catch(std::exception&)
		{
			res = false;
		}
		return res;
	}
}

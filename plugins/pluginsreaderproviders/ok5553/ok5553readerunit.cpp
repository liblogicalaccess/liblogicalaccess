/**
 * \file ok5553readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief OK5553 reader unit.
 */

#include "ok5553readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"

#include "logicalaccess/bufferhelper.hpp"
#include "ok5553readerprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/ok5553readercardadapter.hpp"
#include "readercardadapters/iso7816ok5553readercardadapter.hpp"
#include "commands/desfireev1iso7816commands.hpp"
#include "commands/mifareok5553commands.hpp"
#include "commands/mifareultralightok5553commands.hpp"
#include <boost/filesystem.hpp>
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

namespace logicalaccess
{
	OK5553ReaderUnit::OK5553ReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new OK5553ReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<OK5553ReaderCardAdapter> (new OK5553ReaderCardAdapter()));
		
		boost::shared_ptr<SerialPortDataTransport> dataTransport(new SerialPortDataTransport());
		setDataTransport(dataTransport);
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/OK5553ReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	OK5553ReaderUnit::~OK5553ReaderUnit()
	{
		disconnectFromReader();
	}

	std::string OK5553ReaderUnit::getName() const
	{
		return getDataTransport()->getName();
	}

	std::string OK5553ReaderUnit::getConnectedName()
	{
		return getName();
	}

	void OK5553ReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
	}

	bool OK5553ReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool inserted = false;
		if (removalIdentifier.size() > 0)
		{
			d_insertedChip = createChip((d_card_type == "UNKNOWN") ? "GenericTag" : d_card_type);
			d_insertedChip->setChipIdentifier(removalIdentifier);
			removalIdentifier.clear();
			inserted = true;
		}
		else
		{
			boost::shared_ptr<Chip> chip = getChipInAir(maxwait);
			if (chip)
			{
				d_insertedChip = chip;
				inserted = true;
			}
		}

		return inserted;
	}

	bool OK5553ReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;
		removalIdentifier.clear();
		if (d_insertedChip)
		{
			while (!removed && ((currentWait < maxwait) || maxwait == 0))
			{
				boost::shared_ptr<Chip> chip = getChipInAir(250);
				if (chip)
				{
					std::vector<unsigned char> tmpId = chip->getChipIdentifier();
					if (tmpId != d_insertedChip->getChipIdentifier())
					{
						d_insertedChip.reset();
						removalIdentifier = tmpId;
						removed = true;
					}
					else
					{
		#ifdef _WINDOWS
						Sleep(100);
		#elif defined(LINUX)
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

	bool OK5553ReaderUnit::connect()
	{
		INFO_SIMPLE_("Starting connect...");
		if (getSingleChip())
		{
			if (getSingleChip()->getCardType() == "DESFire")
			{
				std::vector<unsigned char> tmp = rats();
				if (tmp.size() == 0)
				{
					d_insertedChip = getChipInAir();
					tmp = rats();
					if (tmp.size() == 0)
					{
						d_insertedChip.reset();
					}
				}
			}
		}
		
		return bool(d_insertedChip);
	}

	void OK5553ReaderUnit::disconnect()
	{
		/*
		if (getRplethConfiguration()->getMode() == RplethMode::RS)
		{
			getDefaultRplethReaderCardAdapter()->sendAsciiCommand ("x");
			getDefaultRplethReaderCardAdapter()->sendAsciiCommand ("v");
		}*/

		INFO_SIMPLE_("Disconnected from the chip");
	}

	std::vector<unsigned char> OK5553ReaderUnit::asciiToHex (const std::vector<unsigned char>& source)
	{
		std::vector<unsigned char> res;
		char tmp [3];
		if (source.size() > 1)
		{
			for (size_t i = 0; i <= source.size() - 2; i+=2)
			{
				tmp [0] = source[i];
				tmp [1] = source[i+1];
				tmp [2] = '\0';
				res.push_back (static_cast<unsigned char>(strtoul (tmp, NULL, 16)));
			}
		}
		else
		{
			res = source;
		}
		return res;
	}

	boost::shared_ptr<Chip> OK5553ReaderUnit::getChipInAir(unsigned int maxwait)
	{
		INFO_SIMPLE_("Starting get chip in air...");

		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> buf;
		unsigned int currentWait = 0;
		while (!chip && (maxwait == 0 || currentWait < maxwait))
		{
			try
			{
				buf = getDefaultOK5553ReaderCardAdapter()->sendAsciiCommand ("s");
			}
			catch (std::exception&)
			{
				buf.clear();
			}
			d_successedRATS.clear();
			if (buf.size () > 0)
			{
				buf = asciiToHex (buf);
				if (buf[0] == ChipType::MIFARE)
				{
					chip = createChip ("Mifare");
					buf.erase (buf.begin());
					chip->setChipIdentifier(buf);
				}
				else if (buf[0] == ChipType::DESFIRE)
				{
					chip = createChip ("DESFire");
					buf.erase (buf.begin());
					chip->setChipIdentifier(buf);					
					boost::dynamic_pointer_cast<DESFireISO7816Commands>(chip->getCommands())->getCrypto()->setCryptoContext(boost::dynamic_pointer_cast<DESFireProfile>(chip->getProfile()), chip->getChipIdentifier());
				}
				else if (buf[0] == ChipType::MIFAREULTRALIGHT)
				{
					chip = createChip ("MifareUltralight");
					buf.erase (buf.begin());
					chip->setChipIdentifier(buf);
				}
			}
			if (!chip)
			{
	#ifdef _WINDOWS
				Sleep(250);
	#elif defined(LINUX)
				usleep(250000);
	#endif
				currentWait += 250;
			}
		}

		return chip;
	}
	
	boost::shared_ptr<Chip> OK5553ReaderUnit::createChip(std::string type)
	{
		INFO_("Create chip %s", type.c_str());
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<Commands> commands;

			if (type ==  "Mifare1K" || type == "Mifare4K" || type == "Mifare")
			{
				commands.reset(new MifareOK5553Commands);
				rca = getDefaultReaderCardAdapter();
			}
			else if (type ==  "MifareUltralight")
			{
				commands.reset(new MifareUltralightOK5553Commands());
				rca = getDefaultReaderCardAdapter();
			}
			else if (type == "GenericTag")
			{
				rca = getDefaultReaderCardAdapter();
			}
			else if (type == "DESFire")
			{
				rca.reset(new ISO7816OK5553ReaderCardAdapter());
				commands.reset(new DESFireISO7816Commands());
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

	boost::shared_ptr<Chip> OK5553ReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > OK5553ReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<OK5553ReaderCardAdapter> OK5553ReaderUnit::getDefaultOK5553ReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		if (!adapter->getDataTransport())
			adapter->setDataTransport(getDataTransport());
		return boost::dynamic_pointer_cast<OK5553ReaderCardAdapter>(adapter);
	}

	string OK5553ReaderUnit::getReaderSerialNumber()
	{
		return "";
	}

	bool OK5553ReaderUnit::isConnected()
	{
		return bool(d_insertedChip);
	}

	bool OK5553ReaderUnit::connectToReader()
	{
		INFO_SIMPLE_("Starting connection to reader...");
		boost::shared_ptr<DataTransport> dataTransport = getDataTransport();
		if (!dataTransport->getReaderUnit())
		{
			dataTransport->setReaderUnit(shared_from_this());
			setDataTransport(dataTransport);
		}
		
		return getDataTransport()->connect();
	}

	void OK5553ReaderUnit::disconnectFromReader()
	{
		INFO_SIMPLE_("Starting disconnection to reader...");
		getDataTransport()->disconnect();
	}

	void OK5553ReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		ReaderUnit::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void OK5553ReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		ReaderUnit::unSerialize(node);
	}

	boost::shared_ptr<OK5553ReaderProvider> OK5553ReaderUnit::getOK5553ReaderProvider() const
	{
		return boost::dynamic_pointer_cast<OK5553ReaderProvider>(getReaderProvider());
	}

	std::vector<unsigned char> OK5553ReaderUnit::reqA ()
	{
		std::vector<unsigned char> answer = getDefaultOK5553ReaderCardAdapter()->sendAsciiCommand ("t01E326");
		answer = asciiToHex (answer);
		if (answer.size () > 1)
		{
			if (answer[0] == answer.size()-1)
			{
				answer.erase (answer.begin());
			}
			else
				answer.clear();
		}
		else
		{
			answer.clear();
		}
		return answer;
	}

	std::vector<unsigned char> OK5553ReaderUnit::rats ()
	{
		std::vector<unsigned char> answer;

		// Sending two RATS is not supported without a new Select. Doesn't send another one if the first successed.
		if (d_successedRATS.size() == 0)
		{
			INFO_("Sending a RATS");
			answer = getDefaultOK5553ReaderCardAdapter()->sendAsciiCommand ("t020FE020");
			answer = asciiToHex (answer);
			if (answer.size () > 1)
			{
				if (answer[0] == answer.size()-1)
				{
					answer.erase (answer.begin());
				}
				else
					answer.clear();
			}
			else
			{
				answer.clear();
				THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "No tag present in rfid field");
			}

			d_successedRATS = answer;
		}
		else
		{
			answer = d_successedRATS;
		}

		return answer;
	}
}

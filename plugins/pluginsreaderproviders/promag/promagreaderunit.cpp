/**
 * \file promagreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Promag reader unit.
 */

#include "promagreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "promagreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/promagreadercardadapter.hpp"
#include <boost/filesystem.hpp>
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

namespace logicalaccess
{
	PromagReaderUnit::PromagReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new PromagReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<PromagReaderCardAdapter> (new PromagReaderCardAdapter()));
		boost::shared_ptr<SerialPortDataTransport> dataTransport(new SerialPortDataTransport());
		setDataTransport(dataTransport);
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/PromagReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	PromagReaderUnit::~PromagReaderUnit()
	{
		disconnectFromReader();
	}

	std::string PromagReaderUnit::getName() const
	{
		return getDataTransport()->getName();
	}

	std::string PromagReaderUnit::getConnectedName()
	{
		return getName();
	}

	void PromagReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
	}

	bool PromagReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool inserted = false;
		unsigned int currentWait = 0;

		if(d_promagIdentifier.size() == 0)
		{
			retrieveReaderIdentifier();
		}

		do
		{
			boost::shared_ptr<Chip> chip = getChipInAir();
			if (chip)
			{
				d_insertedChip = chip;
				inserted = true;
			}

			if (!inserted)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				currentWait += 100;
			}
		} while (!inserted && (maxwait == 0 || currentWait < maxwait));

		return inserted;
	}

	bool PromagReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;

		if(d_promagIdentifier.size() == 0)
		{
			retrieveReaderIdentifier();
		}

		if (d_insertedChip)
		{
			unsigned int currentWait = 0;
			do
			{
				boost::shared_ptr<Chip> chip = getChipInAir();
				if (chip)
				{
					if (chip->getChipIdentifier() != d_insertedChip->getChipIdentifier())
					{
						d_insertedChip.reset();
						removed = true;
					}
				}
				else
				{
					d_insertedChip.reset();
					removed = true;
				}

				if (!removed)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					currentWait += 100;
				}
			} while (!removed && (maxwait == 0 || currentWait < maxwait));
		}

		return removed;
	}

	bool PromagReaderUnit::connect()
	{
		return true;
	}

	void PromagReaderUnit::disconnect()
	{
	}

	std::vector<unsigned char> PromagReaderUnit::getPingCommand() const
	{
		std::vector<unsigned char> cmd;

		cmd.push_back(static_cast<unsigned char>('N'));

		return cmd;
	}

	boost::shared_ptr<Chip> PromagReaderUnit::getChipInAir()
	{
		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>('R'));

		std::vector<unsigned char> rawidbuf = getDefaultPromagReaderCardAdapter()->sendCommand(cmd);
		if (rawidbuf.size() > 0)
		{
			string rawidstr = BufferHelper::getStdString(rawidbuf);
			std::vector<unsigned char> insertedId = XmlSerializable::formatHexString(rawidstr);	
			chip = ReaderUnit::createChip(
				(d_card_type == "UNKNOWN" ? "GenericTag" : d_card_type),
				insertedId
			);
		}

		return chip;
	}
	
	boost::shared_ptr<Chip> PromagReaderUnit::createChip(std::string type)
	{
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			boost::shared_ptr<ReaderCardAdapter> rca;

			if (type == "GenericTag")
				rca = getDefaultReaderCardAdapter();
			else
				return chip;

			rca->setDataTransport(getDataTransport());
		}
		return chip;
	}

	boost::shared_ptr<Chip> PromagReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > PromagReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<PromagReaderCardAdapter> PromagReaderUnit::getDefaultPromagReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<PromagReaderCardAdapter>(adapter);
	}

	string PromagReaderUnit::getReaderSerialNumber()
	{
		string ret;

		return ret;
	}

	bool PromagReaderUnit::isConnected()
	{
		return bool(d_insertedChip);
	}

	bool PromagReaderUnit::connectToReader()
	{
		return getDataTransport()->connect();
	}

	void PromagReaderUnit::disconnectFromReader()
	{
		getDataTransport()->disconnect();
	}

	void PromagReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		ReaderUnit::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void PromagReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		ReaderUnit::unSerialize(node);
	}

	boost::shared_ptr<PromagReaderProvider> PromagReaderUnit::getPromagReaderProvider() const
	{
		return boost::dynamic_pointer_cast<PromagReaderProvider>(getReaderProvider());
	}

	bool PromagReaderUnit::retrieveReaderIdentifier()
	{
		bool ret;
		std::vector<unsigned char> cmd;
		try
		{
			cmd.push_back(static_cast<unsigned char>('N'));

			std::vector<unsigned char> r = getDefaultPromagReaderCardAdapter()->sendCommand(cmd);			

			d_promagIdentifier = r;
			ret = true;
		}
		catch(std::exception&)
		{
			ret = false;
		}

		return ret;
	}

	std::vector<unsigned char> PromagReaderUnit::getPromagIdentifier()
	{
		return d_promagIdentifier;
	}
}

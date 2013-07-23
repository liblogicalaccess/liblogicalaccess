/**
 * \file a3mlgm5600readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 reader unit.
 */

#include "a3mlgm5600readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "readercardadapters/a3mlgm5600readercardadapter.hpp"
#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include "logicalaccess/readerproviders/readerunit.hpp"
#include "a3mlgm5600readerprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "a3mlgm5600ledbuzzerdisplay.hpp"
#include "a3mlgm5600lcddisplay.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/udpdatatransport.hpp"


namespace logicalaccess
{
	A3MLGM5600ReaderUnit::A3MLGM5600ReaderUnit()
		: ReaderUnit()
{
		d_readerUnitConfig.reset(new A3MLGM5600ReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<A3MLGM5600ReaderCardAdapter> (new A3MLGM5600ReaderCardAdapter()));
		boost::shared_ptr<UdpDataTransport> dataTransport(new UdpDataTransport());
		dataTransport->setIpAddress("192.168.1.100");
		dataTransport->setPort(2000);
		setDataTransport(dataTransport);
		d_ledBuzzerDisplay.reset(new A3MLGM5600LEDBuzzerDisplay());
		d_lcdDisplay.reset(new A3MLGM5600LCDDisplay());
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/A3MLGM5600ReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	A3MLGM5600ReaderUnit::~A3MLGM5600ReaderUnit()
	{
		
	}
	
	std::string A3MLGM5600ReaderUnit::getName() const
	{
		return std::string("");
	}

	std::string A3MLGM5600ReaderUnit::getConnectedName()
	{
		char conv[64];
		sprintf(conv, "A3MLGM5600 Device Address: %d", d_deviceAddress);

		return std::string(conv);
	}

	void A3MLGM5600ReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
	}

	bool A3MLGM5600ReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool inserted = false;
		unsigned int currentWait = 0;

		do
		{
			try
			{
				std::vector<unsigned char> buf = hlRequest();
				if (buf.size() > 0)
				{
					d_insertedChip = createChip((d_card_type == "UNKNOWN") ? "GenericTag" : d_card_type);
					d_insertedChip->setChipIdentifier(buf);
					inserted = true;
				}
			}
			catch (LibLogicalAccessException&)
			{
				inserted = false;
			}

			if (!inserted)
			{
#ifdef _WINDOWS
				Sleep(250);
#elif defined(LINUX)
				usleep(250000);
#endif
				currentWait += 250;
			}
		} while (!inserted && (maxwait == 0 || currentWait < maxwait));

		return inserted;
	}

	bool A3MLGM5600ReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;

		if (d_insertedChip)
		{
			do
			{
				std::vector<unsigned char> buf = hlRequest();
				removed = (buf.size() == 0 || d_insertedChip->getChipIdentifier() != buf);

				if (!removed)
				{
	#ifdef _WINDOWS
					Sleep(250);
	#elif defined(LINUX)
					usleep(250000);
	#endif
					currentWait += 250;
				}
			} while (!removed && (maxwait == 0 || currentWait < maxwait));

			if (removed)
			{
				d_insertedChip.reset();
			}
		}

		return removed;
	}
	
	string A3MLGM5600ReaderUnit::getPADKey()
	{
		boost::shared_ptr<A3MLGM5600ReaderCardAdapter> adapter = getDefaultA3MLGM5600ReaderCardAdapter();
		std::vector<unsigned char> res = adapter->sendCommand(0x17, std::vector<unsigned char>());
		
		return BufferHelper::getStdString(res);
	}

	std::vector<unsigned char> A3MLGM5600ReaderUnit::hlRequest()
	{
		std::vector<unsigned char> buf;
		buf.push_back(0x01);	// 0x00: Request Idle, 0x01: Request all (wake up all)

		return getDefaultA3MLGM5600ReaderCardAdapter()->sendCommand(0x98, buf);
	}

	bool A3MLGM5600ReaderUnit::connect()
	{
		return true;
	}

	void A3MLGM5600ReaderUnit::disconnect()
	{
		
	}

	bool A3MLGM5600ReaderUnit::connectToReader()
	{
		getDataTransport()->setReaderUnit(shared_from_this());
		return getDataTransport()->connect();
	}

	void A3MLGM5600ReaderUnit::disconnectFromReader()
	{
		getDataTransport()->disconnect();
	}

	boost::shared_ptr<Chip> A3MLGM5600ReaderUnit::createChip(std::string type)
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

	boost::shared_ptr<Chip> A3MLGM5600ReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > A3MLGM5600ReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<A3MLGM5600ReaderCardAdapter> A3MLGM5600ReaderUnit::getDefaultA3MLGM5600ReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<A3MLGM5600ReaderCardAdapter>(adapter);
	}

	string A3MLGM5600ReaderUnit::getReaderSerialNumber()
	{	
		boost::shared_ptr<A3MLGM5600ReaderCardAdapter> adapter = getDefaultA3MLGM5600ReaderCardAdapter();
		std::vector<unsigned char> res = adapter->sendCommand(0x09, std::vector<unsigned char>());
		
		return BufferHelper::getHex(res);
	}

	bool A3MLGM5600ReaderUnit::isConnected()
	{
		return bool(d_insertedChip);
	}	

	void A3MLGM5600ReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		ReaderUnit::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void A3MLGM5600ReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		ReaderUnit::unSerialize(node);
	}

	boost::shared_ptr<A3MLGM5600ReaderProvider> A3MLGM5600ReaderUnit::getA3MLGM5600ReaderProvider() const
	{
		return boost::dynamic_pointer_cast<A3MLGM5600ReaderProvider>(getReaderProvider());
	}

	boost::shared_ptr<A3MLGM5600ReaderUnit> A3MLGM5600ReaderUnit::getSingletonInstance()
	{
		static boost::shared_ptr<A3MLGM5600ReaderUnit> instance;
		if (!instance)
		{
			instance.reset(new A3MLGM5600ReaderUnit());
		}
		return instance;
	}

	void A3MLGM5600ReaderUnit::resetRF(int offtime)
	{
		std::vector<unsigned char> data;
		data.push_back(static_cast<unsigned char>(offtime));

		getDefaultA3MLGM5600ReaderCardAdapter()->sendCommand(0x27, data, 1000 + (offtime * 100));
	}
}

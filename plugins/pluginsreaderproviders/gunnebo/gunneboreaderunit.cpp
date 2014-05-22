/**
 * \file gunneboreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Gunnebo reader unit.
 */

#include "gunneboreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "gunneboreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/gunneboreadercardadapter.hpp"
#include <boost/filesystem.hpp>
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

namespace logicalaccess
{
	/*
	 *
	 *  Gunnebo reader only send ID through Serial port. It doesn't support any request command !
	 *
	 */

	GunneboReaderUnit::GunneboReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new GunneboReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<GunneboReaderCardAdapter> (new GunneboReaderCardAdapter()));
		boost::shared_ptr<SerialPortDataTransport> dataTransport(new SerialPortDataTransport());
		setDataTransport(dataTransport);
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/GunneboReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }

		removalIdentifier.clear();
	}

	GunneboReaderUnit::~GunneboReaderUnit()
	{
		disconnectFromReader();
	}

	std::string GunneboReaderUnit::getName() const
	{
		return getDataTransport()->getName();
	}

	std::string GunneboReaderUnit::getConnectedName()
	{
		return getName();
	}

	void GunneboReaderUnit::setCardType(std::string cardType)
	{
		LOG(LogLevel::INFOS) << ) << , "Setting card type {0x%s(%s)}", cardType.c_str(), cardType.c_str());
		d_card_type = cardType;
	}

	bool GunneboReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool oldValue = Settings::getInstance()->IsLogEnabled;
		if (oldValue && !Settings::getInstance()->SeeWaitInsertionLog)
		{
			Settings::getInstance()->IsLogEnabled = false;		// Disable logs for this part (otherwise too much log output in file)
		}

		LOG(LogLevel::INFOS) << ) << , "Waiting insertion... max wait {%u}", maxwait);

		bool inserted = false;
		unsigned int currentWait = 0;
		std::vector<unsigned char> createChipId;
		
		try
		{
			// Gunnebo reader doesn't handle commands but we want to simulate the same behavior that for all readers
			// So we send a dummy commmand which does nothing
			do
			{
				try
				{
					if (removalIdentifier.size() > 0)
					{
						createChipId = removalIdentifier;
					}
					else
					{
						std::vector<unsigned char> cmd;
						cmd.push_back(0xff);	// trick
					
						std::vector<unsigned char> tmpASCIIId = getDefaultGunneboReaderCardAdapter()->sendCommand(cmd);

						if (tmpASCIIId.size() > 0)
						{
							createChipId = processCardId(tmpASCIIId);
						}
					}

					if (createChipId.size() > 0)
					{
						d_insertedChip = ReaderUnit::createChip((d_card_type == "UNKNOWN" ? "GenericTag" : d_card_type), createChipId);
						LOG(LogLevel::INFOS) << ) << , "Chip detected !");
						inserted = true;
					}
				}
				catch (std::exception&)
				{
					// No response received is ignored !
				}

				if (!inserted)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					currentWait += 500;
				}
			} while (!inserted && (maxwait == 0 || currentWait < maxwait));
		}
		catch (...)
		{
			Settings::getInstance()->IsLogEnabled = oldValue;
			throw;
		}

		removalIdentifier.clear();

		LOG(LogLevel::INFOS) << ) << , "Returns card inserted ? {%d} function timeout expired ? {%d}", inserted, (maxwait != 0 && currentWait >= maxwait));
		Settings::getInstance()->IsLogEnabled = oldValue;

		return inserted;
	}

	bool GunneboReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool oldValue = Settings::getInstance()->IsLogEnabled;
		if (oldValue && !Settings::getInstance()->SeeWaitRemovalLog)
		{
			Settings::getInstance()->IsLogEnabled = false;		// Disable logs for this part (otherwise too much log output in file)
		}

		LOG(LogLevel::INFOS) << ) << , "Waiting removal... max wait {%u}", maxwait);
		bool removed = false;
		removalIdentifier.clear();

		unsigned int currentWait = 0;
		try
		{
			// The inserted chip will stay inserted until a new identifier is read on the serial port.
			if (d_insertedChip)
			{
				do
				{
					try
					{
						std::vector<unsigned char> cmd;
						cmd.push_back(0xff);	// trick

						std::vector<unsigned char> buf = getDefaultGunneboReaderCardAdapter()->sendCommand(cmd);
						if (buf.size() > 0)
						{
							std::vector<unsigned char> tmpId = processCardId(buf);
							if (tmpId.size() > 0 && (tmpId != d_insertedChip->getChipIdentifier()))
							{
								LOG(LogLevel::INFOS) << ) << , "Card found but not same chip ! The previous card has been removed !");
								d_insertedChip.reset();
								removalIdentifier = tmpId;
								removed = true;
							}
						}
					}
					catch (std::exception&)
					{
						// No response received is ignored !
					}

					if (!removed)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
						currentWait += 500;
					}
				} while (!removed && (maxwait == 0 || currentWait < maxwait));
			}
		}
		catch (...)
		{
			Settings::getInstance()->IsLogEnabled = oldValue;
			throw;
		}

		LOG(LogLevel::INFOS) << ) << , "Returns card removed ? {%d} - function timeout expired ? {%d}", removed, (maxwait != 0 && currentWait >= maxwait));

		Settings::getInstance()->IsLogEnabled = oldValue;

		return removed;
	}

	std::vector<unsigned char> GunneboReaderUnit::processCardId(std::vector<unsigned char>& rawSerialData)
	{
		std::vector<unsigned char> ret;

		if (rawSerialData.size() > 0)
		{
			unsigned long long l = atoull(BufferHelper::getStdString(rawSerialData));
			char bufTmpId[128];
			memset(bufTmpId, 0x00, sizeof(bufTmpId));
#if !defined(__unix__)
			sprintf_s(bufTmpId, sizeof(bufTmpId), "%012llx", l);
#else
			sprintf(bufTmpId, "%012llx", l);
#endif

			ret = formatHexString(std::string(bufTmpId));
		}

		return ret;
	}

	bool GunneboReaderUnit::connect()
	{
		LOG(LogLevel::WARNINGS) << , "Connect do nothing with Gunnebo reader");
		return true;
	}

	void GunneboReaderUnit::disconnect()
	{
		LOG(LogLevel::WARNINGS) << , "Disconnect do nothing with Gunnebo reader");
	}
	
	boost::shared_ptr<Chip> GunneboReaderUnit::createChip(std::string type)
	{
		LOG(LogLevel::INFOS) << ) << , "Creating chip... chip type {0x%s(%s)}", type.c_str());
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			LOG(LogLevel::INFOS) << ) << , "Chip created successfully !");
			boost::shared_ptr<ReaderCardAdapter> rca;

			if (type == "GenericTag")
			{
				LOG(LogLevel::INFOS) << ) << , "Generic tag Chip created");
				rca = getDefaultReaderCardAdapter();
			}
			else
				return chip;

			rca->setDataTransport(getDataTransport());
		}
		return chip;
	}

	boost::shared_ptr<Chip> GunneboReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > GunneboReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<GunneboReaderCardAdapter> GunneboReaderUnit::getDefaultGunneboReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<GunneboReaderCardAdapter>(adapter);
	}

	string GunneboReaderUnit::getReaderSerialNumber()
	{
		LOG(LogLevel::WARNINGS) << , "Do nothing with Gunnebo reader");
		string ret;
		return ret;
	}

	bool GunneboReaderUnit::isConnected()
	{
		if (d_insertedChip)
			LOG(LogLevel::INFOS) << ) << , "Is connected {1}");
		else
			LOG(LogLevel::INFOS) << ) << , "Is connected {0}");
		return bool(d_insertedChip);
	}

	bool GunneboReaderUnit::connectToReader()
	{
		getDataTransport()->setReaderUnit(shared_from_this());
		return getDataTransport()->connect();
	}

	void GunneboReaderUnit::disconnectFromReader()
	{
		getDataTransport()->disconnect();
	}

	std::vector<unsigned char> GunneboReaderUnit::getPingCommand() const
	{
		std::vector<unsigned char> cmd;

		cmd.push_back(0xff);
		
		return cmd;
	}

	void GunneboReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		ReaderUnit::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void GunneboReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		ReaderUnit::unSerialize(node);
	}

	boost::shared_ptr<GunneboReaderProvider> GunneboReaderUnit::getGunneboReaderProvider() const
	{
		return boost::dynamic_pointer_cast<GunneboReaderProvider>(getReaderProvider());
	}
}

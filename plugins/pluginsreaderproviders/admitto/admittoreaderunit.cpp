/**
 * \file admittoreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Admitto reader unit.
 */

#include "admittoreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "admittoreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/admittoreadercardadapter.hpp"
#include <boost/filesystem.hpp>
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

namespace logicalaccess
{
	/*
	 *
	 *  WARNING: THE READER ONLY WORKS WITH X86 SYSTEMS !
	 *  Admitto reader only send ID through Serial port on card insertion and [0x4e] on card removed. It doesn't support any request command !  
	 *
	 */

	AdmittoReaderUnit::AdmittoReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new AdmittoReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<AdmittoReaderCardAdapter> (new AdmittoReaderCardAdapter()));
		boost::shared_ptr<SerialPortDataTransport> dataTransport(new SerialPortDataTransport());
		setDataTransport(dataTransport);
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/AdmittoReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	AdmittoReaderUnit::~AdmittoReaderUnit()
	{
		disconnectFromReader();
	}

	std::string AdmittoReaderUnit::getName() const
	{
		return getDataTransport()->getName();
	}

	std::string AdmittoReaderUnit::getConnectedName()
	{
		return getName();
	}

	void AdmittoReaderUnit::setCardType(std::string cardType)
	{
		INFO_("Setting card type {0x%s(%s)}", cardType.c_str(), cardType.c_str());
		d_card_type = cardType;
	}

	bool AdmittoReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool oldValue = Settings::getInstance()->IsLogEnabled;
		if (oldValue && !Settings::getInstance()->SeeWaitInsertionLog)
		{
			Settings::getInstance()->IsLogEnabled = false;		// Disable logs for this part (otherwise too much log output in file)
		}

		INFO_("Waiting insertion... max wait {%u}", maxwait);

		bool inserted = false;
		unsigned int currentWait = 0;
		
		try
		{
			// Admitto reader doesn't handle commands but we want to simulate the same behavior that for all readers
			// So we send a dummy commmand which does nothing
			do
			{
				try
				{
					std::vector<unsigned char> cmd;
					cmd.push_back(0xff);	// trick

					std::vector<unsigned char> buf = getDefaultAdmittoReaderCardAdapter()->sendCommand(cmd);
					if (buf.size() > 0)
					{
						if (buf[0] == 0x4e)
						{
							INFO_("[Card removed] response received when we were on WaitInsertion! We ignore this response and wait for valid card number.");
						}
						else
						{
							unsigned int l = static_cast<unsigned int>(atoull(BufferHelper::getStdString(buf)));
							char bufTmpId[64];
							sprintf(bufTmpId, "%08x", l);
							d_insertedChip = ReaderUnit::createChip(
								(d_card_type == "UNKNOWN" ? "GenericTag" : d_card_type),
								formatHexString(std::string(bufTmpId))
							);
							INFO_("Chip detected !");
							inserted = true;
						}
					}
				}
				catch (std::exception&)
				{
					// No response received is ignored !
				}

				if (!inserted)
				{
#ifdef _WINDOWS
					Sleep(500);
#elif defined(__unix__)
					usleep(500000);
#endif
					currentWait += 500;
				}
			} while (!inserted && (maxwait == 0 || currentWait < maxwait));
		}
		catch (...)
		{
			Settings::getInstance()->IsLogEnabled = oldValue;
			throw;
		}

		INFO_("Returns card inserted ? {%d} function timeout expired ? {%d}", inserted, (maxwait != 0 && currentWait >= maxwait));
		Settings::getInstance()->IsLogEnabled = oldValue;

		return inserted;
	}

	bool AdmittoReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool oldValue = Settings::getInstance()->IsLogEnabled;
		if (oldValue && !Settings::getInstance()->SeeWaitRemovalLog)
		{
			Settings::getInstance()->IsLogEnabled = false;		// Disable logs for this part (otherwise too much log output in file)
		}

		INFO_("Waiting removal... max wait {%u}", maxwait);
		bool removed = false;
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

						std::vector<unsigned char> buf = getDefaultAdmittoReaderCardAdapter()->sendCommand(cmd);
						if (buf.size() > 0)
						{
							if (buf[0] == 0x4e)
							{
								INFO_("Card removed !");
								d_insertedChip.reset();
								removed = true;
							}
							else
							{
								unsigned int l = static_cast<unsigned int>(atoull(BufferHelper::getStdString(buf)));
								char bufTmpId[64];
								sprintf(bufTmpId, "%08x", l);
								boost::shared_ptr<Chip> chip = ReaderUnit::createChip(
									(d_card_type == "UNKNOWN" ? "GenericTag" : d_card_type),
									formatHexString(std::string(bufTmpId))
								);

								if (chip->getChipIdentifier() != d_insertedChip->getChipIdentifier())
								{
									INFO_("Card found but not same chip ! The previous card has been removed !");
									d_insertedChip.reset();
									removed = true;
								}
							}
						}
					}
					catch (std::exception&)
					{
						// No response received is ignored !
					}

					if (!removed)
					{
#ifdef _WINDOWS
						Sleep(500);
#elif defined(__unix__)
						usleep(500000);
#endif
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

		INFO_("Returns card removed ? {%d} - function timeout expired ? {%d}", removed, (maxwait != 0 && currentWait >= maxwait));

		Settings::getInstance()->IsLogEnabled = oldValue;

		return removed;
	}

	bool AdmittoReaderUnit::connect()
	{
		WARNING_("Connect do nothing with Admitto reader");
		return true;
	}

	void AdmittoReaderUnit::disconnect()
	{
		WARNING_("Disconnect do nothing with Admitto reader");
	}

	boost::shared_ptr<Chip> AdmittoReaderUnit::createChip(std::string type)
	{
		INFO_("Creating chip... chip type {0x%s(%s)}", type.c_str());
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			INFO_("Chip created successfully !");
			boost::shared_ptr<ReaderCardAdapter> rca;

			if (type == "GenericTag")
			{
				INFO_("Generic tag Chip created");
				rca = getDefaultReaderCardAdapter();
			}
			else
				return chip;

			rca->setDataTransport(getDataTransport());
		}
		return chip;
	}

	boost::shared_ptr<Chip> AdmittoReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > AdmittoReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<AdmittoReaderCardAdapter> AdmittoReaderUnit::getDefaultAdmittoReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<AdmittoReaderCardAdapter>(adapter);
	}

	std::string AdmittoReaderUnit::getReaderSerialNumber()
	{
		WARNING_("Do nothing with Admitto reader");
		std::string ret;
		return ret;
	}

	bool AdmittoReaderUnit::isConnected()
	{
		if (d_insertedChip)
			INFO_("Is connected {1}");
		else
			INFO_("Is connected {0}");
		return bool(d_insertedChip);
	}

	bool AdmittoReaderUnit::connectToReader()
	{
		getDataTransport()->setReaderUnit(shared_from_this());
		return getDataTransport()->connect();
	}

	void AdmittoReaderUnit::disconnectFromReader()
	{
		INFO_("Disconnecting from reader...");
		getDataTransport()->disconnect();
	}

	std::vector<unsigned char> AdmittoReaderUnit::getPingCommand() const
	{
		std::vector<unsigned char> cmd;

		cmd.push_back(0xff);

		return cmd;
	}

	void AdmittoReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		ReaderUnit::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void AdmittoReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		ReaderUnit::unSerialize(node);
	}

	boost::shared_ptr<AdmittoReaderProvider> AdmittoReaderUnit::getAdmittoReaderProvider() const
	{
		return boost::dynamic_pointer_cast<AdmittoReaderProvider>(getReaderProvider());
	}
}

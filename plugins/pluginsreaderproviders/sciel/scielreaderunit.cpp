/**
 * \file scielreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SCIEL reader unit.
 */

#include "scielreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "scielreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/scielreadercardadapter.hpp"
#include <boost/filesystem.hpp>
#include "readercardadapters/scieldatatransport.hpp"

namespace logicalaccess
{
	SCIELReaderUnit::SCIELReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new SCIELReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<SCIELReaderCardAdapter> (new SCIELReaderCardAdapter()));
		boost::shared_ptr<ScielDataTransport> dataTransport(new ScielDataTransport());
		setDataTransport(dataTransport);
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/SCIELReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	SCIELReaderUnit::~SCIELReaderUnit()
	{
		disconnectFromReader();
	}

	std::string SCIELReaderUnit::getName() const
	{
		return getDataTransport()->getName();
	}

	std::string SCIELReaderUnit::getConnectedName()
	{
		return getName();
	}

	void SCIELReaderUnit::setCardType(std::string cardType)
	{
		LOG(LogLevel::INFOS) << "Setting card type {" << cardType << "}";
		d_card_type = cardType;
	}

	bool SCIELReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool oldValue = Settings::getInstance()->IsLogEnabled;
		if (oldValue && !Settings::getInstance()->SeeWaitInsertionLog)
		{
			Settings::getInstance()->IsLogEnabled = false;		// Disable logs for this part (otherwise too much log output in file)
		}

		LOG(LogLevel::INFOS) << "Waiting insertion... max wait {" << maxwait << "}";

		bool inserted = false;
		std::chrono::steady_clock::time_point const clock_timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(maxwait);	

		do
		{
			refreshChipList();
			std::vector<boost::shared_ptr<Chip> > chipList = getChipList();
			if (!chipList.empty())
			{
				d_insertedChip = chipList.front();
				inserted = true;
				LOG(LogLevel::INFOS) << "Chip detected !";
			}

			if (!inserted)
				std::this_thread::sleep_for(std::chrono::milliseconds(250));
		} while (!inserted && std::chrono::steady_clock::now() < clock_timeout);

		LOG(LogLevel::INFOS) << "Returns card inserted ? {" << inserted << "} function timeout expired ? {" << (std::chrono::steady_clock::now() < clock_timeout) << "}";
		Settings::getInstance()->IsLogEnabled = oldValue;

		return inserted;
	}

	bool SCIELReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool oldValue = Settings::getInstance()->IsLogEnabled;
		if (oldValue && !Settings::getInstance()->SeeWaitRemovalLog)
		{
			Settings::getInstance()->IsLogEnabled = false;		// Disable logs for this part (otherwise too much log output in file)
		}

		LOG(LogLevel::INFOS) << "Waiting removal... max wait {" << maxwait << "}";

		bool removed = false;
		std::chrono::steady_clock::time_point const clock_timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(maxwait);	
		if (d_insertedChip)
		{
			do
			{
				refreshChipList();
				std::vector<boost::shared_ptr<Chip> > chipList = getChipList();

				std::vector<boost::shared_ptr<Chip> >::iterator i = std::find_if (chipList.begin(), chipList.end(), SCIELReaderUnit::Finder(d_insertedChip));
				if (i == chipList.end())
				{
					d_insertedChip.reset();
					removed = true;
				}

				if (!removed)
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
			} while (!removed && std::chrono::steady_clock::now() < clock_timeout);
		}

		LOG(LogLevel::INFOS) << "Returns card removed ? {" << removed << "} - function timeout expired ? {" << (std::chrono::steady_clock::now() < clock_timeout) << "}";

		Settings::getInstance()->IsLogEnabled = oldValue;

		return removed;
	}

	bool SCIELReaderUnit::connect()
	{
		LOG(LogLevel::WARNINGS) << "Connect do nothing with Sciel reader";
		return true;
	}

	void SCIELReaderUnit::disconnect()
	{
		LOG(LogLevel::WARNINGS) << "Disconnect do nothing with Sciel reader";
	}

	bool ChipSortPredicate(const boost::shared_ptr<Chip>& lhs, const boost::shared_ptr<Chip>& rhs)
	{
		return lhs->getReceptionLevel() < rhs->getReceptionLevel();
	}

	std::list<boost::shared_ptr<Chip> > SCIELReaderUnit::getReaderChipList()
	{
		LOG(LogLevel::INFOS) << "Retrieving reader chip list...";
		std::list<boost::shared_ptr<Chip> > chipList;
		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x41));
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(d_scielIdentifier[0]);
		cmd.push_back(d_scielIdentifier[1]);

		try
		{
			LOG(LogLevel::INFOS) << "Sending COM command...";

			std::list<std::vector<unsigned char> > allTags = getDefaultSCIELReaderCardAdapter()->receiveTagsListCommand(cmd);

			for (std::list<std::vector<unsigned char> >::iterator i = allTags.begin(); i != allTags.end(); ++i)
			{
				boost::shared_ptr<Chip> rChip = createChipFromBuffer((*i));
				if (rChip)
				{
					chipList.push_back(rChip);
				}
			}
		}
		catch(std::exception& ex)
		{
			LOG(LogLevel::ERRORS) << "Error retrieving reader chip list: " << ex.what();
		}

		LOG(LogLevel::INFOS) << "Chip list retrieved (" << chipList.size() << " chips)!";
		return chipList;
	}

	/*
	 * The timer "Time before tag out" is not used anymore: If tag was in tag-in and go in tag-out, we keep the tag in the list (waitRemoval will still wait the tag go OUT of the tag-out threshold)
	 * We wait one or x tags in Tag-in, and we remove the tag from the list only if the tag is not anymore in the tag-out threshold.
	 */
	std::list<boost::shared_ptr<Chip> > SCIELReaderUnit::refreshChipList()
	{
		std::list<boost::shared_ptr<Chip> > chipList = getReaderChipList();

		LOG(LogLevel::DEBUGS) << "**** READER CHIP LIST RAW ****";
		for (std::list<boost::shared_ptr<Chip> >::iterator i = chipList.begin(); i != chipList.end(); ++i)
		{
			LOG(LogLevel::DEBUGS) << "  -> Chip identifier " << BufferHelper::getHex((*i)->getChipIdentifier()) << " Reception {" << static_cast<unsigned int>((*i)->getReceptionLevel()) << "}";
		}
		LOG(LogLevel::DEBUGS) << "**** END LIST ****";

		// Timer management. We handle the global timeout when tags disappears (it was done before by the key, but it sucked !)
		for (std::vector<boost::shared_ptr<Chip> >::iterator i = d_chipList.begin(); i != d_chipList.end(); ++i)
		{
			bool findChip = false;
			for (std::list<boost::shared_ptr<Chip> >::iterator j = chipList.begin(); j != chipList.end() && !findChip; ++j)
			{
				findChip = ((*i)->getChipIdentifier() == (*j)->getChipIdentifier());
			}
			
			std::string hexid = BufferHelper::getHex((*i)->getChipIdentifier());

			// The chip cannot be found, manage timer
			if (!findChip)
			{
				if (d_tagGoneTimeout[hexid] < getSCIELConfiguration()->getTimeRemoval())
				{
					d_tagGoneTimeout[hexid]++;
					chipList.push_back(*i);
				}
				else
				{
					d_tagGoneTimeout.erase(hexid);
				}
			}
			else
			{
				d_tagGoneTimeout[hexid] = 0;
			}
		}

		// Add new chip to the timer management
		for (std::list<boost::shared_ptr<Chip> >::iterator i = chipList.begin(); i != chipList.end(); ++i)
		{
			std::string hexid = BufferHelper::getHex((*i)->getChipIdentifier());
			if (d_tagGoneTimeout.find(hexid) == d_tagGoneTimeout.end())
			{
				d_tagGoneTimeout[hexid] = 0;
			}
		}

		LOG(LogLevel::DEBUGS) << "**** READER CHIP LIST AFTER GLOBAL TIMEOUT ****";
		for (std::list<boost::shared_ptr<Chip> >::iterator i = chipList.begin(); i != chipList.end(); ++i)
		{
			LOG(LogLevel::DEBUGS) << "  -> Chip identifier " << BufferHelper::getHex((*i)->getChipIdentifier()) << " Reception {" << static_cast<unsigned int>((*i)->getReceptionLevel()) << "}";
		}
		LOG(LogLevel::DEBUGS) << "**** END LIST ****";

		// Remember, the chip list in air is sorted by reception level
		chipList.sort(ChipSortPredicate);

		d_chipList.clear();

		// refresh area
		std::map<std::string, int> tagInArea;
		std::map<std::string, int> tagOutArea;
		std::map<std::string, int> safetyArea;
		std::map<std::string, int> ignoreArea;
		bool safetyAreaUsed = false;

		for (std::list<boost::shared_ptr<Chip> >::iterator i = chipList.begin(); i != chipList.end(); ++i)
		{
			int timeInArea = 1;
			bool forceTagArea = false;

			// Refresh inserted chip info
			if (d_insertedChip)
			{
				if (d_insertedChip->getChipIdentifier() == (*i)->getChipIdentifier())
				{
					d_insertedChip->setPowerStatus((*i)->getPowerStatus());
					d_insertedChip->setReceptionLevel((*i)->getReceptionLevel());
				}
			}

			std::string identifierHexStr = BufferHelper::getHex((*i)->getChipIdentifier());
			// In safety area
			if ((*i)->getReceptionLevel() <= getSCIELConfiguration()->getSafetyThreshold())
			{
				if (d_safetyArea.find(identifierHexStr) != d_safetyArea.end())
				{
					timeInArea += d_safetyArea[identifierHexStr];
				}

				if (timeInArea < (getSCIELConfiguration()->getTimeBeforeSafety() / 1000))
				{
					safetyArea[identifierHexStr] = timeInArea;
					forceTagArea = true;
				}
				else
				{
					safetyArea[identifierHexStr] = getSCIELConfiguration()->getTimeBeforeSafety() / 1000;
					d_chipList.push_back((*i));
					safetyAreaUsed = true;
				}
			}
			
			// Not (or not yet) in safety area
			if (forceTagArea || (*i)->getReceptionLevel() > getSCIELConfiguration()->getSafetyThreshold())
			{
				// We don't care about chip in tag-*** area if chip exists in safety area
				if (!safetyAreaUsed)
				{
					bool insertChip = false;
					if ((*i)->getReceptionLevel() <= getSCIELConfiguration()->getTagInThreshold())
					{
						if (getSCIELConfiguration()->getTagInThreshold() < getSCIELConfiguration()->getTagOutThreshold())
						{							
							if (d_tagInArea.find(identifierHexStr) != d_tagInArea.end())
							{
								timeInArea += d_tagInArea[identifierHexStr];
							}

							if (timeInArea < (getSCIELConfiguration()->getTimeBeforeTagIn() / 1000))
							{
								tagInArea[identifierHexStr] = timeInArea;
								if (d_safetyArea.find(identifierHexStr) != d_safetyArea.end())
								{
									safetyArea[identifierHexStr] = d_safetyArea[identifierHexStr];
									d_chipList.push_back((*i));
									safetyAreaUsed = true;
								}
							}
							else
							{
								tagInArea[identifierHexStr] = getSCIELConfiguration()->getTimeBeforeTagIn() / 1000;
								insertChip = true;								
							}
						}
						else
						{
							bool ignoreTag = false;
							if (d_ignoreArea.find(identifierHexStr) != d_ignoreArea.end())
							{
								ignoreTag = (d_ignoreArea[identifierHexStr] >= getSCIELConfiguration()->getTimeBeforeTagOut() / 1000);
							}

							if (!ignoreTag)
							{
								// Tag-out area
								if ((*i)->getReceptionLevel() <= getSCIELConfiguration()->getTagOutThreshold())
								{
									if (d_tagOutArea.find(identifierHexStr) != d_tagOutArea.end())
									{
										timeInArea += d_tagOutArea[identifierHexStr];
									}

									if (timeInArea < (getSCIELConfiguration()->getTimeBeforeTagOut() / 1000))
									{										
										tagOutArea[identifierHexStr] = timeInArea;
										// The tag doesn't reach the time yet, we keep it in last area
										if (d_safetyArea.find(identifierHexStr) != d_safetyArea.end())
										{
											safetyArea[identifierHexStr] = d_safetyArea[identifierHexStr];
											if (safetyArea[identifierHexStr] >= getSCIELConfiguration()->getTimeBeforeSafety() / 1000)
											{
												d_chipList.push_back((*i));
												safetyAreaUsed = true;
											}
										}
										else if(d_tagInArea.find(identifierHexStr) != d_tagInArea.end())
										{
											tagInArea[identifierHexStr] = d_tagInArea[identifierHexStr];
											if (tagInArea[identifierHexStr] >= getSCIELConfiguration()->getTimeBeforeTagIn() / 1000)
											{
												insertChip = true;
											}
										}
									}
									else
									{
										tagOutArea[identifierHexStr] = getSCIELConfiguration()->getTimeBeforeTagOut() / 1000;
										insertChip = true;
									}
								}
								else
								{
									// Exited tag-out area, we must ignore it in tag-in area
									if (d_safetyArea.find(identifierHexStr) != d_safetyArea.end() || d_tagOutArea.find(identifierHexStr) != d_tagOutArea.end())
									{
										if (d_ignoreArea.find(identifierHexStr) != d_ignoreArea.end())
										{
											timeInArea += d_ignoreArea[identifierHexStr];
										}

										if (timeInArea < getSCIELConfiguration()->getTimeRemoval())
										{											
											ignoreArea[identifierHexStr] = timeInArea;
											// The tag doesn't reach the time yet, we keep it in last area
											if (d_safetyArea.find(identifierHexStr) != d_safetyArea.end())
											{
												safetyArea[identifierHexStr] = d_safetyArea[identifierHexStr];
												if (safetyArea[identifierHexStr] >= getSCIELConfiguration()->getTimeBeforeSafety() / 1000)
												{
													d_chipList.push_back((*i));
													safetyAreaUsed = true;
												}
											}
											else if (d_tagOutArea.find(identifierHexStr) != d_tagOutArea.end())
											{
												tagOutArea[identifierHexStr] = d_tagOutArea[identifierHexStr];
												if (tagOutArea[identifierHexStr] >= getSCIELConfiguration()->getTimeBeforeTagOut() / 1000)
												{													
													insertChip = true;
												}
											}
										}
										else
										{
											ignoreArea[identifierHexStr] = getSCIELConfiguration()->getTimeRemoval();							
										}
									}
									// Real tag-in
									else
									{
										if (d_tagInArea.find(identifierHexStr) != d_tagInArea.end())
										{
											timeInArea += d_tagInArea[identifierHexStr];
										}

										if (timeInArea < (getSCIELConfiguration()->getTimeBeforeTagIn() / 1000))
										{											
											tagInArea[identifierHexStr] = timeInArea;
										}
										else
										{
											tagInArea[identifierHexStr] = getSCIELConfiguration()->getTimeBeforeTagIn() / 1000;
											insertChip = true;								
										}
									}
								}
							}
							else
							{
								// Tag still present in tag-in area after tag-out, we continue to ignore it
								ignoreArea[identifierHexStr] = d_ignoreArea[identifierHexStr];
							}
						}
					}
					// Tag-out area (when tag-in threshold lower than tag-out threshold)
					else if ((*i)->getReceptionLevel() <= getSCIELConfiguration()->getTagOutThreshold())
					{
						// Was already inserted
						if (d_safetyArea.find(identifierHexStr) != d_safetyArea.end() || d_tagInArea.find(identifierHexStr) != d_tagInArea.end() || d_tagOutArea.find(identifierHexStr) != d_tagOutArea.end())
						{
							if (d_tagOutArea.find(identifierHexStr) != d_tagOutArea.end())
							{
								timeInArea += d_tagOutArea[identifierHexStr];
							}

							if (timeInArea < (getSCIELConfiguration()->getTimeBeforeTagOut() / 1000))
							{											
								tagOutArea[identifierHexStr] = timeInArea;
								// The tag doesn't reach the time yet, we keep it in last area
								if (d_safetyArea.find(identifierHexStr) != d_safetyArea.end())
								{
									safetyArea[identifierHexStr] = d_safetyArea[identifierHexStr];
									d_chipList.push_back((*i));
									safetyAreaUsed = true;
								}
								else if (d_tagInArea.find(identifierHexStr) != d_tagInArea.end())
								{
									tagInArea[identifierHexStr] = d_tagInArea[identifierHexStr];
									insertChip = true;
								}
							}
							else
							{
								tagOutArea[identifierHexStr] = getSCIELConfiguration()->getTimeBeforeTagOut() / 1000;
								insertChip = true;								
							}
						}
					}
					// Unknow area (high reception level, outside all area)
					else
					{
						if (d_safetyArea.find(identifierHexStr) != d_safetyArea.end() || d_tagInArea.find(identifierHexStr) != d_tagInArea.end() || d_tagOutArea.find(identifierHexStr) != d_tagOutArea.end())
						{
							if (d_ignoreArea.find(identifierHexStr) != d_ignoreArea.end())
							{
								timeInArea += d_ignoreArea[identifierHexStr];
							}

							if (timeInArea < getSCIELConfiguration()->getTimeRemoval())
							{											
								ignoreArea[identifierHexStr] = timeInArea;
								// The tag doesn't reach the time yet, we keep it in last area
								if (d_safetyArea.find(identifierHexStr) != d_safetyArea.end())
								{
									safetyArea[identifierHexStr] = d_safetyArea[identifierHexStr];
									if (safetyArea[identifierHexStr] >= getSCIELConfiguration()->getTimeBeforeSafety() / 1000)
									{
										d_chipList.push_back((*i));
										safetyAreaUsed = true;
									}
								}
								else if (d_tagInArea.find(identifierHexStr) != d_tagInArea.end())
								{
									tagInArea[identifierHexStr] = d_tagInArea[identifierHexStr];
									if (tagInArea[identifierHexStr] >= getSCIELConfiguration()->getTimeBeforeTagIn() / 1000)
									{													
										insertChip = true;
									}
								}
								else if (d_tagOutArea.find(identifierHexStr) != d_tagOutArea.end())
								{
									tagOutArea[identifierHexStr] = d_tagOutArea[identifierHexStr];
									if (tagOutArea[identifierHexStr] >= getSCIELConfiguration()->getTimeBeforeTagOut() / 1000)
									{													
										insertChip = true;
									}
								}
							}
							else
							{
								ignoreArea[identifierHexStr] = getSCIELConfiguration()->getTimeRemoval();							
							}
						}
					}

					// If safety area used, the chip list is already filled
					if (!safetyAreaUsed)
					{
						// We only insert chips which are recognized as inserted (after timeout management etc.)
						if (insertChip)
						{
							d_chipList.push_back((*i));
						}
					}

					// Old functionnality :
					// Only one tag was selected before. Disabled because now we list all tags that are in Tag-in or was in Tag-in and are now in Tag-out (so still available).
					/*if (insertChip)
					{
						// In tag-*** area, only one chip can be authorized
						if (d_chipList.size() == 0)
						{
							d_chipList.push_back((*i));
						}
						else
						{
							if (d_insertedChip)
							{
								// The last inserted chip is still in area, we use it
								if (d_insertedChip->getChipIdentifier() == (*i)->getChipIdentifier())
								{
									d_chipList.clear();
									d_chipList.push_back((*i));
								}
							}
						}
					}*/
				}
			}
		}

		d_tagInArea = tagInArea;
		d_tagOutArea = tagOutArea;
		d_safetyArea = safetyArea;
		d_ignoreArea = ignoreArea;

		LOG(LogLevel::DEBUGS) << "**** FINAL CHIP LIST ****";
		for (std::vector<boost::shared_ptr<Chip> >::iterator i = d_chipList.begin(); i != d_chipList.end(); ++i)
		{
			LOG(LogLevel::DEBUGS) << "  -> Chip identifier " << BufferHelper::getHex((*i)->getChipIdentifier()) << " Reception {" << static_cast<unsigned int>((*i)->getReceptionLevel()) << "}";
		}
		LOG(LogLevel::DEBUGS) << "**** END LIST ****";

		LOG(LogLevel::DEBUGS) << "**** TAG-IN AREA CHIP LIST ****";
		for (std::map<std::string, int>::iterator i = d_tagInArea.begin(); i != d_tagInArea.end(); ++i)
		{
			LOG(LogLevel::DEBUGS) << "  -> Chip identifier {" << (*i).first << "} Seconds in area {" << (*i).second << "}";
		}
		LOG(LogLevel::DEBUGS) << "**** END LIST ****";

		LOG(LogLevel::DEBUGS) << "**** TAG-OUT AREA CHIP LIST ****";
		for (std::map<std::string, int>::iterator i = d_tagOutArea.begin(); i != d_tagOutArea.end(); ++i)
		{
			LOG(LogLevel::DEBUGS) << "  -> Chip identifier {" << (*i).first << "} Seconds in area {" << (*i).second << "}";
		}
		LOG(LogLevel::DEBUGS) << "**** END LIST ****";

		LOG(LogLevel::DEBUGS) << "**** TAG-SAFETY AREA CHIP LIST ****";
		for (std::map<std::string, int>::iterator i = d_safetyArea.begin(); i != d_safetyArea.end(); ++i)
		{
			LOG(LogLevel::DEBUGS) << "  -> Chip identifier {" << (*i).first << "} Seconds in area {" << (*i).second << "}";
		}
		LOG(LogLevel::DEBUGS) << "**** END LIST ****";

		LOG(LogLevel::DEBUGS) << "**** TAG-IGNORE AREA CHIP LIST ****";
		for (std::map<std::string, int>::iterator i = d_ignoreArea.begin(); i != d_ignoreArea.end(); ++i)
		{
			LOG(LogLevel::DEBUGS) << "  -> Chip identifier {" << (*i).first << "} Seconds in area {" << (*i).second << "}";
		}
		LOG(LogLevel::DEBUGS) << "**** END LIST ****";

		return chipList;
	}

	boost::shared_ptr<Chip> SCIELReaderUnit::createChip(std::string type)
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

	boost::shared_ptr<Chip> SCIELReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > SCIELReaderUnit::getChipList()
	{		
		return d_chipList;
	}

	boost::shared_ptr<SCIELReaderCardAdapter> SCIELReaderUnit::getDefaultSCIELReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<SCIELReaderCardAdapter>(adapter);
	}

	string SCIELReaderUnit::getReaderSerialNumber()
	{
		string ret;

		return ret;
	}

	bool SCIELReaderUnit::isConnected()
	{
		return bool(d_insertedChip);
	}

	bool SCIELReaderUnit::connectToReader()
	{
		bool ret = getDataTransport()->connect();
		if (ret)
		{
			ret = retrieveReaderIdentifier();
			if (!ret)
			{
				getDataTransport()->disconnect();
			}
		}

		return ret;
	}

	void SCIELReaderUnit::disconnectFromReader()
	{
		getDataTransport()->disconnect();
	}

	std::vector<unsigned char> SCIELReaderUnit::getPingCommand() const
	{
		std::vector<unsigned char> cmd;

		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x39));
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x30));

		return cmd;
	}

	void SCIELReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		ReaderUnit::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void SCIELReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		ReaderUnit::unSerialize(node);
	}

	boost::shared_ptr<SCIELReaderProvider> SCIELReaderUnit::getSCIELReaderProvider() const
	{
		return boost::dynamic_pointer_cast<SCIELReaderProvider>(getReaderProvider());
	}

	bool SCIELReaderUnit::retrieveReaderIdentifier()
	{
		LOG(LogLevel::INFOS) << "Retrieving the SCIEL Reader Identifier...";
		bool ret;
		std::vector<unsigned char> cmd;
		d_scielIdentifier.clear();

		try
		{
			cmd.push_back(static_cast<unsigned char>(0x30));
			cmd.push_back(static_cast<unsigned char>(0x39));
			cmd.push_back(static_cast<unsigned char>(0x30));
			cmd.push_back(static_cast<unsigned char>(0x30));
			cmd.push_back(static_cast<unsigned char>(0x30));
			cmd.push_back(static_cast<unsigned char>(0x30));

			std::vector<unsigned char> r = getDefaultSCIELReaderCardAdapter()->sendCommand(cmd);
			EXCEPTION_ASSERT_WITH_LOG(r.size() >= 2, LibLogicalAccessException, "Bad response getting SCIEL reader identifier.");

			d_scielIdentifier = std::vector<unsigned char>(r.end() - 2 , r.end());

			ret = true;
		}
		catch(std::exception&)
		{
			ret = false;
		}

		LOG(LogLevel::INFOS) << "SCIEL Reader Identifier " << BufferHelper::getHex(d_scielIdentifier) << " Len {" << d_scielIdentifier.size() << "}...";

		return ret;
	}

	unsigned char SCIELReaderUnit::getTimeRemovalFromContextualStack()
	{
		unsigned char timeRemoval = 0;

		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x35));
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(d_scielIdentifier[0]);
		cmd.push_back(d_scielIdentifier[1]);

		std::vector<unsigned char> r = getDefaultSCIELReaderCardAdapter()->sendCommand(cmd);
		EXCEPTION_ASSERT_WITH_LOG(r.size() >= 6, LibLogicalAccessException, "Bad response getting SCIEL removal time. Bad response length.");
		EXCEPTION_ASSERT_WITH_LOG(r[0] == cmd[0] && r[1] == cmd[1], LibLogicalAccessException, "Bad response getting SCIEL removal time. Bad command response identifier.");
		EXCEPTION_ASSERT_WITH_LOG(r[4] == d_scielIdentifier[0] && r[5] == d_scielIdentifier[1], LibLogicalAccessException, "Bad response getting SCIEL removal time. Bad reader response identifier.");

		timeRemoval = static_cast<unsigned char>(strtoul(BufferHelper::getStdString(std::vector<unsigned char>(r.end() - 2, r.end())).c_str(), NULL, 16));

		return timeRemoval;
	}

	void SCIELReaderUnit::setTimeRemovalFromContextualStack(unsigned char timeRemoval)
	{
		char tmp[64];
		sprintf(tmp, "%02x", timeRemoval);
		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x36));
		cmd.push_back(static_cast<unsigned char>(tmp[0]));
		cmd.push_back(static_cast<unsigned char>(tmp[1]));
		cmd.push_back(d_scielIdentifier[0]);
		cmd.push_back(d_scielIdentifier[1]);

		std::vector<unsigned char> r = getDefaultSCIELReaderCardAdapter()->sendCommand(cmd);
		EXCEPTION_ASSERT_WITH_LOG(r.size() >= 8, LibLogicalAccessException, "Bad response setting SCIEL removal time. Bad response length.");
		EXCEPTION_ASSERT_WITH_LOG(r[0] == 'O' && r[1] == 'K', LibLogicalAccessException, "Bad response setting SCIEL removal time. Bad response status.");
		EXCEPTION_ASSERT_WITH_LOG(r[2] == cmd[0] && r[3] == cmd[1], LibLogicalAccessException, "Bad response setting SCIEL removal time. Bad command response identifier.");
		EXCEPTION_ASSERT_WITH_LOG(r[6] == d_scielIdentifier[0] && r[7] == d_scielIdentifier[1], LibLogicalAccessException, "Bad response setting SCIEL removal time. Bad reader response identifier.");
	}

	unsigned char SCIELReaderUnit::getReceptionLevel()
	{
		unsigned char level = 0;

		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x42));
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(d_scielIdentifier[0]);
		cmd.push_back(d_scielIdentifier[1]);

		std::vector<unsigned char> r = getDefaultSCIELReaderCardAdapter()->sendCommand(cmd);
		EXCEPTION_ASSERT_WITH_LOG(r.size() >= 6, LibLogicalAccessException, "Bad response getting SCIEL reception level. Bad response length.");
		EXCEPTION_ASSERT_WITH_LOG(r[0] == cmd[0] && r[1] == cmd[1], LibLogicalAccessException, "Bad response getting SCIEL reception level. Bad command response identifier.");
		EXCEPTION_ASSERT_WITH_LOG(r[4] == d_scielIdentifier[0] && r[5] == d_scielIdentifier[1], LibLogicalAccessException, "Bad response getting SCIEL reception level. Bad reader response identifier.");

		level = static_cast<unsigned char>(strtoul(BufferHelper::getStdString(std::vector<unsigned char>(r.end() - 2, r.end())).c_str(), NULL, 16));

		return level;
	}

	void SCIELReaderUnit::setReceptionLevel(unsigned char level)
	{
		char tmp[64];
		sprintf(tmp, "%02x", level);
		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x43));
		cmd.push_back(static_cast<unsigned char>(tmp[0]));
		cmd.push_back(static_cast<unsigned char>(tmp[1]));
		cmd.push_back(d_scielIdentifier[0]);
		cmd.push_back(d_scielIdentifier[1]);

		std::vector<unsigned char> r = getDefaultSCIELReaderCardAdapter()->sendCommand(cmd);
		EXCEPTION_ASSERT_WITH_LOG(r.size() >= 8, LibLogicalAccessException, "Bad response setting SCIEL reception level. Bad response length.");
		EXCEPTION_ASSERT_WITH_LOG(r[0] == 'O' && r[1] == 'K', LibLogicalAccessException, "Bad response setting SCIEL reception level. Bad response status.");
		EXCEPTION_ASSERT_WITH_LOG(r[2] == cmd[0] && r[3] == cmd[1], LibLogicalAccessException, "Bad response setting SCIEL reception level. Bad command response identifier.");
		EXCEPTION_ASSERT_WITH_LOG(r[6] == d_scielIdentifier[0] && r[7] == d_scielIdentifier[1], LibLogicalAccessException, "Bad response setting SCIEL reception level. Bad reader response identifier.");
	}

	unsigned char SCIELReaderUnit::getADConvertorValue()
	{
		LOG(LogLevel::INFOS) << "Retrieving the Ambiant Noise...";
		unsigned char convertorValue = 0;

		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>(0x39));
		cmd.push_back(static_cast<unsigned char>(0x37));
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(static_cast<unsigned char>(0x30));
		cmd.push_back(d_scielIdentifier[0]);
		cmd.push_back(d_scielIdentifier[1]);

		std::vector<unsigned char> r = getDefaultSCIELReaderCardAdapter()->sendCommand(cmd);
		EXCEPTION_ASSERT_WITH_LOG(r.size() >= 6, LibLogicalAccessException, "Bad response getting SCIEL AD convertor value. Bad response length.");
		EXCEPTION_ASSERT_WITH_LOG(r[0] == cmd[0] && r[1] == cmd[1], LibLogicalAccessException, "Bad response getting SCIEL AD convertor value. Bad command response identifier.");
		EXCEPTION_ASSERT_WITH_LOG(r[4] == d_scielIdentifier[0] && r[5] == d_scielIdentifier[1], LibLogicalAccessException, "Bad response getting SCIEL AD convertor value. Bad reader response identifier.");

		convertorValue = static_cast<unsigned char>(strtoul(BufferHelper::getStdString(std::vector<unsigned char>(r.begin() + 2, r.end() - 2)).c_str(), NULL, 16));

		return convertorValue;
	}

	std::vector<unsigned char> SCIELReaderUnit::getSCIELIdentifier()
	{
		return d_scielIdentifier;
	}

	ChipPowerStatus SCIELReaderUnit::getELAPowerStatus(unsigned char flag)
	{
		ChipPowerStatus status;

		switch (flag)
		{
		case 0x00:
			status = CPS_POWER_HIGH;
			break;

		case 0x20:
			status = CPS_POWER_LOW;
			break;

		default:
			status = CPS_UNKNOWN;
			break;
		}

		return status;
	}

	boost::shared_ptr<Chip> SCIELReaderUnit::createChipFromBuffer(std::vector<unsigned char> buffer)
	{
		boost::shared_ptr<Chip> chip;
		if (buffer.size() > 2)
		{
			chip= createChip((d_card_type == "UNKNOWN") ? "GenericTag" : d_card_type);
			chip->setReceptionLevel(buffer[0]);

			LOG(LogLevel::DEBUGS) << "Creating chip from buffer " << BufferHelper::getHex(buffer) << " Len {" << buffer.size() << "}...";

			// If tag identifier is 24 bits, we can use the power status
			// [xx] [aa] [bb] [cc] [yy] = structure of 24 bits (aabbcc is the ID)
			if (buffer.size() < 6)
			{
				chip->setPowerStatus(getELAPowerStatus(buffer[1] & 0xe0));
				buffer = std::vector<unsigned char>(buffer.begin() + 1, buffer.end());
				buffer[0] &= 0xf;
			}
			else	// Otherwise (32 bits), the power status cannot be used, the whole 32 bits is the number !
			{
				chip->setPowerStatus(CPS_UNKNOWN);
				buffer = std::vector<unsigned char>(buffer.begin() + 1, buffer.end());
			}
			buffer.resize(buffer.size() - 1);
			chip->setChipIdentifier(buffer);
		}
		else
		{
			LOG(LogLevel::WARNINGS) << "Buffer too small to be valid for a chip.";
		}

		return chip;
	}
}

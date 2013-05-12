/**
 * \file KeyboardReaderUnit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Keyboard reader unit.
 */

#include "KeyboardReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "KeyboardReaderProvider.h"
#include "logicalaccess/Cards/Chip.h"
#include <boost/filesystem.hpp>


namespace logicalaccess
{
	KeyboardReaderUnit::KeyboardReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new KeyboardReaderUnitConfiguration());
		d_card_type = "UNKNOWN";
		
		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/KeyboardReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	KeyboardReaderUnit::~KeyboardReaderUnit()
	{
		disconnectFromReader();
	}

	std::string KeyboardReaderUnit::getName() const
	{
		return std::string(d_devicename);
	}

	std::string KeyboardReaderUnit::getConnectedName()
	{
		return getName();
	}

	void KeyboardReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
	}

	bool KeyboardReaderUnit::waitInsertion(unsigned int maxwait)
	{
		INFO_("Waiting insertion... max wait {%u}", maxwait);

		bool inserted = false;
		std::vector<unsigned char> createChipId;

		if (d_removalIdentifier.size() > 0)
		{
			createChipId = d_removalIdentifier;
		}
		else
		{
			createChipId = getChipInAir(maxwait);
		}

		if (createChipId.size() > 0)
		{
			d_insertedChip = ReaderUnit::createChip((d_card_type == "UNKNOWN" ? "GenericTag" : d_card_type), createChipId);
			INFO_SIMPLE_("Chip detected !");
			inserted = true;
		}
		
		return inserted;
	}

	bool KeyboardReaderUnit::waitRemoval(unsigned int maxwait)
	{
		INFO_("Waiting removal... max wait {%u}", maxwait);

		bool removed = false;
		unsigned int currentWait = 0;
		d_removalIdentifier.clear();

		// The inserted chip will stay inserted until a new identifier is read from the input device.
		if (d_insertedChip)
		{
			std::vector<unsigned char> tmpId = getChipInAir(maxwait);
			if (tmpId.size() > 0 && (tmpId != d_insertedChip->getChipIdentifier()))
			{
				INFO_SIMPLE_("Card found but not same identifier ! The previous card has been removed !");
				d_insertedChip.reset();
				d_removalIdentifier = tmpId;
				removed = true;
			}
		}

		INFO_("Returns card removed ? {%d} - function timeout expired ? {%d}", removed, (maxwait != 0 && currentWait >= maxwait));

		return removed;
	}

	std::vector<unsigned char> KeyboardReaderUnit::getChipInAir(unsigned int maxwait)
	{
		std::vector<unsigned char> ret;

		bool process = false;
		char c = 0x00;
		if (waitInputChar(c, maxwait))
		{
			do
			{
				ret.push_back(static_cast<unsigned char>(c));
				c = 0x00;
			} while (waitInputChar(c, 250));

			process = true;
		}


		// Check for prefix/suffix before any conversion

		if (process)
		{
			std::string prefix = getKeyboardConfiguration()->getPrefix();
			if (!prefix.empty())
			{
				if (ret.size() <= prefix.size())
				{
					process = false;
				}
				else
				{
					for (unsigned int i = 0; i < prefix.size() && process; ++i)
					{
						process = (prefix.c_str()[i] == ret.at(i));
					}

					if (process)
					{
						ret.erase(ret.begin(), ret.begin() + (prefix.size() - 1));
					}
				}
			}
		}

		if (process)
		{
			std::string suffix = getKeyboardConfiguration()->getSuffix();
			if (!suffix.empty())
			{
				if (ret.size() <= suffix.size())
				{
					process = false;
				}
				else
				{
					for (unsigned int i = 0; i < suffix.size() && process; ++i)
					{
						process = (suffix.c_str()[i] == ret.at(ret.size() - suffix.size() + i));
					}

					if (process)
					{
						ret.resize(ret.size() - suffix.size());
					}
				}
			}
		}

		if (process)
		{
			// Now do data conversion
			// WARNING: limited to 64-bit buffer long for now

			unsigned long long convert = 0;

			if (getKeyboardConfiguration()->getIsDecimalNumber())
			{
				char tmp[2];
				memset(tmp, 0x00, sizeof(tmp));

				size_t fact = ret.size() - 1;
				for (std::vector<unsigned char>::iterator it = ret.begin(); it != ret.end(); ++it, --fact)
				{
					tmp[0] = *it;
					unsigned long tmpi = strtoul(tmp, NULL, 10);
					convert += static_cast<unsigned long long>(tmpi * pow(10, fact));
				}
			}
			else
			{
				std::string tmp(ret.begin(), ret.end());
				convert = strtoul(tmp.c_str(), NULL, 16);
			}

			ret.clear();
			ret.push_back(static_cast<unsigned char>((convert >> 56) & 0xff));
			ret.push_back(static_cast<unsigned char>((convert >> 48) & 0xff));
			ret.push_back(static_cast<unsigned char>((convert >> 40) & 0xff));
			ret.push_back(static_cast<unsigned char>((convert >> 32) & 0xff));
			ret.push_back(static_cast<unsigned char>((convert >> 24) & 0xff));
			ret.push_back(static_cast<unsigned char>((convert >> 16) & 0xff));
			ret.push_back(static_cast<unsigned char>((convert >> 8) & 0xff));
			ret.push_back(static_cast<unsigned char>(convert & 0xff));
		}

		if (!process)
		{
			ret.clear();
		}

		return ret;
	}

	bool KeyboardReaderUnit::waitInputChar(char &c, unsigned int maxwait)
	{
		bool ret = false;

#ifdef _WINDOWS
		DWORD res = WaitForSingleObject(getKeyboardReaderProvider()->hKbdEvent, maxwait);
		if (res == WAIT_OBJECT_0)
		{
			ResetEvent(getKeyboardReaderProvider()->hKbdEvent);
			c = getKeyboardReaderProvider()->sKeyboard->enteredKeyChar;
			SetEvent(getKeyboardReaderProvider()->hKbdEventProcessed);

			ret = true;
		}
#endif

		return ret;
	}

	bool KeyboardReaderUnit::connect()
	{
		return (d_insertedChip);
	}

	void KeyboardReaderUnit::disconnect()
	{
	}
	
	boost::shared_ptr<Chip> KeyboardReaderUnit::createChip(std::string type)
	{
		return ReaderUnit::createChip(type);
	}

	boost::shared_ptr<Chip> KeyboardReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > KeyboardReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	std::string KeyboardReaderUnit::getReaderSerialNumber()
	{
		return std::string();
	}

	bool KeyboardReaderUnit::isConnected()
	{
		return (d_insertedChip);
	}

	bool KeyboardReaderUnit::connectToReader()
	{
		bool ret = false;

#ifdef _WINDOWS
		// Just make a filter on input devices
		if (getKeyboardReaderProvider()->sKeyboard != NULL)
		{
			std::strcpy(getKeyboardReaderProvider()->sKeyboard->selectedDeviceName, getName().c_str());
			std::strcpy(getKeyboardReaderProvider()->sKeyboard->keyboardLayout, getKeyboardConfiguration()->getKeyboardLayout().c_str());
			ret = true;
		}
#endif

		return ret;
	}

	void KeyboardReaderUnit::disconnectFromReader()
	{
#ifdef _WINDOWS
		// Don't really disconnect or close any listening, but remove the device filter if any.
		if (getKeyboardReaderProvider()->sKeyboard != NULL)
		{
			memset(getKeyboardReaderProvider()->sKeyboard->selectedDeviceName, 0x00, DEVICE_NAME_MAXLENGTH);
		}
#endif
	}

	void KeyboardReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		
		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		node.put("DeviceName", d_devicename);
		d_readerUnitConfig->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void KeyboardReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		INFO_SIMPLE_("Unserialize Keyboard reader unit...");

		d_devicename = node.get_child("DeviceName").get_value<std::string>();
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
	}

	boost::shared_ptr<KeyboardReaderProvider> KeyboardReaderUnit::getKeyboardReaderProvider() const
	{
		return boost::dynamic_pointer_cast<KeyboardReaderProvider>(getReaderProvider());
	}

	void KeyboardReaderUnit::setKeyboard(const std::string& devicename)
	{
		d_devicename = devicename;
	}
}

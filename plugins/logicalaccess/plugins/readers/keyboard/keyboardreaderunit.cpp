/**
 * \file keyboardreaderunit.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Keyboard reader unit.
 */

#include <logicalaccess/plugins/readers/keyboard/keyboardreaderunit.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/readers/keyboard/keyboardreaderprovider.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <boost/filesystem.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace logicalaccess
{
KeyboardReaderUnit::KeyboardReaderUnit()
    : ReaderUnit(READER_KEYBOARD)
    , d_vendorId(0)
    , d_productId(0)
{
    d_readerUnitConfig.reset(new KeyboardReaderUnitConfiguration());
    d_card_type         = CHIP_UNKNOWN;
    d_instanceConnected = false;

    try
    {
        boost::property_tree::ptree pt;
        read_xml(
            (boost::filesystem::current_path().string() + "/KeyboardReaderUnit.config"),
            pt);
        d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
    }
    catch (...)
    {
    }
}

KeyboardReaderUnit::~KeyboardReaderUnit()
{
    KeyboardReaderUnit::disconnectFromReader();
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
    LOG(LogLevel::INFOS) << "Waiting insertion... max wait {" << maxwait << "}";

    bool inserted = false;
    ByteVector createChipId;

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
        d_insertedChip = ReaderUnit::createChip(
            (d_card_type == CHIP_UNKNOWN ? CHIP_GENERICTAG : d_card_type), createChipId);
        LOG(LogLevel::INFOS) << "Chip detected !";
        inserted = true;
    }

    return inserted;
}

bool KeyboardReaderUnit::waitRemoval(unsigned int maxwait)
{
    LOG(LogLevel::INFOS) << "Waiting removal... max wait {" << maxwait << "}";

    bool removed = false;
    d_removalIdentifier.clear();

    // The inserted chip will stay inserted until a new identifier is read from the input
    // device.
    if (d_insertedChip)
    {
        ByteVector tmpId = getChipInAir(maxwait);
        if (tmpId.size() > 0 && (tmpId != d_insertedChip->getChipIdentifier()))
        {
            LOG(LogLevel::INFOS) << "Card found AND not same identifier as previous ! "
                                    "The previous card has been removed !";
            d_insertedChip.reset();
            d_removalIdentifier = tmpId;
            removed             = true;
        }
        else if (tmpId.size() > 0 && tmpId == d_insertedChip->getChipIdentifier())
        {
            LOG(LogLevel::INFOS)
                << "Card found but this is the same identifier as previous. Ignoring...";
        }
    }

    LOG(LogLevel::INFOS) << "Returns card removed ? {" << removed
                         << "} - function timeout expired ? " << (maxwait != 0);

    return removed;
}

ByteVector KeyboardReaderUnit::getChipInAir(unsigned int maxwait)
{
    ByteVector ret;

    bool process = false;
    char c       = 0x00;
    if (waitInputChar(c, maxwait))
    {
        do
        {
            ret.push_back(static_cast<unsigned char>(c));
            c = 0x00;
        } while (waitInputChar(c, 250));

        LOG(LogLevel::INFOS) << "Keyboard chars received successfully {"
                             << BufferHelper::getHex(ret) << "}-{"
                             << BufferHelper::getStdString(ret) << "}! Processing...";

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
                    process =
                        (suffix.c_str()[i] == ret.at(ret.size() - suffix.size() + i));
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
            for (ByteVector::iterator it = ret.begin(); it != ret.end(); ++it, --fact)
            {
                tmp[0]             = *it;
                unsigned long tmpi = strtoul(tmp, nullptr, 10);
                convert += static_cast<unsigned long long>(tmpi * pow(10, fact));
            }
        }
        else
        {
            std::string tmp(ret.begin(), ret.end());
            convert = strtoul(tmp.c_str(), nullptr, 16);
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

bool KeyboardReaderUnit::waitInputChar(char &c, unsigned int maxwait) const
{
    bool ret = false;

#ifdef _WINDOWS
    DWORD res = WaitForSingleObject(getKeyboardReaderProvider()->hKbdEvent,
                                    (maxwait == 0) ? INFINITE : maxwait);
    if (res == WAIT_OBJECT_0)
    {
        LOG(LogLevel::INFOS) << "Keyboard event detected!";

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
    return (bool)d_insertedChip;
}

void KeyboardReaderUnit::disconnect()
{
}

std::shared_ptr<Chip> KeyboardReaderUnit::createChip(std::string type)
{
    return ReaderUnit::createChip(type);
}

std::shared_ptr<Chip> KeyboardReaderUnit::getSingleChip()
{
    std::shared_ptr<Chip> chip = d_insertedChip;
    return chip;
}

std::vector<std::shared_ptr<Chip>> KeyboardReaderUnit::getChipList()
{
    std::vector<std::shared_ptr<Chip>> chipList;
    std::shared_ptr<Chip> singleChip = getSingleChip();
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
    return (bool)d_insertedChip;
}

bool KeyboardReaderUnit::connectToReader()
{
    LOG(LogLevel::INFOS) << "Connecting to reader...";
    bool ret = false;

#ifdef _WINDOWS
    // Just make a filter on input devices
    if (getKeyboardReaderProvider()->sKeyboard != nullptr)
    {
        strcpy(getKeyboardReaderProvider()->sKeyboard->selectedDeviceName,
               getName().c_str());
        strcpy(getKeyboardReaderProvider()->sKeyboard->keyboardLayout,
               getKeyboardConfiguration()->getKeyboardLayout().c_str());
        ret = true;
    }
#endif

    d_instanceConnected = ret;
    return ret;
}

void KeyboardReaderUnit::disconnectFromReader()
{
    LOG(LogLevel::INFOS) << "Disconnecting from reader...";
#ifdef _WINDOWS
    if (d_instanceConnected)
    {
        // Don't really disconnect or close any listening, but remove the device filter if
        // any.
        if (getKeyboardReaderProvider()->sKeyboard != nullptr)
        {
            memset(getKeyboardReaderProvider()->sKeyboard->selectedDeviceName, 0x00,
                   DEVICE_NAME_MAXLENGTH);
        }
        d_instanceConnected = false;
    }
#endif
}

void KeyboardReaderUnit::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getReaderProvider()->getRPType());
    node.put("DeviceName", d_devicename);
    d_readerUnitConfig->serialize(node);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void KeyboardReaderUnit::unSerialize(boost::property_tree::ptree &node)
{
    LOG(LogLevel::INFOS) << "Unserialize Keyboard reader unit...";

    d_devicename = node.get_child("DeviceName").get_value<std::string>();
    d_readerUnitConfig->unSerialize(
        node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
}

std::shared_ptr<KeyboardReaderProvider>
KeyboardReaderUnit::getKeyboardReaderProvider() const
{
    return std::dynamic_pointer_cast<KeyboardReaderProvider>(getReaderProvider());
}

void KeyboardReaderUnit::setKeyboard(const std::string &devicename, unsigned int vendorId,
                                     unsigned int productId)
{
    d_devicename = devicename;
    d_vendorId   = vendorId;
    d_productId  = productId;
}

unsigned int KeyboardReaderUnit::getVendorId() const
{
    return d_vendorId;
}

unsigned int KeyboardReaderUnit::getProductId() const
{
    return d_productId;
}
}
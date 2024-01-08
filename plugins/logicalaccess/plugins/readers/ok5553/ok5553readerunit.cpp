/**
 * \file ok5553readerunit.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief OK5553 reader unit.
 */

#include <logicalaccess/plugins/readers/ok5553/ok5553readerunit.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>

#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/readers/ok5553/ok5553readerprovider.hpp>
#include <logicalaccess/services/accesscontrol/cardsformatcomposite.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/readers/ok5553/readercardadapters/ok5553readercardadapter.hpp>
#include <logicalaccess/plugins/readers/ok5553/readercardadapters/iso7816ok5553readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireev1iso7816commands.hpp>
#include <logicalaccess/plugins/readers/ok5553/commands/mifareok5553commands.hpp>
#include <logicalaccess/plugins/readers/ok5553/commands/mifareultralightok5553commands.hpp>
#include <logicalaccess/plugins/cards/mifare/mifare1kchip.hpp>
#include <logicalaccess/plugins/cards/mifare/mifare4kchip.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightchip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <logicalaccess/readerproviders/serialportdatatransport.hpp>

namespace logicalaccess
{
OK5553ReaderUnit::OK5553ReaderUnit()
    : ReaderUnit(READER_OK5553)
{
    d_readerUnitConfig.reset(new OK5553ReaderUnitConfiguration());
    ReaderUnit::setDefaultReaderCardAdapter(std::make_shared<OK5553ReaderCardAdapter>());

    std::shared_ptr<SerialPortDataTransport> dataTransport(new SerialPortDataTransport());
    ReaderUnit::setDataTransport(dataTransport);
    d_card_type = CHIP_UNKNOWN;

    try
    {
        boost::property_tree::ptree pt;
        read_xml(
            (boost::filesystem::current_path().string() + "/OK5553ReaderUnit.config"),
            pt);
        d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
    }
    catch (...)
    {
    }
}

OK5553ReaderUnit::~OK5553ReaderUnit()
{
    OK5553ReaderUnit::disconnectFromReader();
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
        d_insertedChip =
            createChip((d_card_type == CHIP_UNKNOWN) ? CHIP_GENERICTAG : d_card_type);
        d_insertedChip->setChipIdentifier(removalIdentifier);
        removalIdentifier.clear();
        inserted = true;
    }
    else
    {
        std::shared_ptr<Chip> chip = getChipInAir(maxwait);
        if (chip)
        {
            d_insertedChip = chip;
            inserted       = true;
        }
    }

    return inserted;
}

bool OK5553ReaderUnit::waitRemoval(unsigned int maxwait)
{
    bool removed = false;
    removalIdentifier.clear();
    if (d_insertedChip)
    {
        unsigned int currentWait = 0;
        while (!removed && ((currentWait < maxwait) || maxwait == 0))
        {
            std::shared_ptr<Chip> chip = getChipInAir(250);
            if (chip)
            {
                ByteVector tmpId = chip->getChipIdentifier();
                if (tmpId != d_insertedChip->getChipIdentifier())
                {
                    d_insertedChip.reset();
                    removalIdentifier = tmpId;
                    removed           = true;
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    LOG(LogLevel::INFOS) << "Starting connect...";
    if (getSingleChip())
    {
        if (getSingleChip()->getCardType() == "DESFire")
        {
            ByteVector tmp = rats();
            if (tmp.size() == 0)
            {
                d_insertedChip = getChipInAir();
                tmp            = rats();
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

    LOG(LogLevel::INFOS) << "Disconnected from the chip";
}

ByteVector OK5553ReaderUnit::asciiToHex(const ByteVector &source)
{
    ByteVector res;
    if (source.size() > 1)
    {
        char tmp[3];
        for (size_t i = 0; i <= source.size() - 2; i += 2)
        {
            tmp[0] = source[i];
            tmp[1] = source[i + 1];
            tmp[2] = '\0';
            res.push_back(static_cast<unsigned char>(strtoul(tmp, nullptr, 16)));
        }
    }
    else
    {
        res = source;
    }
    return res;
}

std::shared_ptr<Chip> OK5553ReaderUnit::getChipInAir(unsigned int maxwait)
{
    LOG(LogLevel::INFOS) << "Starting get chip in air...";

    std::shared_ptr<Chip> chip;
    ByteVector buf;
    unsigned int currentWait = 0;
    while (!chip && (maxwait == 0 || currentWait < maxwait))
    {
        try
        {
            buf = getDefaultOK5553ReaderCardAdapter()->sendAsciiCommand("s");
        }
        catch (std::exception &)
        {
            buf.clear();
        }
        d_successedRATS.clear();
        if (buf.size() > 0)
        {
            buf = asciiToHex(buf);
            if (buf[0] == MIFARE)
            {
                chip = createChip("Mifare");
                buf.erase(buf.begin());
                chip->setChipIdentifier(buf);
            }
            else if (buf[0] == DESFIRE)
            {
                chip = createChip("DESFire");
                buf.erase(buf.begin());
                chip->setChipIdentifier(buf);
                std::dynamic_pointer_cast<DESFireChip>(chip)
                    ->getCrypto()
                    ->setCryptoContext(chip->getChipIdentifier());
            }
            else if (buf[0] == MIFAREULTRALIGHT)
            {
                chip = createChip("MifareUltralight");
                buf.erase(buf.begin());
                chip->setChipIdentifier(buf);
            }
        }
        if (!chip)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            currentWait += 250;
        }
    }

    return chip;
}

std::shared_ptr<Chip> OK5553ReaderUnit::createChip(std::string type)
{
    LOG(LogLevel::INFOS) << "Create chip " << type;
    std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

    if (chip)
    {
        std::shared_ptr<ReaderCardAdapter> rca;
        std::shared_ptr<Commands> commands;

        if (type == CHIP_MIFARE1K || type == CHIP_MIFARE4K || type == CHIP_MIFARE)
        {
            commands.reset(new MifareOK5553Commands);
            rca = getDefaultReaderCardAdapter();
        }
        else if (type == CHIP_MIFAREULTRALIGHT)
        {
            commands.reset(new MifareUltralightOK5553Commands());
            rca = getDefaultReaderCardAdapter();
        }
        else if (type == CHIP_GENERICTAG)
        {
            rca = getDefaultReaderCardAdapter();
        }
        else if (type == CHIP_DESFIRE)
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

std::shared_ptr<Chip> OK5553ReaderUnit::getSingleChip()
{
    std::shared_ptr<Chip> chip = d_insertedChip;
    return chip;
}

std::vector<std::shared_ptr<Chip>> OK5553ReaderUnit::getChipList()
{
    std::vector<std::shared_ptr<Chip>> chipList;
    std::shared_ptr<Chip> singleChip = getSingleChip();
    if (singleChip)
    {
        chipList.push_back(singleChip);
    }
    return chipList;
}

std::shared_ptr<OK5553ReaderCardAdapter>
OK5553ReaderUnit::getDefaultOK5553ReaderCardAdapter()
{
    std::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
    if (!adapter->getDataTransport())
        adapter->setDataTransport(getDataTransport());
    return std::dynamic_pointer_cast<OK5553ReaderCardAdapter>(adapter);
}

std::string OK5553ReaderUnit::getReaderSerialNumber()
{
    return std::string();
}

bool OK5553ReaderUnit::isConnected()
{
    return bool(d_insertedChip);
}

bool OK5553ReaderUnit::connectToReader()
{
    LOG(LogLevel::INFOS) << "Starting connection to reader...";
    std::shared_ptr<DataTransport> dataTransport = getDataTransport();
    if (!dataTransport->getReaderUnit())
    {
        dataTransport->setReaderUnit(shared_from_this());
        setDataTransport(dataTransport);
    }

    return getDataTransport()->connect();
}

void OK5553ReaderUnit::disconnectFromReader()
{
    LOG(LogLevel::INFOS) << "Starting disconnection to reader...";
    getDataTransport()->disconnect();
}

void OK5553ReaderUnit::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    ReaderUnit::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void OK5553ReaderUnit::unSerialize(boost::property_tree::ptree &node)
{
    ReaderUnit::unSerialize(node);
}

std::shared_ptr<OK5553ReaderProvider> OK5553ReaderUnit::getOK5553ReaderProvider() const
{
    return std::dynamic_pointer_cast<OK5553ReaderProvider>(getReaderProvider());
}

ByteVector OK5553ReaderUnit::reqA()
{
    ByteVector answer = getDefaultOK5553ReaderCardAdapter()->sendAsciiCommand("t01E326");
    answer            = asciiToHex(answer);
    if (answer.size() > 1)
    {
        if (answer[0] == answer.size() - 1)
        {
            answer.erase(answer.begin());
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

ByteVector OK5553ReaderUnit::rats()
{
    ByteVector answer;

    // Sending two RATS is not supported without a new Select. Doesn't send another one if
    // the first successed.
    if (d_successedRATS.size() == 0)
    {
        LOG(LogLevel::INFOS) << "Sending a RATS";
        answer = getDefaultOK5553ReaderCardAdapter()->sendAsciiCommand("t020FE020");
        answer = asciiToHex(answer);
        if (answer.size() > 1)
        {
            if (answer[0] == answer.size() - 1)
            {
                answer.erase(answer.begin());
            }
            else
                answer.clear();
        }
        else
        {
            answer.clear();
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument,
                                     "No tag present in rfid field");
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
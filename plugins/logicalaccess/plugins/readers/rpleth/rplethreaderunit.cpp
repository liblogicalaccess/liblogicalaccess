/**
 * \file rplethreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth reader unit.
 */

#include <logicalaccess/plugins/readers/rpleth/rplethreaderunit.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>

#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/readers/rpleth/rplethreaderprovider.hpp>
#include <logicalaccess/services/accesscontrol/cardsformatcomposite.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/readerproviders/tcpdatatransport.hpp>
#include <logicalaccess/plugins/readers/rpleth/rplethledbuzzerdisplay.hpp>
#include <logicalaccess/plugins/readers/rpleth/rplethlcddisplay.hpp>
#include <logicalaccess/plugins/readers/rpleth/rplethdatatransport.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireiso7816commands.hpp>
#include <logicalaccess/plugins/readers/pcsc/pcscreaderunit.hpp>

#include <logicalaccess/readerproviders/serialportxml.hpp>
#include <logicalaccess/plugins/readers/rpleth/rplethreaderunitconfiguration.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace logicalaccess
{
RplethReaderUnit::RplethReaderUnit()
    : ISO7816ReaderUnit(READER_RPLETH)
{
    d_readerUnitConfig.reset(new RplethReaderUnitConfiguration());
    ReaderUnit::setDefaultReaderCardAdapter(std::make_shared<RplethReaderCardAdapter>());

    std::shared_ptr<RplethDataTransport> dataTransport(new RplethDataTransport());
    dataTransport->setIpAddress("192.168.1.100");
    dataTransport->setPort(23);
    ReaderUnit::setDataTransport(dataTransport);
    d_card_type = CHIP_UNKNOWN;
    d_lcdDisplay.reset(new RplethLCDDisplay());
    d_ledBuzzerDisplay.reset(new RplethLEDBuzzerDisplay());

    try
    {
        boost::property_tree::ptree pt;
        read_xml(
            (boost::filesystem::current_path().string() + "/RplethReaderUnit.config"),
            pt);
        d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
    }
    catch (...)
    {
    }
}

RplethReaderUnit::~RplethReaderUnit()
{
    RplethReaderUnit::disconnectFromReader();
}

std::string RplethReaderUnit::getName() const
{
    return std::string();
}

std::string RplethReaderUnit::getConnectedName()
{
    return getName();
}

void RplethReaderUnit::setCardType(std::string cardType)
{
    d_card_type = cardType;
    if (getDefaultRplethReaderCardAdapter()->getDataTransport()->isConnected())
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(HID));
        command.push_back(static_cast<unsigned char>(SET_CARDTYPE));
        command.push_back(static_cast<unsigned char>(cardType.size()));
        command.insert(command.end(), cardType.begin(), cardType.end());
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }
}

bool RplethReaderUnit::waitInsertion(unsigned int maxwait)
{
    bool inserted = false;
    if (getRplethConfiguration()->getMode() == PROXY)
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(HID));
        command.push_back(static_cast<unsigned char>(WAIT_INSERTION));
        command.push_back(static_cast<unsigned char>(0x04));
        BufferHelper::setUInt32(command, maxwait);
        ByteVector answer;
        try
        {
            auto rpleth_maxwait = maxwait + Settings::getInstance()->DataTransportTimeout;
            answer              = getDefaultRplethReaderCardAdapter()->sendRplethCommand(
                command, true, rpleth_maxwait);
        }
        catch (LibLogicalAccessException &)
        {
            answer.clear();
        }

        if (answer.size() > 0)
        {
            unsigned char ctypelen = answer[0];
            EXCEPTION_ASSERT_WITH_LOG(
                answer.size() >= static_cast<size_t>(1 + ctypelen + 1),
                LibLogicalAccessException, "Wrong card type length.");
            std::string ctype =
                (d_card_type == CHIP_UNKNOWN)
                    ? std::string(answer.begin() + 1, answer.begin() + 1 + ctypelen)
                    : d_card_type;
            unsigned char csnlen = answer[1 + ctypelen];
            EXCEPTION_ASSERT_WITH_LOG(answer.size() >=
                                          static_cast<size_t>(1 + ctypelen + 1 + csnlen),
                                      LibLogicalAccessException, "Wrong csn length.");
            ByteVector csn = ByteVector(answer.begin() + 1 + ctypelen + 1, answer.end());

            d_insertedChip = d_proxyReader->createChip(ctype, csn);
            inserted       = bool(d_insertedChip);

            if (inserted)
            {
                if (ctype == CHIP_DESFIRE_EV1 || ctype == CHIP_DESFIRE)
                {
                    std::dynamic_pointer_cast<DESFireISO7816Commands>(
                        d_insertedChip->getCommands())
                        ->setSAMChip(getSAMChip());
                }
            }
        }
    }
    else
    {
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
    }

    return inserted;
}

void RplethReaderUnit::sendCardWaited(ByteVector card)
{
    ByteVector command, tmp;
    command.push_back(static_cast<unsigned char>(HID));
    command.push_back(static_cast<unsigned char>(SEND_CARDS));

    std::string card_string = BufferHelper::getHex(card);
    tmp.insert(tmp.end(), card_string.begin(), card_string.end());

    command.push_back(static_cast<unsigned char>(tmp.size()));
    command.insert(command.end(), tmp.begin(), tmp.end());
    getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, true);
}

ByteVector RplethReaderUnit::receiveCardWaited(bool present)
{
    ByteVector command;

    command.push_back(static_cast<unsigned char>(HID));
    command.push_back(static_cast<unsigned char>(RECEIVE_UNPRESENTED_CARDS));
    command.push_back(static_cast<unsigned char>(0x01));
    command.push_back(static_cast<unsigned char>(present));
    ByteVector answer =
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, true);

    return BufferHelper::fromHexString(std::string(answer.begin(), answer.end()));
}

bool RplethReaderUnit::waitRemoval(unsigned int maxwait)
{
    bool removed = false;
    if (d_insertedChip)
    {
        if (getRplethConfiguration()->getMode() == PROXY)
        {
            ByteVector command;
            command.push_back(static_cast<unsigned char>(HID));
            command.push_back(static_cast<unsigned char>(WAIT_REMOVAL));
            command.push_back(static_cast<unsigned char>(0x04));
            BufferHelper::setUInt32(command, maxwait);
            try
            {
                auto rpleth_maxwait =
                    maxwait + Settings::getInstance()->DataTransportTimeout;
                getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, true,
                                                                       rpleth_maxwait);
                d_insertedChip.reset();
                LOG(LogLevel::INFOS) << "Card removed";
                removed = true;
            }
            catch (LibLogicalAccessException &)
            {
            }
        }
        else
        {
            unsigned int currentWait = 0;
            removalIdentifier.clear();

            while (!removed && ((currentWait < maxwait) || maxwait == 0))
            {
                std::shared_ptr<Chip> chip;
                try
                {
                    chip = getChipInAir(250);
                }
                catch (LibLogicalAccessException &)
                {
                }
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
                        std::this_thread::sleep_for(std::chrono::milliseconds(250));
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
    }

    return removed;
}

bool RplethReaderUnit::connect()
{
    if (getRplethConfiguration()->getMode() == PROXY)
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(HID));
        command.push_back(static_cast<unsigned char>(CONNECT));
        command.push_back(static_cast<unsigned char>(0x00));
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, true);

        if (d_insertedChip->getChipIdentifier().size() == 0)
        {
            d_insertedChip->setChipIdentifier(getInsertedChipIdentifier());
        }

        if (d_insertedChip->getGenericCardType() == "DESFire")
            std::dynamic_pointer_cast<DESFireChip>(d_insertedChip)
                ->getCrypto()
                ->setCryptoContext(d_insertedChip->getChipIdentifier());
    }

    return true;
}

void RplethReaderUnit::disconnect()
{
    if (getRplethConfiguration()->getMode() == PROXY)
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(HID));
        command.push_back(static_cast<unsigned char>(DISCONNECT));
        command.push_back(static_cast<unsigned char>(0x00));
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, true);
    }

    LOG(LogLevel::INFOS) << "Disconnected from the chip";
}

ByteVector RplethReaderUnit::getInsertedChipIdentifier()
{
    ByteVector csn;
    if (getRplethConfiguration()->getMode() == PROXY)
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(HID));
        command.push_back(static_cast<unsigned char>(GET_CSN));
        command.push_back(static_cast<unsigned char>(0x00));
        csn = getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, true);
    }

    LOG(LogLevel::INFOS) << "Inserted chip identifier get";
    return csn;
}

std::shared_ptr<Chip> RplethReaderUnit::getChipInAir(unsigned int maxwait)
{
    LOG(LogLevel::INFOS) << "Starting get chip in air...";

    std::shared_ptr<Chip> chip;
    ByteVector buf = receiveBadge(maxwait);

    if (buf.size() > 0)
    {
        chip = createChip((d_card_type == CHIP_UNKNOWN) ? CHIP_GENERICTAG : d_card_type);
        chip->setChipIdentifier(buf);
    }

    return chip;
}

std::shared_ptr<Chip> RplethReaderUnit::createChip(std::string type)
{
    LOG(LogLevel::INFOS) << "Create chip " << type;
    std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

    return chip;
}

std::shared_ptr<Chip> RplethReaderUnit::getSingleChip()
{
    std::shared_ptr<Chip> chip = d_insertedChip;
    return chip;
}

std::vector<std::shared_ptr<Chip>> RplethReaderUnit::getChipList()
{
    std::vector<std::shared_ptr<Chip>> chipList;
    std::shared_ptr<Chip> singleChip = getSingleChip();
    if (singleChip)
    {
        chipList.push_back(singleChip);
    }
    return chipList;
}

std::shared_ptr<RplethReaderCardAdapter>
RplethReaderUnit::getDefaultRplethReaderCardAdapter()
{
    std::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
    if (adapter)
    {
        if (!adapter->getDataTransport())
        {
            adapter->setDataTransport(getDataTransport());
        }
    }
    return std::dynamic_pointer_cast<RplethReaderCardAdapter>(adapter);
}

std::string RplethReaderUnit::getReaderSerialNumber()
{
    return "";
}

bool RplethReaderUnit::isConnected()
{
    return bool(d_insertedChip);
}

bool RplethReaderUnit::connectToReader()
{
    LOG(LogLevel::INFOS) << "Starting connection to reader...";
    std::shared_ptr<DataTransport> dataTransport = getDataTransport();
    if (!dataTransport->getReaderUnit())
    {
        dataTransport->setReaderUnit(shared_from_this());
        setDataTransport(dataTransport);
    }

    bool connected = true;
    if (!getDataTransport()->isConnected())
    {
        connected = getDataTransport()->connect();
        if (connected && getRplethConfiguration()->getMode() == RplethMode::PROXY)
        {
            LOG(LogLevel::INFOS)
                << "Data transport connected, initializing PROXY mode...";
            std::string type = getProxyReaderType();

            if (type == "PCSC") // Avoid scard resources
            {
                LOG(LogLevel::INFOS) << "default PROXY PCSC ...";
                d_proxyReader = std::make_shared<PCSCReaderUnit>("");
            }
            else
            {
                std::shared_ptr<ReaderProvider> rp =
                    LibraryManager::getInstance()->getReaderProvider(type);
                if (rp)
                {
                    d_proxyReader = rp->createReaderUnit();
                }
            }

            if (d_proxyReader)
            {
                std::shared_ptr<RplethDataTransport> rpdt =
                    std::dynamic_pointer_cast<RplethDataTransport>(getDataTransport());

                EXCEPTION_ASSERT_WITH_LOG(
                    rpdt, LibLogicalAccessException,
                    "Rpleth data transport required for proxy mode.");
                d_proxyReader->setDataTransport(rpdt);
            }
        }
    }

    if (connected)
    {
        connected = ISO7816ReaderUnit::connectToReader();
    }

    return connected;
}

void RplethReaderUnit::disconnectFromReader()
{
    LOG(LogLevel::INFOS) << "Starting disconnection to reader...";

    ISO7816ReaderUnit::disconnectFromReader();
    getDataTransport()->disconnect();
}

void RplethReaderUnit::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    ReaderUnit::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void RplethReaderUnit::unSerialize(boost::property_tree::ptree &node)
{
    ReaderUnit::unSerialize(node);
}

std::shared_ptr<RplethReaderProvider> RplethReaderUnit::getRplethReaderProvider() const
{
    return std::dynamic_pointer_cast<RplethReaderProvider>(getReaderProvider());
}

bool RplethReaderUnit::getDhcpState()
{
    bool res = false;
    ByteVector command;
    command.push_back(static_cast<unsigned char>(RPLETH));
    command.push_back(static_cast<unsigned char>(STATEDHCP));
    command.push_back(static_cast<unsigned char>(0x00));
    ByteVector answer =
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, true);
    if (answer.size() > 0)
        res = answer[0] != 0x00;
    return res;
}

void RplethReaderUnit::setDhcpState(bool status)
{
    ByteVector command;
    command.push_back(static_cast<unsigned char>(RPLETH));
    command.push_back(static_cast<unsigned char>(DHCP));
    command.push_back(static_cast<unsigned char>(0x01));
    command.push_back(static_cast<unsigned char>(status));
    getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
}

void RplethReaderUnit::setReaderIp(const ByteVector &address)
{
    if (address.size() == 4)
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(RPLETH));
        command.push_back(static_cast<unsigned char>(IP));
        command.push_back(static_cast<unsigned char>(0x04));
        for (int i = 0; i < 4; i++)
        {
            command.push_back(static_cast<unsigned char>(address[i]));
        }
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }
}

void RplethReaderUnit::setReaderMac(const ByteVector &address)
{
    if (address.size() == 6)
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(RPLETH));
        command.push_back(static_cast<unsigned char>(MAC));
        command.push_back(static_cast<unsigned char>(0x06));
        for (int i = 0; i < 6; i++)
        {
            command.push_back(static_cast<unsigned char>(address[i]));
        }
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }
}

void RplethReaderUnit::setReaderSubnet(const ByteVector &address)
{
    if (address.size() == 4)
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(RPLETH));
        command.push_back(static_cast<unsigned char>(SUBNET));
        command.push_back(static_cast<unsigned char>(0x04));
        for (int i = 0; i < 4; i++)
        {
            command.push_back(static_cast<unsigned char>(address[i]));
        }
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }
}

void RplethReaderUnit::setReaderGateway(const ByteVector &address)
{
    if (address.size() == 4)
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(RPLETH));
        command.push_back(static_cast<unsigned char>(GATEWAY));
        command.push_back(static_cast<unsigned char>(0x04));
        for (int i = 0; i < 4; i++)
        {
            command.push_back(static_cast<unsigned char>(address[i]));
        }
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }
}

void RplethReaderUnit::setReaderPort(int port)
{
    ByteVector command;
    command.push_back(static_cast<unsigned char>(RPLETH));
    command.push_back(static_cast<unsigned char>(PORT));
    command.push_back(static_cast<unsigned char>(0x02));
    command.push_back(static_cast<unsigned char>(port >> 8));
    command.push_back(static_cast<unsigned char>(port & 0xff));
    getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
}

void RplethReaderUnit::resetReader()
{
    ByteVector command;
    command.push_back(static_cast<unsigned char>(RPLETH));
    command.push_back(static_cast<unsigned char>(RESET));
    command.push_back(static_cast<unsigned char>(0x00));
    getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
}

void RplethReaderUnit::setReaderMessage(const std::string &message)
{
    ByteVector command;
    command.push_back(static_cast<unsigned char>(RPLETH));
    command.push_back(static_cast<unsigned char>(MESSAGE));
    command.push_back(static_cast<unsigned char>(message.size()));
    command.insert(command.end(), message.begin(), message.end());
    getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
}

void RplethReaderUnit::nop()
{
    ByteVector command;
    command.push_back(static_cast<unsigned char>(HID));
    command.push_back(static_cast<unsigned char>(NOP));
    command.push_back(static_cast<unsigned char>(0x00));
    getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
}

void RplethReaderUnit::setContext(const std::string &context)
{
    ByteVector command;
    command.push_back(static_cast<unsigned char>(RPLETH));
    command.push_back(static_cast<unsigned char>(SET_CONTEXT));
    command.push_back(static_cast<unsigned char>(context.size()));
    command.insert(command.end(), context.begin(), context.end());
    getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
}

std::string RplethReaderUnit::getContext()
{
    std::string context;
    ByteVector command;
    command.push_back(static_cast<unsigned char>(RPLETH));
    command.push_back(static_cast<unsigned char>(GET_CONTEXT));
    command.push_back(0x00);
    ByteVector answer =
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, true);
    if (answer.size() > 0)
    {
        context = std::string(answer.begin(), answer.end());
    }
    return context;
}

ByteVector RplethReaderUnit::badge(long int timeout)
{
    ByteVector command;
    ByteVector res;
    command.push_back(static_cast<unsigned char>(HID));
    command.push_back(static_cast<unsigned char>(BADGE));
    command.push_back(static_cast<unsigned char>(0x00));
    try
    {
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, false);
        res = receiveBadge(timeout);
    }
    catch (std::invalid_argument &e)
    {
        std::cout << e.what() << std::endl;
    }
    return res;
}

ByteVector RplethReaderUnit::receiveBadge(long int timeout)
{
    LOG(LogLevel::COMS) << "receiveBadge";
    ByteVector res;
    ByteVector cmd;

    try
    {
        std::shared_ptr<RplethDataTransport> dt =
            std::dynamic_pointer_cast<RplethDataTransport>(
                getDefaultRplethReaderCardAdapter()->getDataTransport());

        if (dt)
        {
            std::list<ByteVector> badges = dt->getBadges();

            if (badges.empty())
            {
                getDefaultRplethReaderCardAdapter()->sendRplethCommand(cmd, false,
                                                                       timeout);
                // We have the ping - client is here
            }

            try
            {
                getDefaultRplethReaderCardAdapter()->sendRplethCommand(cmd, true,
                                                                       timeout);
            }
            catch (const std::exception &)
            { // We dont care about timeout
            }

            if (!badges.empty())
            {
                res = badges.front();
                badges.pop_front();
            }
        }
        // res contains full wiegand trame, it need to take the csn
        if (res.size() > 0)
            res = getCsn(res);
    }
    catch (std::invalid_argument &)
    {
    }
    return res;
}

ByteVector RplethReaderUnit::getCsn(const ByteVector &trame) const
{
    ByteVector result;
    std::shared_ptr<RplethReaderUnitConfiguration> conf =
        std::dynamic_pointer_cast<RplethReaderUnitConfiguration>(d_readerUnitConfig);
    if (conf && conf->getLength() != 0)
    {
        if (trame.size() * 8 >=
            static_cast<size_t>(conf->getLength() + conf->getOffset()))
        {
            long long tmp = 0;
            for (int i = 0; i < static_cast<int>(trame.size()); i++)
            {
                tmp <<= 8;
                tmp |= trame[i];
            }
            tmp >>= conf->getOffset();
            tmp &= static_cast<int>(pow(2, conf->getLength()) - 1);
            int size = 0;
            if (conf->getLength() % 8 == 0)
                size = conf->getLength() / 8;
            else
                size = conf->getLength() / 8 + 1;
            for (int i = 0, j = 0; i < size; i++, j += 8)
            {
                result.insert(result.begin(), (tmp >> j) & 0xff);
            }
        }
        else
        {
            LOG(LogLevel::WARNINGS) << "Wrong trame length (" << trame.size() * 8
                                    << " < (" << conf->getLength() << " + "
                                    << conf->getOffset() << "))";
        }
    }
    else
    {
        result.insert(result.begin(), trame.begin(), trame.end());
    }

    return result;
}

std::string RplethReaderUnit::getProxyReaderType()
{
    std::string rptype = "";
    ByteVector command;
    command.push_back(static_cast<unsigned char>(HID));
    command.push_back(static_cast<unsigned char>(GET_READERTYPE));
    command.push_back(static_cast<unsigned char>(0x00));
    ByteVector answer =
        getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, true);
    if (answer.size() > 0)
    {
        rptype = std::string(answer.begin(), answer.end());
    }

    LOG(LogLevel::COMS) << "getProxyReaderType returns " << rptype;

    return rptype;
}

std::shared_ptr<RplethReaderUnitConfiguration> RplethReaderUnit::getRplethConfiguration()
{
    return std::dynamic_pointer_cast<RplethReaderUnitConfiguration>(getConfiguration());
}
}
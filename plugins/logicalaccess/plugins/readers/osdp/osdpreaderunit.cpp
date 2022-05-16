/**
 * \file osdpreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR reader unit.
 */

#include <logicalaccess/plugins/readers/osdp/osdpreaderunit.hpp>
#include <logicalaccess/plugins/readers/osdp/osdpreaderprovider.hpp>


#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/services/accesscontrol/cardsformatcomposite.hpp>
#include <logicalaccess/cards/chip.hpp>

#include <boost/filesystem.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/plugins/readers/osdp/readercardadapters/osdpserialportdatatransport.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireev1iso7816commands.hpp>
#include <logicalaccess/plugins/readers/osdp/readercardadapters/osdpreadercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireiso7816resultchecker.hpp>

#include <logicalaccess/plugins/readers/osdp/osdpcommands.hpp>
#include <logicalaccess/plugins/readers/osdp/osdpledbuzzerdisplay.hpp>
#include <logicalaccess/plugins/readers/osdp/osdplcddisplay.hpp>

namespace logicalaccess
{
OSDPReaderUnit::OSDPReaderUnit()
    : ReaderUnit(READER_OSDP), m_tamperStatus(false), m_inserted(false)
{
    d_readerUnitConfig.reset(new OSDPReaderUnitConfiguration());

    ReaderUnit::setDataTransport(std::make_shared<OSDPSerialPortDataTransport>());

    std::shared_ptr<ReaderCardAdapter> rca(new ReaderCardAdapter());
    rca->setDataTransport(ReaderUnit::getDataTransport());

    m_commands.reset(new OSDPCommands());
    m_commands->setReaderCardAdapter(rca);

    d_card_type = CHIP_UNKNOWN;

    try
    {
        boost::property_tree::ptree pt;
        read_xml((boost::filesystem::current_path().string() + "/OSDPReaderUnit.config"),
                 pt);
        d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
    }
    catch (...)
    {
    }
}

OSDPReaderUnit::~OSDPReaderUnit()
{
    OSDPReaderUnit::disconnectFromReader();
}

std::string OSDPReaderUnit::getName() const
{
    return getDataTransport()->getName();
}

std::string OSDPReaderUnit::getConnectedName()
{
    return getName();
}

void OSDPReaderUnit::setCardType(std::string cardType)
{
    LOG(LogLevel::INFOS) << "Setting card type {" << cardType << "}";
    d_card_type = cardType;
}

std::shared_ptr<Chip> OSDPReaderUnit::createChip(std::string type)
{
    std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);
    setcryptocontext setcryptocontextfct;

    if (chip)
    {
        LOG(LogLevel::INFOS) << "Chip created successfully !";
        std::shared_ptr<ReaderCardAdapter> rca;
        std::shared_ptr<Commands> commands;
        
        if (m_current_csn.size() > 0)
        {
            chip->setChipIdentifier(m_current_csn);
        }

        if (type == "DESFire" || type == "DESFireEV1")
        {
            LOG(LogLevel::INFOS) << "Mifare DESFire EV1 Chip created";
            rca.reset(new OSDPReaderCardAdapter(
                m_commands, getOSDPConfiguration()->getRS485Address(),
                std::make_shared<DESFireISO7816ResultChecker>()));
            commands = LibraryManager::getInstance()->getCommands("DESFireEV1ISO7816");
            *(void **)(&setcryptocontextfct) =
                LibraryManager::getInstance()->getFctFromName(
                    "setCryptoContextDESFireEV1ISO7816Commands",
                    LibraryManager::READERS_TYPE);
            setcryptocontextfct(&commands, &chip);
        }
        else
            return chip;

        rca->setDataTransport(getDataTransport());
        if (commands)
        {
            commands->setReaderCardAdapter(rca);
            commands->setChip(chip);
            chip->setCommands(commands);
        }
    }
    return chip;
}

bool OSDPReaderUnit::waitInsertion(unsigned int maxwait)
{
    unsigned int currentWait = 0;

    do
    {
        std::shared_ptr<OSDPChannel> poll = m_commands->poll();
        if (!m_inserted)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            currentWait += 100;
        }
    } while (!m_inserted && (maxwait == 0 || currentWait < maxwait));

    if (m_inserted)
    {
        if (getOSDPConfiguration()->getVisualFeedback())
        {
            s_led_cmd osdp_LED_cmd = {0,
                                      0,
                                      TemporaryControleCode::SetTemporaryState,
                                      5,
                                      5,
                                      OSDPColor::Green,
                                      OSDPColor::Black,
                                      30,
                                      0,
                                      PermanentControlCode::SetPermanentState,
                                      1,
                                      0,
                                      OSDPColor::Red,
                                      OSDPColor::Black};
            m_commands->led(osdp_LED_cmd);

            s_buz_cmd osdp_BUZ_cmd = {0, 2, 1, 1, 3};
            m_commands->buz(osdp_BUZ_cmd);
        }

        d_insertedChip = createChip(d_card_type);
    }

    return m_inserted;
}

bool OSDPReaderUnit::waitRemoval(unsigned int maxwait)
{
    unsigned int currentWait = 0;

    do
    {
        std::shared_ptr<OSDPChannel> poll = m_commands->poll();

        if (m_inserted)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            currentWait += 100;
        }
    } while (m_inserted && (maxwait == 0 || currentWait < maxwait));

    return !m_inserted;
}

bool OSDPReaderUnit::waitKeypadInputs(unsigned int maxwait)
{
    unsigned int currentWait = 0;

    m_last_keypad.clear();
    do
    {
        std::shared_ptr<OSDPChannel> poll = m_commands->poll();

        if (m_last_keypad.size() == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        currentWait += 100;
    } while (m_last_keypad.size() == 0 && (maxwait == 0 || currentWait < maxwait));

    return (m_last_keypad.size() > 0);
}

bool OSDPReaderUnit::connect()
{
    return true;
}

void OSDPReaderUnit::disconnect()
{
    if (getOSDPConfiguration()->getTransparentMode())
    {
        std::shared_ptr<OSDPChannel> result = m_commands->disconnectFromSmartcard();
        if (result->getCommandsType() != OSDPCommandsType::OSDP_ACK)
        {
            LOG(LogLevel::ERRORS) << "Impossible to disconnect from card";
        }
    }

    m_inserted = false;
    m_current_csn.clear();
}

void OSDPReaderUnit::checkPDAuthentication(std::shared_ptr<OSDPChannel> challenge)
{
    ByteVector cryptogramInput;

    std::shared_ptr<OSDPReaderUnitConfiguration> config = getOSDPConfiguration();
    challenge->getSecureChannel()->deriveKey(config->getSCBKKey(), config->getSCBKDKey());
    challenge->getSecureChannel()->computeAuthenticationData();
}

bool OSDPReaderUnit::connectToReader()
{
    bool ret = getDataTransport()->connect();
    if (ret)
    {
        m_commands->initCommands(getOSDPConfiguration()->getRS485Address(), getOSDPConfiguration()->getInstallMode());

        // Test if can read
        //m_commands->poll();

        // Start Secure Channel
        std::shared_ptr<OSDPChannel> challenge = m_commands->challenge();
        checkPDAuthentication(challenge);
        std::shared_ptr<OSDPChannel> crypt = m_commands->sCrypt();
        // Successful Authentication
        crypt->isSCB = true;

        std::shared_ptr<OSDPChannel> result = m_commands->getProfile();
        if (result->getCommandsType() == OSDP_XRD)
        {
            ByteVector &data = result->getData();
            uint8_t mode     = getOSDPConfiguration()->getTransparentMode() ? 0x01 : 0x00;
            if (data.size() > 2 && data[0x03] != mode)
            {
                result = m_commands->setProfile(mode);
                if (result->getCommandsType() != OSDP_ACK)
                    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                             "Impossible to set Profile");
            }
        }
        
        m_commands->setCardEventHandler(
            std::bind(&OSDPReaderUnit::onCardEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        m_commands->setKeypadEventHandler(
            std::bind(&OSDPReaderUnit::onKeypadEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        m_commands->setTamperEventHandler(std::bind(&OSDPReaderUnit::onTamperEvent, this, std::placeholders::_1, std::placeholders::_2));
    }

    return ret;
}

void OSDPReaderUnit::onCardEvent(uint8_t readerAddress, ByteVector data, uint16_t bitCount)
{
    if (readerAddress == getOSDPConfiguration()->getRS485Address())
    {
        m_inserted = true;
        if (data.size() > 0)
        {
            m_current_csn = data;
        }
    }
    else
    {
         LOG(LogLevel::INFOS) << "Skipping card event from reader address "
                              << readerAddress;
    }
}

void OSDPReaderUnit::onKeypadEvent(uint8_t readerAddress, ByteVector data, uint16_t bitCount)
{
    if (readerAddress == getOSDPConfiguration()->getRS485Address())
    {
        m_last_keypad = data;
    }
    else
    {
         LOG(LogLevel::INFOS) << "Skipping keypad event from reader address "
                              << readerAddress;
    }
}

void OSDPReaderUnit::onTamperEvent(bool tamperStatus, bool /*powerFailure*/)
{
    LOG(LogLevel::INFOS) << "Tamper status changed to: " << tamperStatus;
    m_tamperStatus = tamperStatus;
}

void OSDPReaderUnit::disconnectFromReader()
{
    getDataTransport()->disconnect();
}

std::shared_ptr<Chip> OSDPReaderUnit::getSingleChip()
{
    std::shared_ptr<Chip> chip = d_insertedChip;
    return chip;
}

std::vector<std::shared_ptr<Chip>> OSDPReaderUnit::getChipList()
{
    std::vector<std::shared_ptr<Chip>> chipList;
    std::shared_ptr<Chip> singleChip = getSingleChip();
    if (singleChip)
    {
        chipList.push_back(singleChip);
    }
    return chipList;
}

std::string OSDPReaderUnit::getReaderSerialNumber()
{
    return std::string();
}

bool OSDPReaderUnit::isConnected()
{
    return true;
}

std::shared_ptr<LCDDisplay> OSDPReaderUnit::getLCDDisplay()
{
    auto lcd = std::make_shared<OSDPLCDDisplay>();
    lcd->setReaderCardAdapter(getDefaultReaderCardAdapter());
    return lcd;
}

std::shared_ptr<LEDBuzzerDisplay> OSDPReaderUnit::getLEDBuzzerDisplay()
{
    auto ledbuzzer = std::make_shared<OSDPLEDBuzzerDisplay>();
    ledbuzzer->setReaderCardAdapter(getDefaultReaderCardAdapter());
    return ledbuzzer;
}

void OSDPReaderUnit::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    ReaderUnit::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void OSDPReaderUnit::unSerialize(boost::property_tree::ptree &node)
{
    ReaderUnit::unSerialize(node);
}
}

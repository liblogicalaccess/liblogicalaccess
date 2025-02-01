/**
 * \file stidstrreaderunit.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief STidSTR reader unit.
 */

#include <logicalaccess/plugins/readers/stidstr/stidstrreaderunit.hpp>
#include <logicalaccess/plugins/readers/stidstr/stidstrreaderprovider.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>

#include <logicalaccess/services/accesscontrol/cardsformatcomposite.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/readers/stidstr/readercardadapters/stidstrreadercardadapter.hpp>
#include <logicalaccess/plugins/readers/stidstr/stidstrledbuzzerdisplay.hpp>

#include <logicalaccess/plugins/readers/stidstr/commands/desfireev1stidstrcommands.hpp>
#include <logicalaccess/plugins/readers/stidstr/commands/mifarestidstrcommands.hpp>
#include <boost/filesystem.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/plugins/readers/stidstr/readercardadapters/stidstrreaderserialportdatatransport.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireev1iso7816commands.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarechip.hpp>

#include <logicalaccess/plugins/readers/stidstr/stidstrreaderunitconfiguration.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace logicalaccess
{
STidSTRReaderUnit::STidSTRReaderUnit()
    : ISO7816ReaderUnit(READER_STIDSTR)
{
    d_readerUnitConfig.reset(new STidSTRReaderUnitConfiguration());
    ReaderUnit::setDefaultReaderCardAdapter(
        std::make_shared<STidSTRReaderCardAdapter>(STID_PM_NORMAL, STID_CMD_READER));
    d_ledBuzzerDisplay.reset(new STidSTRLEDBuzzerDisplay());
    std::shared_ptr<STidSTRSerialPortDataTransport> dataTransport(new STidSTRSerialPortDataTransport());
    dataTransport->setPortBaudRate(38400);
    ReaderUnit::setDataTransport(dataTransport);
    d_card_type = CHIP_UNKNOWN;

    try
    {
        boost::property_tree::ptree pt;
        read_xml(
            (boost::filesystem::current_path().string() + "/STidSTRReaderUnit.config"),
            pt);
        d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
    }
    catch (...)
    {
    }
}

STidSTRReaderUnit::~STidSTRReaderUnit()
{
    STidSTRReaderUnit::disconnectFromReader();
}

std::string STidSTRReaderUnit::getName() const
{
    return getDataTransport()->getName();
}

std::string STidSTRReaderUnit::getConnectedName()
{
    return getName();
}

void STidSTRReaderUnit::setCardType(std::string cardType)
{
    LOG(LogLevel::INFOS) << "Setting card type {" << cardType << "}";
    d_card_type = cardType;
}

bool STidSTRReaderUnit::waitInsertion(unsigned int maxwait)
{
    bool oldValue = Settings::getInstance()->IsLogEnabled;
    if (oldValue && !Settings::getInstance()->SeeWaitInsertionLog)
    {
        Settings::getInstance()->IsLogEnabled =
            false; // Disable logs for this part (otherwise too much log output in file)
    }

    auto stidprgdt = std::dynamic_pointer_cast<STidSTRSerialPortDataTransport>(getDataTransport());
    EXCEPTION_ASSERT_WITH_LOG(stidprgdt, LibLogicalAccessException,
                              "Invalid data transport.");

    LOG(LogLevel::INFOS) << "Waiting insertion... max wait {" << maxwait << "}";
    bool inserted = false;
    std::chrono::steady_clock::time_point const clock_timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(maxwait);

    try
    {
        do
        {
            std::shared_ptr<Chip> chip;
            if (getSTidSTRConfiguration()->getProtocolVersion() == STID_SSCP_V1)
            {
                chip = scanARaw(); // scan14443A() => Obsolete. It's just used for testing purpose !
                if (!chip)
                {
                    chip = scan14443B();
                }
            }
            else
            {
                chip = scanGlobal();
            }

            if (chip)
            {
                LOG(LogLevel::INFOS) << "Chip detected !";
                d_insertedChip = chip;
                inserted       = true;
            }

            if (!inserted)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
            else
            {
                if ((chip->getCardType() == CHIP_DESFIRE_EV1 ||
                     chip->getCardType() == CHIP_DESFIRE) &&
                    getSTidSTRConfiguration()->getPN532Direct())
                {
                    std::dynamic_pointer_cast<DESFireISO7816Commands>(
                        d_insertedChip->getCommands())
                        ->setSAMChip(getSAMChip());
                }
            }
        } while (!inserted && std::chrono::steady_clock::now() < clock_timeout);
    }
    catch (...)
    {
        Settings::getInstance()->IsLogEnabled = oldValue;
        throw;
    }

    LOG(LogLevel::INFOS) << "Returns card inserted ? {" << inserted
                         << "} function timeout expired ? {"
                         << (std::chrono::steady_clock::now() < clock_timeout) << "}";
    Settings::getInstance()->IsLogEnabled = oldValue;

    return inserted;
}

bool STidSTRReaderUnit::waitRemoval(unsigned int maxwait)
{
    bool oldValue = Settings::getInstance()->IsLogEnabled;
    if (oldValue && !Settings::getInstance()->SeeWaitRemovalLog)
    {
        Settings::getInstance()->IsLogEnabled =
            false; // Disable logs for this part (otherwise too much log output in file)
    }

    LOG(LogLevel::INFOS) << "Waiting removal... max wait {" << maxwait << "}";
    bool removed = false;
    std::chrono::steady_clock::time_point const clock_timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(maxwait);
    try
    {
        if (d_insertedChip)
        {
            do
            {
                std::shared_ptr<Chip> chip;
                if (getSTidSTRConfiguration()->getProtocolVersion() == STID_SSCP_V1)
                {
                    chip = scanARaw(); // scan14443A() => Obsolete. It's
                                                             // just used for testing purpose
                                                             // !
                    if (!chip)
                    {
                        chip = scan14443B();
                    }
                }
                else
                {
                    chip = scanGlobal();
                }

                if (chip)
                {
                    if (chip->getChipIdentifier() != d_insertedChip->getChipIdentifier())
                    {
                        LOG(LogLevel::INFOS) << "Card found but not same chip ! The "
                                                "previous card has been removed !";
                        d_insertedChip.reset();
                        removed = true;
                    }
                }
                else
                {
                    LOG(LogLevel::INFOS) << "Card removed !";
                    d_insertedChip.reset();
                    removed = true;
                }

                if (!removed)
                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
            } while (!removed && std::chrono::steady_clock::now() < clock_timeout);
        }
    }
    catch (...)
    {
        Settings::getInstance()->IsLogEnabled = oldValue;
        throw;
    }

    LOG(LogLevel::INFOS) << "Returns card removed ? {" << removed
                         << "} - function timeout expired ? {"
                         << (std::chrono::steady_clock::now() < clock_timeout) << "}";

    Settings::getInstance()->IsLogEnabled = oldValue;

    return removed;
}

bool STidSTRReaderUnit::connect()
{
    LOG(LogLevel::WARNINGS) << "Connect do nothing with STid STR reader";
    return true;
}

void STidSTRReaderUnit::disconnect()
{
    LOG(LogLevel::WARNINGS) << "Disconnect do nothing with STid STR reader";
}

bool STidSTRReaderUnit::connectToReader()
{
    bool connected = getDataTransport()->connect();
    if (connected)
    {
        // Negotiate sessions according to communication options
        if ((getSTidSTRConfiguration()->getCommunicationMode() & STID_CM_SIGNED) ==
            STID_CM_SIGNED)
        {
            LOG(LogLevel::INFOS) << "Signed communication required, negotiating HMAC...";
            authenticateHMAC();
        }
        if ((getSTidSTRConfiguration()->getCommunicationMode() & STID_CM_CIPHERED) ==
            STID_CM_CIPHERED)
        {
            LOG(LogLevel::INFOS) << "Ciphered communication required, negotiating AES...";
            authenticateAES();
        }
    }

    if (connected)
    {
        connected = ISO7816ReaderUnit::connectToReader();
    }

    return connected;
}

void STidSTRReaderUnit::disconnectFromReader()
{
    ISO7816ReaderUnit::disconnectFromReader();
    getDataTransport()->disconnect();
}

ByteVector STidSTRReaderUnit::getPingCommand() const
{
    ByteVector buffer;

    buffer.push_back(0x00);
    buffer.push_back(0x08);

    return buffer;
}

std::shared_ptr<Chip> STidSTRReaderUnit::createChip(std::string type)
{
    LOG(LogLevel::INFOS) << "Creating chip... chip type {" << type << "}";
    std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

    if (chip)
    {
        LOG(LogLevel::INFOS) << "Chip created successfully !";
        std::shared_ptr<ReaderCardAdapter> rca;
        std::shared_ptr<Commands> commands;

        if (type == "Mifare1K" || type == "Mifare4K" || type == "Mifare")
        {
            LOG(LogLevel::INFOS) << "Mifare classic Chip created";
            rca.reset(new STidSTRReaderCardAdapter(STID_PM_NORMAL, STID_CMD_MIFARE_CLASSIC));
            commands.reset(new MifareSTidSTRCommands());
        }
        else if (type == "DESFire" || type == "DESFireEV1")
        {
            LOG(LogLevel::INFOS) << "Mifare DESFire Chip created";
            rca.reset(new STidSTRReaderCardAdapter(STID_PM_NORMAL, STID_CMD_DESFIRE));
            if (getSTidSTRConfiguration()->getPN532Direct())
            {
                commands.reset(new DESFireEV1ISO7816Commands());
            }
            else
            {
                commands.reset(new DESFireEV1STidSTRCommands());
            }
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
    else
    {
        LOG(LogLevel::ERRORS) << "Unable to create the chip with this type !";
    }
    return chip;
}

std::shared_ptr<Chip> STidSTRReaderUnit::getSingleChip()
{
    // LOG(LogLevel::INFOS) << "Getting the first detect chip...");
    std::shared_ptr<Chip> chip = d_insertedChip;

    return chip;
}

std::vector<std::shared_ptr<Chip>> STidSTRReaderUnit::getChipList()
{
    // LOG(LogLevel::INFOS) << "Getting all detected chips...");
    std::vector<std::shared_ptr<Chip>> chipList;
    std::shared_ptr<Chip> singleChip = getSingleChip();
    if (singleChip)
    {
        chipList.push_back(singleChip);
    }
    return chipList;
}

std::shared_ptr<STidSTRReaderCardAdapter>
STidSTRReaderUnit::getDefaultSTidSTRReaderCardAdapter()
{
    std::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
    return std::dynamic_pointer_cast<STidSTRReaderCardAdapter>(adapter);
}

std::string STidSTRReaderUnit::getReaderSerialNumber()
{
    LOG(LogLevel::WARNINGS) << "Do nothing with STid STR reader";
    return std::string();
}

bool STidSTRReaderUnit::isConnected()
{
    if (d_insertedChip)
        LOG(LogLevel::INFOS) << "Is connected {1}";
    else
        LOG(LogLevel::INFOS) << "Is connected {0}";
    return bool(d_insertedChip);
}

void STidSTRReaderUnit::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    ReaderUnit::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void STidSTRReaderUnit::unSerialize(boost::property_tree::ptree &node)
{
    ReaderUnit::unSerialize(node);
}

std::shared_ptr<STidSTRReaderProvider> STidSTRReaderUnit::getSTidSTRReaderProvider() const
{
    return std::dynamic_pointer_cast<STidSTRReaderProvider>(getReaderProvider());
}

std::shared_ptr<Chip> STidSTRReaderUnit::scan14443A()
{
    LOG(LogLevel::INFOS) << "Scanning 14443A chips...";
    std::shared_ptr<Chip> chip;
    ByteVector response =
        getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0002, ByteVector());
    if (response.size() > 0)
    {
        bool haveCard = (response[0] == 0x01);
        if (haveCard && response.size() > 4)
        {
            STidCmdType stidCardType = static_cast<STidCmdType>(response[1]);

            LOG(LogLevel::INFOS) << "Response detected stid card type {" << stidCardType
                                 << "}";

            // unsigned char cardSize = response.constData<unsigned char>()[2];
            // unsigned char chipInfo = response.constData<unsigned char>()[3];
            unsigned char uidLen = response[4];

            LOG(LogLevel::INFOS) << "UID length {" << uidLen << "}";

            if (uidLen > 0)
            {
                ByteVector uid =
                    ByteVector(response.begin() + 5, response.begin() + 5 + uidLen);

                LOG(LogLevel::INFOS) << "UID " << BufferHelper::getHex(uid) << "-{"
                                     << BufferHelper::getStdString(uid) << "}";
                std::string cardType = CHIP_UNKNOWN;
                switch (stidCardType)
                {
                case STID_CMD_MIFARE_CLASSIC:
                    LOG(LogLevel::INFOS) << "Mifare classic type !";
                    cardType = "Mifare";
                    break;

                case STID_CMD_DESFIRE:
                    LOG(LogLevel::INFOS) << "Mifare DESFire type !";
                    cardType = "DESFireEV1";
                    break;

                default:
                    LOG(LogLevel::INFOS) << "Unknown type !";
                    cardType = "UNKNOWN";
                    break;
                }

                chip = createChip(cardType);
                chip->setChipIdentifier(uid);

                LOG(LogLevel::INFOS)
                    << "Scanning for specific chip type, mandatory for STid reader...";
                // Scan for specific chip type, mandatory for STid reader...
                if (cardType == "DESFire" || cardType == "DESFireEV1")
                {
                    std::shared_ptr<DESFireEV1Chip> dchip =
                        std::dynamic_pointer_cast<DESFireEV1Chip>(chip);
                    std::dynamic_pointer_cast<DESFireEV1STidSTRCommands>(
                        dchip->getDESFireEV1Commands())
                        ->scanDESFire();
                }
                else if (cardType == "Mifare" || cardType == "Mifare1K" ||
                         cardType == "Mifare4K")
                {
                    std::shared_ptr<MifareChip> mchip =
                        std::dynamic_pointer_cast<MifareChip>(chip);
                    std::dynamic_pointer_cast<MifareSTidSTRCommands>(
                        mchip->getMifareCommands())
                        ->scanMifare();
                }
            }
            else
            {
                LOG(LogLevel::ERRORS) << "No UID retrieved !";
            }
        }
        else if (!haveCard)
        {
            LOG(LogLevel::INFOS) << "No card detected !";
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Command length should be > 4";
        }
    }
    else
    {
        LOG(LogLevel::ERRORS) << "No response !";
    }

    return chip;
}

std::shared_ptr<Chip> STidSTRReaderUnit::createChipFromiso14443aBuffer(const ByteVector& data)
{
    std::shared_ptr<Chip> chip;
    if (data.size() > 0)
    {
        bool haveCard = (data[0] == 0x01);
        if (haveCard && data.size() > 5)
        {
            unsigned short atqa = (data[1] << 8) | data[2];
            LOG(LogLevel::INFOS) << "ATQA (Answer To Request Type A) value {0x"
                                 << std::hex << atqa << std::dec << "(" << atqa << ")}";

            // Voir documentation AN10833 de NXP
            std::string cardType = CHIP_UNKNOWN;
            switch (atqa)
            {
            case 0x002:
                LOG(LogLevel::INFOS) << "Mifare classic 4K !";
                cardType = "Mifare4K";
                break;
            case 0x004:
                LOG(LogLevel::INFOS) << "Mifare classic 1K !"; // Ou Mifare Mini
                cardType = "Mifare1K";
                break;
            case 0x0042:
            case 0x0044:
                LOG(LogLevel::INFOS) << "Mifare Ultralight";
                cardType = "MifareUltralight";
                break;
            case 0x0344:
                LOG(LogLevel::INFOS) << "Mifare DESFire type !";
                cardType = "DESFireEV1";
                break;
            default:
                LOG(LogLevel::INFOS) << "Unknown type !";
                cardType = CHIP_UNKNOWN;
                break;
            }

            // unsigned char sak = data[3];
            // LOG(LogLevel::INFOS) << "SAK (Select Acknowloedge Type A) value
            // {0x%x(%u)}", sak);

            unsigned char uidLen = data[4];
            LOG(LogLevel::INFOS) << "UID length {" << uidLen << "}";

            if (uidLen > 0)
            {
                ByteVector uid =
                    ByteVector(data.begin() + 5, data.begin() + 5 + uidLen);

                LOG(LogLevel::INFOS) << "UID " << BufferHelper::getHex(uid) << "-{"
                                     << BufferHelper::getStdString(uid) << "}";

                chip = createChip(cardType);
                chip->setChipIdentifier(uid);

                LOG(LogLevel::INFOS)
                    << "Scanning for specific chip type, mandatory for STid reader...";
                // Scan for specific chip type, mandatory for STid reader...
                if (cardType == "DESFire" || cardType == "DESFireEV1")
                {
                    std::shared_ptr<DESFireEV1Commands> chipcmd =
                        std::dynamic_pointer_cast<DESFireEV1Commands>(
                            chip->getCommands());
                    auto stev1 =
                        std::dynamic_pointer_cast<DESFireEV1STidSTRCommands>(chipcmd);
                    if (stev1)
                        stev1->scanDESFire();
                }
                else if (cardType == "Mifare" || cardType == "Mifare1K" ||
                         cardType == "Mifare4K")
                {
                    std::shared_ptr<MifareCommands> chipcmd =
                        std::dynamic_pointer_cast<MifareCommands>(chip->getCommands());
                    std::dynamic_pointer_cast<MifareSTidSTRCommands>(chipcmd)
                        ->scanMifare();
                }
            }
            else
            {
                LOG(LogLevel::ERRORS) << "No UID retrieved !";
            }
        }
        else if (!haveCard)
        {
            LOG(LogLevel::INFOS) << "No card detected !";
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Command length should be > 5";
        }
    }
    else
    {
        LOG(LogLevel::ERRORS) << "No data !";
    }
    return chip;
}

std::shared_ptr<Chip> STidSTRReaderUnit::scanARaw()
{
    LOG(LogLevel::INFOS) << "Scanning 14443A RAW chips...";
    std::shared_ptr<Chip> chip;
    ByteVector command;
    command.push_back(getSTidSTRConfiguration()->getPN532Direct()
                          ? 0x01
                          : 0x00); // 0x01 to Request ATS (required for DESFire / PN532
                                   // direct communication), 0x00 otherwise

    ByteVector response =
        getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000F, command);
    
    return createChipFromiso14443aBuffer(response);
}

std::shared_ptr<Chip> STidSTRReaderUnit::scan14443B()
{
    LOG(LogLevel::INFOS) << "Scanning 14443B chips...";
    std::shared_ptr<Chip> chip;
    ByteVector response =
        getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0009, ByteVector());
    if (response.size() > 0)
    {
        bool haveCard = (response[0] == 0x01);
        if (haveCard && response.size() > 4)
        {
            unsigned char uidLen = response[1];
            if (uidLen > 0)
            {
                ByteVector uid =
                    ByteVector(response.begin() + 2, response.begin() + 5 + uidLen);

                LOG(LogLevel::INFOS) << "UID length {" << uidLen << "}";
                chip = createChip(CHIP_UNKNOWN);
                chip->setChipIdentifier(uid);
            }
            else
            {
                LOG(LogLevel::ERRORS) << "No UID retrieved !";
            }
        }
        else if (!haveCard)
        {
            LOG(LogLevel::INFOS) << "No card detected !";
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Command length should be > 4";
        }
    }
    else
    {
        LOG(LogLevel::ERRORS) << "No response !";
    }

    return chip;
}

std::shared_ptr<Chip> STidSTRReaderUnit::createGenericChipFromBuffer(const ByteVector& data, std::string cardType, bool lenIsShort)
{
    std::shared_ptr<Chip> chip;
    if (data.size() > 0)
    {
        bool haveCard = (data[0] == 0x01);
        if (haveCard && data.size() > 3)
        {
            unsigned short uidLen = data[2];
            if (lenIsShort)
            {
                uidLen |= data[1] << 8;
            }
            LOG(LogLevel::INFOS) << "UID length {" << uidLen << "}";

            if (uidLen > 0)
            {
                ByteVector uid = ByteVector(data.begin() + 3, data.begin() + 3 + uidLen);
                LOG(LogLevel::INFOS) << "UID " << BufferHelper::getHex(uid) << "-{"
                                     << BufferHelper::getStdString(uid) << "}";
                                     
                if (cardType == "")
                {
                    cardType = CHIP_GENERICTAG;
                }

                chip = createChip(cardType);
                chip->setChipIdentifier(uid);
            }
            else
            {
                LOG(LogLevel::ERRORS) << "No UID retrieved !";
            }
        }
        else if (!haveCard)
        {
            LOG(LogLevel::INFOS) << "No card detected !";
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Command length should be > 3";
        }
    }
    else
    {
        LOG(LogLevel::ERRORS) << "No response !";
    }
    return chip;
}

std::shared_ptr<Chip> STidSTRReaderUnit::scanGlobal()
{
    return scanGlobal(true, true, true, true, true, false, true, true);
}

std::shared_ptr<Chip> STidSTRReaderUnit::scanGlobal(bool iso14443a, bool activeRats, bool iso14443b, bool lf125khz, bool blueNfc, bool selectedKeyBlueNfc, bool keyboard, bool imageScanEngine)
{
    LOG(LogLevel::INFOS) << "Scanning global...";
    std::shared_ptr<Chip> chip;
    ByteVector command;
    
    unsigned char filter1 = //((tamperSwitch ? 1 : 0) << 7) |
                            //((configCard ? 1 : 0) << 6) |
                            //((touchCoordinates ? 1 : 0) << 2) |
                            ((imageScanEngine ? 1 : 0) << 1) |
                            (keyboard ? 1 : 0);
    unsigned char filter2 = ((selectedKeyBlueNfc) ? 1 : 0 << 5) |
                            ((blueNfc) ? 1 : 0 << 4) |
                            ((lf125khz) ? 1 : 0 << 3) |
                            ((iso14443b) ? 1 : 0 << 2) |
                            ((activeRats) ? 1 : 0 << 1) |
                            (iso14443a ? 1 : 0);

    command.push_back(filter1);
    command.push_back(filter2);

    ByteVector response = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x00B0, command);
    if (response.size() > 0)
    {
        unsigned char infoType = response[0];
        ByteVector data = ByteVector(response.begin() + 1, response.end());
        switch(infoType)
        {
            case 0x01: // ISO14443-A
            {
                chip = createChipFromiso14443aBuffer(data);
            } break;
            case 0x02: // ISO14443-B
            {
                chip = createGenericChipFromBuffer(data);
            } break;
            case 0x03: // 125Khz
            {
                chip = createGenericChipFromBuffer(data);
            } break;
            case 0x04: // Blue-NFC
            {
                chip = createGenericChipFromBuffer(data);
            } break;
            case 0x09: // Keyboard
            {
            } break;
            case 0x10: // Tamper
            {
            } break;
            case 0x11: // Image Scan Engine
            {
                chip = createGenericChipFromBuffer(data, CHIP_GENERICTAG, true);
            } break;
            case 0x12: // Touch Coordinates
            {
            } break;
            case 0x40: // Config Card
            {
            } break;
            default:
            {
                LOG(LogLevel::INFOS) << "Unknown info type.";
            } break;
        }
    }
    else
    {
        LOG(LogLevel::ERRORS) << "No response !";
    }
    return chip;
}

void STidSTRReaderUnit::authenticateHMAC()
{
    LOG(LogLevel::INFOS) << "Authenticating HMAC (signed communication, SSCP v1)...";

    ByteVector buf1;
    std::shared_ptr<HMAC1Key> key = getSTidSTRConfiguration()->getHMACKey();
    if (key->isEmpty())
    {
        LOG(LogLevel::INFOS) << "Empty key... using the default one !";
        key.reset(new HMAC1Key("A0 87 75 4B 75 47 48 10 94 BE"));
    }

    unsigned int len = 20;
    ByteVector key16ks;
    key16ks.resize(len, 0x00);

    ByteVector rndB;
    rndB.resize(16);
    if (RAND_bytes(&rndB[0], static_cast<int>(rndB.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }

    openssl::AESInitializationVector aesiv =
        openssl::AESInitializationVector::createNull();
    openssl::AESCipher cipher;

    ByteVector buf2, encbuf2;

    /* IF THE TIME BETWEEN authenticate 1 and 2 is TOO HIGH, WE RECEIVE AN ERROR INCORRECT
     * DATA */
    /* BY DECLARING EVERYTHING BEFORE authenticate, IT SEEMS TO BE ENOUGH TO WORK FOR COM
     * COMPONENTS */

    ByteVector rndA        = authenticateReader1(true);
    ByteVector keydata = key->getData();
    buf1.insert(buf1.end(), keydata.begin(), keydata.end());
    buf1.insert(buf1.end(), rndA.begin(), rndA.end());

    HMAC(EVP_sha1(), &keydata[0], static_cast<int>(keydata.size()), &buf1[0],
         buf1.size(), &key16ks[0], &len);
    key16ks.resize(key16ks.size() - 4);
    openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(key16ks);

    buf2.insert(buf2.end(), rndA.begin(), rndA.end());
    buf2.insert(buf2.end(), rndB.begin(), rndB.end());
    cipher.cipher(buf2, encbuf2, aeskey, aesiv, false);

    ByteVector encbuf3 = authenticateReader2(encbuf2);

    ByteVector buf3;
    cipher.decipher(encbuf3, buf3, aeskey, aesiv, false);
    EXCEPTION_ASSERT_WITH_LOG(buf3.size() == 32, LibLogicalAccessException,
                              "Bad second reader response length.");

    ByteVector rndB2 = ByteVector(buf3.begin(), buf3.begin() + 16);
    EXCEPTION_ASSERT_WITH_LOG(rndB2 == rndB, LibLogicalAccessException,
                              "Cannot negotiate the session: RndB'' != RndB.");
    ByteVector rndC = ByteVector(buf3.begin() + 16, buf3.begin() + 16 + 16);

    d_sessionKey_hmac_ab.clear();
    d_sessionKey_hmac_ab.push_back(rndB[0]);
    d_sessionKey_hmac_ab.push_back(rndB[1]);
    d_sessionKey_hmac_ab.push_back(rndB[2]);
    d_sessionKey_hmac_ab.push_back(rndC[0]);
    d_sessionKey_hmac_ab.push_back(rndC[1]);
    d_sessionKey_hmac_ab.push_back(rndB[14]);
    d_sessionKey_hmac_ab.push_back(rndB[15]);
    d_sessionKey_hmac_ab.push_back(rndC[13]);
    d_sessionKey_hmac_ab.push_back(rndC[14]);
    d_sessionKey_hmac_ab.push_back(rndC[15]);
    d_sessionKey_hmac_ba = d_sessionKey_hmac_ab;
}

void STidSTRReaderUnit::authenticateAES()
{
    LOG(LogLevel::INFOS) << "Authenticating AES (ciphered communication, SSCP v1)...";

    ByteVector rndB;
    rndB.resize(16);
    if (RAND_bytes(&rndB[0], static_cast<int>(rndB.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }

    std::shared_ptr<AES128Key> key = getSTidSTRConfiguration()->getAESKey();
    if (key->isEmpty())
    {
        LOG(LogLevel::INFOS) << "Empty key... using the default one !";
        key.reset(new AES128Key("E7 4A 54 0F A0 7C 4D B1 B4 64 21 12 6D F7 AD 36"));
    }

    openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(key->getData());
    openssl::AESInitializationVector aesiv =
        openssl::AESInitializationVector::createNull();
    openssl::AESCipher cipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_CBC);

    ByteVector rndA;
    ByteVector buf1, encbuf1;
    ByteVector rndB2;

    /* IF THE TIME BETWEEN authenticate 1 and 2 is TOO HIGH, WE RECEIVE AN ERROR INCORRECT
     * DATA */
    /* BY DECLARING EVERYTHING BEFORE authenticate, IT SEEMS TO BE ENOUGH TO WORK FOR COM
     * COMPONENTS */

    ByteVector encrndA = authenticateReader1(false);
    cipher.decipher(encrndA, rndA, aeskey, aesiv, false);
    buf1.insert(buf1.end(), rndA.begin(), rndA.end());
    buf1.insert(buf1.end(), rndB.begin(), rndB.end());
    cipher.cipher(buf1, encbuf1, aeskey, aesiv, false);

    ByteVector encrndB2 = authenticateReader2(encbuf1);

    cipher.decipher(encrndB2, rndB2, aeskey, aesiv, false);
    EXCEPTION_ASSERT_WITH_LOG(rndB2 == rndB, LibLogicalAccessException,
                              "Cannot negotiate the session: RndB' != RndB.");

    d_sessionKey_aes_ab.clear();
    d_sessionKey_aes_ab.push_back(rndA[0]);
    d_sessionKey_aes_ab.push_back(rndA[1]);
    d_sessionKey_aes_ab.push_back(rndA[2]);
    d_sessionKey_aes_ab.push_back(rndA[3]);
    d_sessionKey_aes_ab.push_back(rndB[0]);
    d_sessionKey_aes_ab.push_back(rndB[1]);
    d_sessionKey_aes_ab.push_back(rndB[2]);
    d_sessionKey_aes_ab.push_back(rndB[3]);
    d_sessionKey_aes_ab.push_back(rndA[12]);
    d_sessionKey_aes_ab.push_back(rndA[13]);
    d_sessionKey_aes_ab.push_back(rndA[14]);
    d_sessionKey_aes_ab.push_back(rndA[15]);
    d_sessionKey_aes_ab.push_back(rndB[12]);
    d_sessionKey_aes_ab.push_back(rndB[13]);
    d_sessionKey_aes_ab.push_back(rndB[14]);
    d_sessionKey_aes_ab.push_back(rndB[15]);
    d_sessionKey_aes_ba = d_sessionKey_aes_ab;
}

ByteVector STidSTRReaderUnit::authenticateReader1(bool isHMAC)
{
    LOG(LogLevel::INFOS) << "Authenticating Reader 1... is HMAC {" << isHMAC << "}";
    ByteVector command, ret;
    command.push_back(isHMAC ? 0x01 : 0x02);

    STidCommunicationMode lastCM = getSTidSTRConfiguration()->getCommunicationMode();
    getSTidSTRConfiguration()->setCommunicationMode(STID_CM_RESERVED);
    try
    {
        ret = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0001, command, STID_PM_AUTH_REQUEST);
    }
    catch (std::exception &e)
    {
        LOG(LogLevel::ERRORS) << "Exception {" << e.what() << "}";
        getSTidSTRConfiguration()->setCommunicationMode(lastCM);
        throw;
    }
    getSTidSTRConfiguration()->setCommunicationMode(lastCM);

    return ret;
}

ByteVector STidSTRReaderUnit::authenticateReader2(const ByteVector &data)
{
    LOG(LogLevel::INFOS) << "Authenticating Reader 2...";
    ByteVector ret;
    STidCommunicationMode lastCM = getSTidSTRConfiguration()->getCommunicationMode();
    getSTidSTRConfiguration()->setCommunicationMode(STID_CM_RESERVED);
    try
    {
        ret = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0001, data, STID_PM_AUTH_REQUEST);
    }
    catch (std::exception &e)
    {
        LOG(LogLevel::ERRORS) << "Exception {" << e.what() << "}";
        getSTidSTRConfiguration()->setCommunicationMode(lastCM);
        throw;
    }
    getSTidSTRConfiguration()->setCommunicationMode(lastCM);

    return ret;
}

void STidSTRReaderUnit::ResetAuthenticate()
{
    LOG(LogLevel::INFOS) << "Reseting authentication...";
    getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000D, ByteVector());
}

void STidSTRReaderUnit::authenticateReader()
{
    LOG(LogLevel::INFOS) << "Authenticating Reader (SSCP v2)...";

    ByteVector idA = { 0x48, 0x6f, 0x07, 0xad };
    ByteVector idB = { 0x53, 0x77, 0x07, 0xad };

    ByteVector rndA;
    rndA.resize(16);
    if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }

    ByteVector command, ret;
    // Key index - not used here
    command.push_back(0x00);
    command.push_back(0x00);

    command.insert(command.end(), rndA.begin(), rndA.end());

    try
    {
        ret = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0000 /* Not used */, command, STID_PM_AUTH_REQUEST);
        
        if (ret.size() < 40)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unexpected response size");
        
        ByteVector idA2 = ByteVector(ret.begin(), ret.begin() + 4);
        ByteVector idB2 = ByteVector(ret.begin() + 4, ret.begin() + 8);
        ByteVector rndA2 = ByteVector(ret.begin() + 8, ret.begin() + 24);
        ByteVector rndB = ByteVector(ret.begin() + 24, ret.begin() + 40);
        ByteVector signature = ByteVector(ret.begin() + 40, ret.end());

        EXCEPTION_ASSERT_WITH_LOG(rndA == rndA2, LibLogicalAccessException,
                              "Cannot negotiate the session: RndA' != RndA.");

        ByteVector signature2 = calculateHMAC(ByteVector(ret.begin(), ret.begin() + 40), STID_KEYCTX_AUTH);

        EXCEPTION_ASSERT_WITH_LOG(signature == signature2, LibLogicalAccessException,
                              "Cannot negotiate the session: wrong signature.");

        command = ByteVector(idA2.begin(), idA2.end());
        command.insert(command.end(), rndB.begin(), rndB.end());
        signature = calculateHMAC(command, STID_KEYCTX_AUTH);
        command.insert(command.end(), signature.begin(), signature.end());

        ret = getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0000 /* Not used */, command, STID_PM_AUTH_REQUEST);
        if (ret.size() < 6)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unexpected response size");
        EXCEPTION_ASSERT_WITH_LOG(ret[4] == 0x00 && ret[5] == 0x08, LibLogicalAccessException,
                              "Wrong authentication status code.");

        ByteVector w = calculateHMAC(rndB, STID_KEYCTX_AUTH2);
        ByteVector info1 = { 0x02, 0x6a, 0x53, 0x82, 0xe6, 0x53};
        ByteVector info2 = { 0x02, 0x6a};

        ByteVector p1 = { 0x00, 0x00, 0x00, 0x00 };
        p1.insert(p1.end(), w.begin(), w.end());
        p1.insert(p1.end(), info1.begin(), info1.end());
        ByteVector hp1 = openssl::SHA256Hash(p1);
        EXCEPTION_ASSERT_WITH_LOG(hp1.size() == 32, LibLogicalAccessException,
                              "Wrong sha256 size when calculating T.");

        ByteVector p2 = { 0x00, 0x00, 0x00, 0x01 };
        p2.insert(p2.end(), w.begin(), w.end());
        p2.insert(p2.end(), info2.begin(), info2.end());
        ByteVector hp2 = openssl::SHA256Hash(p2);
        EXCEPTION_ASSERT_WITH_LOG(hp2.size() == 32, LibLogicalAccessException,
                              "Wrong sha256 size when calculating T.");

        // T = hp1 + hp2
        d_sessionKey_aes_ab = ByteVector(hp1.begin(), hp1.begin() + 16);
        d_sessionKey_aes_ba = ByteVector(hp1.end() - 16, hp1.end());
        d_sessionKey_hmac_ab = ByteVector(hp2.begin(), hp2.begin() + 16);
        d_sessionKey_hmac_ba = ByteVector(hp2.end() - 16, hp2.end());
        d_cmd_counter = 1;
    }
    catch (std::exception &e)
    {
        LOG(LogLevel::ERRORS) << "Exception {" << e.what() << "}";
        throw;
    }
}

unsigned int STidSTRReaderUnit::getHMACLength() const
{
    return (getSTidSTRConfiguration()->getProtocolVersion() == STID_SSCP_V1) ? 10 : 32;
}

ByteVector STidSTRReaderUnit::calculateHMAC(const ByteVector &buf, STidKeyContext kctx) const
{
    auto key = getKeyFromContext(kctx);
    unsigned int len = getHMACLength();
    ByteVector r;
    r.resize(len, 0x00);
    if (getSTidSTRConfiguration()->getProtocolVersion() == STID_SSCP_V1)
    {
        HMAC(EVP_sha1(), &key[0], static_cast<int>(key.size()), &buf[0],
            buf.size(), &r[0], &len);
    }
    else
    {
        HMAC(EVP_sha256(), &key[0], static_cast<int>(key.size()), &buf[0],
            buf.size(), &r[0], &len);
    }
    return r;
}

ByteVector STidSTRReaderUnit::getKeyFromContext(STidKeyContext kctx) const
{
    ByteVector key;
    if ((kctx & STID_KEYCTX_AUTH) == STID_KEYCTX_AUTH)
        key = getSTidSTRConfiguration()->getAESKey()->getData();
    else if ((kctx & STID_KEYCTX_AUTH2) == STID_KEYCTX_AUTH)
        key = cipherData(getSTidSTRConfiguration()->getAESKey()->getData(), ByteVector(), static_cast<STidKeyContext>(STID_KEYCTX_AUTH | STID_KEYCTX_AES));
    else if ((kctx & STID_KEYCTX_B_TO_A) == STID_KEYCTX_AUTH)
    {
        if ((kctx & STID_KEYCTX_AES) == STID_KEYCTX_AES)
            key = d_sessionKey_aes_ba;
        else
            key = d_sessionKey_hmac_ba;
    }
    else
    {
        if ((kctx & STID_KEYCTX_AES) == STID_KEYCTX_AES)
            key = d_sessionKey_aes_ab;
        else
            key = d_sessionKey_hmac_ab;
    }
    return key;
}

ByteVector STidSTRReaderUnit::cipherData(const ByteVector &buf, const ByteVector &iv, STidKeyContext kctx) const
{
    openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(getKeyFromContext(kctx));
    openssl::AESInitializationVector aesiv =
        iv.size() > 0 ? openssl::AESInitializationVector::createFromData(iv) : openssl::AESInitializationVector::createNull();
    openssl::AESCipher cipher;
    ByteVector encbuf;
    cipher.cipher(buf, encbuf, aeskey, aesiv, false);
    return encbuf;
}

ByteVector STidSTRReaderUnit::uncipherData(const ByteVector &buf, const ByteVector &iv, STidKeyContext kctx) const
{
    openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(getKeyFromContext(kctx));
    openssl::AESInitializationVector aesiv =
        iv.size() > 0 ? openssl::AESInitializationVector::createFromData(iv) : openssl::AESInitializationVector::createNull();
    openssl::AESCipher cipher;
    ByteVector decbuf;
    cipher.decipher(buf, decbuf, aeskey, aesiv, false);
    return decbuf;
}

void STidSTRReaderUnit::ChangeReaderKeys(const ByteVector &key_hmac,
                                         const ByteVector &key_aes)
{
    LOG(LogLevel::INFOS) << "Changing reader keys...";
    ByteVector command;
    unsigned char keyMode = 0x00;
    if (key_hmac.size() > 0)
    {
        EXCEPTION_ASSERT_WITH_LOG(key_hmac.size() == 10, LibLogicalAccessException,
                                  "The authentication HMAC key must be 10-byte long.");
        keyMode |= 0x01;
        command.insert(command.end(), key_hmac.begin(), key_hmac.end());
    }
    if (key_aes.size() > 0)
    {
        EXCEPTION_ASSERT_WITH_LOG(key_aes.size() == 16, LibLogicalAccessException,
                                  "The enciphering AES key must be 16-byte long.");
        keyMode |= 0x02;
        command.insert(command.end(), key_aes.begin(), key_aes.end());
    }
    command.insert(command.begin(), &keyMode, &keyMode + 1);

    getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0003, command);
}

void STidSTRReaderUnit::setBaudRate(STidBaudrate baudrate)
{
    LOG(LogLevel::INFOS) << "Setting baudrate... baudrate {0x" << std::hex << baudrate
                         << std::dec << "(" << baudrate << ")}";
    ByteVector command;
    command.push_back(static_cast<unsigned char>(baudrate));
    getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0005, command);
}

void STidSTRReaderUnit::set485Address(unsigned char address)
{
    LOG(LogLevel::INFOS) << "Setting RS485 address... address {0x" << std::hex << address
                         << std::dec << "(" << address << ")}";
    ByteVector command;
    EXCEPTION_ASSERT_WITH_LOG(address <= 127, LibLogicalAccessException,
                              "The RS485 address should be between 0 and 127.");
    command.push_back(address);
    getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0006, command);
}

STidSTRReaderUnit::STidSTRInformation STidSTRReaderUnit::getReaderInformaton()
{
    LOG(LogLevel::INFOS) << "Getting reader information...";
    ByteVector response =
        getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x0008, ByteVector());

    EXCEPTION_ASSERT_WITH_LOG(response.size() >= 5, LibLogicalAccessException,
                              "The GetInfos response should be 5-byte long.");

    STidSTRInformation readerInfo;
    readerInfo.version      = response[0];
    readerInfo.baudrate     = static_cast<STidBaudrate>(response[1]);
    readerInfo.rs485Address = response[2];
    readerInfo.voltage      = (float)response[3] + ((float)response[4] / (float)10);

    LOG(LogLevel::INFOS) << "Returns version {0x" << std::hex << readerInfo.version
                         << std::dec << "(" << readerInfo.version << ")}"
                         << "baudrate {0x" << std::hex << readerInfo.baudrate << std::dec
                         << "(" << readerInfo.baudrate << ")}"
                         << "rs485 address {0x" << std::hex << readerInfo.rs485Address
                         << std::dec << "(" << readerInfo.rs485Address << ")}"
                         << "voltage {0x" << std::hex << readerInfo.voltage << std::dec
                         << "(" << readerInfo.voltage << ")}";

    return readerInfo;
}

void STidSTRReaderUnit::setAllowedCommModes(bool plainComm, bool signedComm,
                                            bool cipheredComm)
{
    LOG(LogLevel::INFOS) << "Setting allowed communication modes... plain comm {"
                         << plainComm << "} signed comm {" << signedComm
                         << "} ciphered comm {" << cipheredComm << "}";
    ByteVector command;
    unsigned char allowedModes = 0x08 | ((plainComm) ? 0x01 : 0x00) |
                                 ((signedComm) ? 0x02 : 0x00) |
                                 ((cipheredComm) ? 0x04 : 0x00);

    command.push_back(allowedModes);
    getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000A, command);
}

void STidSTRReaderUnit::setTamperSwitchSettings(bool useTamperSwitch,
                                                STidTamperSwitchBehavior behavior)
{
    LOG(LogLevel::INFOS) << "Setting tamper switch settings... use tamper {"
                         << useTamperSwitch << "} tamper behavior {0x" << std::hex
                         << behavior << std::dec << "(" << behavior << ")}";
    ByteVector command;

    command.push_back((useTamperSwitch) ? 0x01 : 0x00);
    command.push_back(static_cast<unsigned char>(behavior));

    getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000B, command);
}

void STidSTRReaderUnit::getTamperSwitchInfos(bool &useTamperSwitch,
                                             STidTamperSwitchBehavior &behavior,
                                             bool &swChanged)
{
    LOG(LogLevel::INFOS) << "Getting tamper switch infos...";
    ByteVector response =
        getDefaultSTidSTRReaderCardAdapter()->sendCommand(0x000C, ByteVector());

    EXCEPTION_ASSERT_WITH_LOG(response.size() >= 3, LibLogicalAccessException,
                              "The GetTamperSwitchInfos response should be 3-byte long.");

    useTamperSwitch = (response[0] == 0x01);
    behavior        = static_cast<STidTamperSwitchBehavior>(response[1]);
    swChanged       = (response[2] == 0x01);

    LOG(LogLevel::INFOS) << "Returns use tamper {" << useTamperSwitch
                         << "} tamper behavior {0x" << std::hex << behavior << std::dec
                         << "(" << behavior << ")}";
}

void STidSTRReaderUnit::loadSKB()
{
    LOG(LogLevel::INFOS) << "Loading SKB...";
    unsigned char statusCode = 0;
    ByteVector response      = getDefaultSTidSTRReaderCardAdapter()->sendCommand(
        0x000E, ByteVector(), statusCode);

    EXCEPTION_ASSERT_WITH_LOG(
        response.size() == 0, LibLogicalAccessException,
        "Unable to load the SKB values. An unknown error occurred.");
}

int STidSTRReaderUnit::getCommandCounter(bool incr)
{
    if (incr)
    {
        if (d_cmd_counter >= 0xfffffffe)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Command counter limit reached, please run again authentication.");
        return d_cmd_counter++;
    }
    else
        return d_cmd_counter;
}

std::shared_ptr<STidSTRReaderUnitConfiguration>
STidSTRReaderUnit::getSTidSTRConfiguration() const
{
    return std::dynamic_pointer_cast<STidSTRReaderUnitConfiguration>(getConfiguration());
}
}
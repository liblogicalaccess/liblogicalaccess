/**
 * \file admittoreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Admitto reader unit.
 */

#include <logicalaccess/plugins/readers/admitto/admittoreaderunit.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/readers/admitto/admittoreaderprovider.hpp>
#include <logicalaccess/services/accesscontrol/cardsformatcomposite.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/readers/admitto/readercardadapters/admittoreadercardadapter.hpp>
#include <boost/filesystem.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/plugins/readers/admitto/readercardadapters/admittodatatransport.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace logicalaccess
{
/*
 *
 *  WARNING: THE READER ONLY WORKS WITH X86 SYSTEMS !
 *  Admitto reader only send ID through Serial port on card insertion and [0x4e] on card
 * removed. It doesn't support any request command !
 *
 */

AdmittoReaderUnit::AdmittoReaderUnit()
    : ReaderUnit(READER_ADMITTO)
{
    d_readerUnitConfig.reset(new AdmittoReaderUnitConfiguration());
    ReaderUnit::setDefaultReaderCardAdapter(std::make_shared<AdmittoReaderCardAdapter>());
    const std::shared_ptr<AdmittoDataTransport> dataTransport(new AdmittoDataTransport());
    ReaderUnit::setDataTransport(dataTransport);
    d_card_type = CHIP_UNKNOWN;

    try
    {
        boost::property_tree::ptree pt;
        read_xml(
            (boost::filesystem::current_path().string() + "/AdmittoReaderUnit.config"),
            pt);
        d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
    }
    catch (...)
    {
    }
}

AdmittoReaderUnit::~AdmittoReaderUnit()
{
    AdmittoReaderUnit::disconnectFromReader();
}

std::string AdmittoReaderUnit::getName() const
{
    return getDataTransport()->getName();
}

std::string AdmittoReaderUnit::getConnectedName()
{
    return getName();
}

void AdmittoReaderUnit::setCardType(const std::string cardType)
{
    LOG(LogLevel::INFOS) << "Setting card type {" << cardType << "}";
    d_card_type = cardType;
}

bool AdmittoReaderUnit::waitInsertion(const unsigned int maxwait)
{
    const bool oldValue = Settings::getInstance()->IsLogEnabled;
    if (oldValue && !Settings::getInstance()->SeeWaitInsertionLog)
    {
        Settings::getInstance()->IsLogEnabled =
            false; // Disable logs for this part (otherwise too much log output in file)
    }

    LOG(LogLevel::INFOS) << "Waiting insertion... max wait {" << maxwait << "}";

    bool inserted = false;
    std::chrono::steady_clock::time_point const clock_timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(maxwait);

    try
    {
        // Admitto reader doesn't handle commands but we want to simulate the same
        // behavior that for all readers
        // So we send a dummy commmand which does nothing
        do
        {
            try
            {
                ByteVector cmd;
                cmd.push_back(0xff); // trick

                ByteVector buf = getDefaultAdmittoReaderCardAdapter()->sendCommand(cmd);
                if (buf.size() > 0)
                {
                    if (buf[0] == 0x4e)
                    {
                        LOG(LogLevel::INFOS) << "[Card removed] response received when "
                                                "we were on WaitInsertion! We ignore "
                                                "this response and wait for valid card "
                                                "number.";
                    }
                    else
                    {
                        const unsigned int l = static_cast<unsigned int>(
                            atoull(BufferHelper::getStdString(buf)));
                        char bufTmpId[64];
                        sprintf(bufTmpId, "%08x", l);
                        d_insertedChip = ReaderUnit::createChip(
                            (d_card_type == CHIP_UNKNOWN ? CHIP_GENERICTAG : d_card_type),
                            formatHexString(std::string(bufTmpId)));
                        LOG(LogLevel::INFOS) << "Chip detected !";
                        inserted = true;
                    }
                }
            }
            catch (std::exception &)
            {
                // No response received is ignored !
            }

            if (!inserted)
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
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

bool AdmittoReaderUnit::waitRemoval(const unsigned int maxwait)
{
    const bool oldValue = Settings::getInstance()->IsLogEnabled;
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
        // The inserted chip will stay inserted until a new identifier is read on the
        // serial port.
        if (d_insertedChip)
        {
            do
            {
                try
                {
                    ByteVector cmd;
                    cmd.push_back(0xff); // trick

                    ByteVector buf =
                        getDefaultAdmittoReaderCardAdapter()->sendCommand(cmd);
                    if (buf.size() > 0)
                    {
                        if (buf[0] == 0x4e)
                        {
                            LOG(LogLevel::INFOS) << "Card removed !";
                            d_insertedChip.reset();
                            removed = true;
                        }
                        else
                        {
                            const unsigned int l = static_cast<unsigned int>(
                                atoull(BufferHelper::getStdString(buf)));
                            char bufTmpId[64];
                            sprintf(bufTmpId, "%08x", l);
                            const std::shared_ptr<Chip> chip = ReaderUnit::createChip(
                                (d_card_type == CHIP_UNKNOWN ? CHIP_GENERICTAG
                                                             : d_card_type),
                                formatHexString(std::string(bufTmpId)));

                            if (chip->getChipIdentifier() !=
                                d_insertedChip->getChipIdentifier())
                            {
                                LOG(LogLevel::INFOS) << "Card found but not same chip ! "
                                                        "The previous card has been "
                                                        "removed !";
                                d_insertedChip.reset();
                                removed = true;
                            }
                        }
                    }
                }
                catch (std::exception &)
                {
                    // No response received is ignored !
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

bool AdmittoReaderUnit::connect()
{
    LOG(LogLevel::WARNINGS) << "Connect do nothing with Admitto reader";
    return true;
}

void AdmittoReaderUnit::disconnect()
{
    LOG(LogLevel::WARNINGS) << "Disconnect do nothing with Admitto reader";
}

std::shared_ptr<Chip> AdmittoReaderUnit::createChip(const std::string type)
{
    LOG(LogLevel::INFOS) << "Creating chip... chip type {" << type << "}";
    std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

    if (chip)
    {
        LOG(LogLevel::INFOS) << "Chip created successfully !";
        std::shared_ptr<ReaderCardAdapter> rca;

        if (type == CHIP_GENERICTAG)
        {
            LOG(LogLevel::INFOS) << "Generic tag Chip created";
            rca = getDefaultReaderCardAdapter();
        }
        else
            return chip;

        rca->setDataTransport(getDataTransport());
    }
    return chip;
}

std::shared_ptr<Chip> AdmittoReaderUnit::getSingleChip()
{
    std::shared_ptr<Chip> chip = d_insertedChip;
    return chip;
}

std::vector<std::shared_ptr<Chip>> AdmittoReaderUnit::getChipList()
{
    std::vector<std::shared_ptr<Chip>> chipList;
    const std::shared_ptr<Chip> singleChip = getSingleChip();
    if (singleChip)
    {
        chipList.push_back(singleChip);
    }
    return chipList;
}

std::shared_ptr<AdmittoReaderCardAdapter>
AdmittoReaderUnit::getDefaultAdmittoReaderCardAdapter()
{
    return std::dynamic_pointer_cast<AdmittoReaderCardAdapter>(
        getDefaultReaderCardAdapter());
}

std::string AdmittoReaderUnit::getReaderSerialNumber()
{
    LOG(LogLevel::WARNINGS) << "Do nothing with Admitto reader";
    std::string ret;
    return ret;
}

bool AdmittoReaderUnit::isConnected()
{
    if (d_insertedChip)
        LOG(LogLevel::INFOS) << "Is connected {1}";
    else
        LOG(LogLevel::INFOS) << "Is connected {0}";
    return bool(d_insertedChip);
}

bool AdmittoReaderUnit::connectToReader()
{
    getDataTransport()->setReaderUnit(shared_from_this());
    return getDataTransport()->connect();
}

void AdmittoReaderUnit::disconnectFromReader()
{
    LOG(LogLevel::INFOS) << "Disconnecting from reader...";
    getDataTransport()->disconnect();
}

ByteVector AdmittoReaderUnit::getPingCommand() const
{
    ByteVector cmd;

    cmd.push_back(0xff);

    return cmd;
}

void AdmittoReaderUnit::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    ReaderUnit::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void AdmittoReaderUnit::unSerialize(boost::property_tree::ptree &node)
{
    ReaderUnit::unSerialize(node);
}

std::shared_ptr<AdmittoReaderProvider> AdmittoReaderUnit::getAdmittoReaderProvider() const
{
    return std::dynamic_pointer_cast<AdmittoReaderProvider>(getReaderProvider());
}
}

#include <logicalaccess/plugins/readers/stidprg/stidprgreaderunit.hpp>
#include <logicalaccess/plugins/readers/stidprg/stidprgreaderprovider.hpp>
#include <logicalaccess/plugins/readers/pcsc/commands/dummycommand.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/readers/stidprg/readercardadapters/stidprgbufferparser.hpp>
#include <logicalaccess/plugins/readers/stidprg/readercardadapters/stidprgserialportdatatransport.hpp>
#include <logicalaccess/plugins/readers/stidprg/readercardadapters/stidprgreadercardadapter.hpp>
#include <logicalaccess/plugins/readers/stidprg/stidprgutils.hpp>
#include <logicalaccess/plugins/readers/stidprg/stidprgresultchecker.hpp>
#include <logicalaccess/plugins/readers/stidprg/stidprgreaderunitconfiguration.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp>
#include <logicalaccess/utils.hpp>
#include <memory>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{

/**
 * A guard object that turn the buzzer off in constructor
 * and restore the previous state in the destructor
 */
struct STidPRGReaderUnit::BuzzerModeGuard
{
    explicit BuzzerModeGuard(STidPRGReaderUnit *reader)
        : reader_(reader)
    {
        auto ret = reader->getDefaultReaderCardAdapter()->sendCommand({0x2A, 0, 0, 0});
        if (ret.size() >= 4)
            st_ = (bool)((ret[3] & 0x10) != 0);
        else
            st_ = false;
        reader->toggleBuzzer(false);
    }

    ~BuzzerModeGuard()
    {
        reader_->toggleBuzzer(st_);
    }

  private:
    bool st_;
    STidPRGReaderUnit *reader_;
};

void STidPRGReaderUnit::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    ReaderUnit::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void STidPRGReaderUnit::unSerialize(boost::property_tree::ptree &node)
{
    ReaderUnit::unSerialize(node);
}

STidPRGReaderUnit::STidPRGReaderUnit()
    : ReaderUnit(READER_STIDPRG)
{
    d_readerUnitConfig = std::make_shared<STidPRGReaderUnitConfiguration>();
    ReaderUnit::setDefaultReaderCardAdapter(std::make_shared<STidPRGReaderCardAdapter>());
    auto dt = std::make_shared<STidPRGSerialPortDataTransport>();
    dt->setPortBaudRate(9600);
    ReaderUnit::getDefaultReaderCardAdapter()->setDataTransport(dt);
    ReaderUnit::setDataTransport(dt);
}

bool STidPRGReaderUnit::waitInsertion(unsigned int maxwait)
{
    ElapsedTimeCounter etc;
    EXCEPTION_ASSERT_WITH_LOG(getDataTransport(), LibLogicalAccessException,
                              "No data transport.");
    auto stidprgdt = std::dynamic_pointer_cast<STidPRGSerialPortDataTransport>(getDataTransport());
    EXCEPTION_ASSERT_WITH_LOG(stidprgdt, LibLogicalAccessException,
                              "Invalid data transport.");
    stidprgdt->setReceiveTimeout(100);
    select_chip_type();
    do
    {
        if (d_insertedChip)
        {
            return true;
        }
        d_insertedChip = getCurrentChip();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    } while (etc.elapsed() < maxwait);
    return !!d_insertedChip;
}

bool STidPRGReaderUnit::waitRemoval(unsigned int maxwait)
{
    ElapsedTimeCounter etc;
    BuzzerModeGuard guard(this);

    auto stidprgdt = std::dynamic_pointer_cast<STidPRGSerialPortDataTransport>(getDataTransport());
    stidprgdt->setReceiveTimeout(100);

    do
    {
        auto chip = getCurrentChip();
        if (!chip)
        {
            d_insertedChip = nullptr;
            return true;
        }
        if (chip && d_insertedChip &&
            chip->getChipIdentifier() != d_insertedChip->getChipIdentifier())
        {
            d_insertedChip = chip;
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    } while (etc.elapsed() < maxwait);
    return false;
}

bool STidPRGReaderUnit::isConnected()
{
    return !!d_insertedChip;
}

void STidPRGReaderUnit::setCardType(std::string /*cardType*/)
{
}

std::shared_ptr<Chip> STidPRGReaderUnit::getSingleChip()
{
    return d_insertedChip;
}

std::vector<std::shared_ptr<Chip>> STidPRGReaderUnit::getChipList()
{
    return {};
}

bool STidPRGReaderUnit::connect()
{
    return true;
}

void STidPRGReaderUnit::disconnect()
{
    d_insertedChip = nullptr;
}

bool STidPRGReaderUnit::connectToReader()
{
    return getDataTransport()->connect();
}

void STidPRGReaderUnit::disconnectFromReader()
{
    getDataTransport()->disconnect();
}

std::string STidPRGReaderUnit::getName() const
{
    // The name is the name of the COM port used.
    return getDataTransport()->getName();
}

std::string STidPRGReaderUnit::getReaderSerialNumber()
{
    return "";
}

std::shared_ptr<Chip> STidPRGReaderUnit::getCurrentChip()
{
    auto ret = getDefaultReaderCardAdapter()->sendCommand({0x21, 0, 0, 0});
    if (ret.size() > 3)
    {
        auto uid_len = ret[2];
        if (static_cast<size_t>(3 + uid_len) >= ret.size())
        {
            auto card_uid = std::vector<uint8_t>(ret.begin() + 3, ret.end());
            auto chip     = createChip("Prox", card_uid);
            auto cmd      = std::make_shared<DummyCommands>();
            cmd->setReaderCardAdapter(getDefaultReaderCardAdapter());
            chip->setCommands(cmd);
            return chip;
        }
        assert(0);
    }
    return nullptr;
}

void STidPRGReaderUnit::toggleBuzzer(bool on)
{
    uint8_t p1 = static_cast<uint8_t>(on ? 0x10 : 0x00);
    getDefaultReaderCardAdapter()->sendCommand({0x2D, p1, 0x00, 0x00});
}

std::vector<uint8_t> STidPRGReaderUnit::readBlocks(uint8_t start, uint8_t end)
{
    EXCEPTION_ASSERT_WITH_LOG(end - start <= 11, LibLogicalAccessException,
                              "Cannot read "
                              "that many block.");
    auto ret = getDefaultReaderCardAdapter()->sendCommand({0x41, start, end, 0});
    return ret;
}

bool STidPRGReaderUnit::login(const std::vector<uint8_t> &password)
{
    EXCEPTION_ASSERT_WITH_LOG(password.size() == 4, LibLogicalAccessException,
                              "Password must be 4 bytes.");

    std::vector<uint8_t> cmd = {0x30, 0, 0, 4};
    cmd.insert(cmd.end(), password.begin(), password.end());

    auto ret = getDefaultReaderCardAdapter()->sendCommand(cmd);
    return true;
}

bool STidPRGReaderUnit::writePassword(const std::vector<uint8_t> &old,
                                      const std::vector<uint8_t> &new_pass)
{
    EXCEPTION_ASSERT_WITH_LOG(old.size() == 4, LibLogicalAccessException,
                              "Password must be 4 bytes.");
    EXCEPTION_ASSERT_WITH_LOG(new_pass.size() == 4, LibLogicalAccessException,
                              "Password must be 4 bytes.");

    std::vector<uint8_t> cmd = {0x33, 0, 0, 8};
    cmd.insert(cmd.end(), old.begin(), old.end());
    cmd.insert(cmd.end(), new_pass.begin(), new_pass.end());

    auto ret = getDefaultReaderCardAdapter()->sendCommand(cmd);
    return true;
}

bool STidPRGReaderUnit::writeBlock(uint8_t start, uint8_t end,
                                   const std::vector<uint8_t> &data)
{
    EXCEPTION_ASSERT_WITH_LOG(end >= start, LibLogicalAccessException,
                              "Invalid block number")
    EXCEPTION_ASSERT_WITH_LOG(end - start <= 8, LibLogicalAccessException,
                              "Cannot write "
                              "that many block.");
    EXCEPTION_ASSERT_WITH_LOG(static_cast<int>(data.size()) == (end - start + 1) * 4,
                              LibLogicalAccessException,
                              "Not enough or too many bytes of data.");


    uint8_t p1               = static_cast<uint8_t>(((end & 0xFF) << 4) | (start & 0xFF));
    std::vector<uint8_t> cmd = {0x42, p1, 0, static_cast<uint8_t>((end - start + 1) * 4)};


    cmd.insert(cmd.end(), data.begin(), data.end());
    auto ret = getDefaultReaderCardAdapter()->sendCommand(cmd);
    return true;
}

bool STidPRGReaderUnit::format(uint8_t start, uint8_t end,
                               const std::vector<uint8_t> &pwd)
{
    EXCEPTION_ASSERT_WITH_LOG(end - start <= 11, LibLogicalAccessException,
                              "Cannot use "
                              "that many block.");
    EXCEPTION_ASSERT_WITH_LOG(pwd.size() == 4, LibLogicalAccessException,
                              "Password should be 4 bytes.");
    std::vector<uint8_t> cmd = {0x34, start, end, 4};
    cmd.insert(cmd.end(), pwd.begin(), pwd.end());
    auto ret = getDefaultReaderCardAdapter()->sendCommand(cmd);
    return true;
}

void STidPRGReaderUnit::ihm_control(uint8_t device, int duration)
{
    uint8_t p2 = static_cast<uint8_t>(duration / 100);

    getDefaultReaderCardAdapter()->sendCommand({0x2F, device, p2, 0x00});
}

void STidPRGReaderUnit::select_chip_type()
{
    // Chip type shall be 0x03 for writable 13.56
    getDefaultReaderCardAdapter()->sendCommand({0x20, 0x03, 0, 0x00});
}

std::shared_ptr<STidPRGReaderUnitConfiguration>
STidPRGReaderUnit::getSTidPRGReaderUnitConfiguration() const
{
    return std::dynamic_pointer_cast<STidPRGReaderUnitConfiguration>(d_readerUnitConfig);
}
}

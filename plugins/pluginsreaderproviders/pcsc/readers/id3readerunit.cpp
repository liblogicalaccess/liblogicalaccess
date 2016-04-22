#include "id3readerunit.hpp"
#include "../atrparser.hpp"
#include "../commands/id3resultchecker.hpp"
#include "../pcscdatatransport.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"
#include "cardprobes/cl1356cardprobe.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include <array>
#include <cassert>
#include <iostream>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/utils.hpp>
#include <string>
#include <thread>

using namespace logicalaccess;

ID3ReaderUnit::ID3ReaderUnit(const std::string &name)
    : PCSCReaderUnit(name)
{
}

std::vector<CL1356PlusUtils::Info> ID3ReaderUnit::listCards()
{
    CL1356PlusUtils::CardCount counts;
    {
        APDUWrapperGuard guard(this);
        counts = CL1356PlusUtils::parse_list_card(
            getDefaultReaderCardAdapter()->sendCommand({0x0E, 0, 0}));
    }

    std::vector<CL1356PlusUtils::Info> infos;
    for (int i = 0; i < counts.present_; ++i)
    {
        CL1356PlusUtils::Info info;
        {
            APDUWrapperGuard guard(this);
            info = CL1356PlusUtils::parse_get_card_info(
                getDefaultReaderCardAdapter()->sendCommand(
                    {0x11, static_cast<uint8_t>(i), 0}));
        }
        // We might as well extract ATR.
        info.atr_ = getAtr(i);
        selectCard(i);
        power_card(true);
        info.guessed_type_ =
            ATRParser::guessCardType(info.atr_, PCSC_RUT_ID3_CL1356);

        // This type detection cannot be done by `adjustChip()` because we are
        // not working with Chip object at this point.
        if (info.guessed_type_ == "DESFire" && createCardProbe()->is_desfire_ev1())
            info.guessed_type_ = "DESFireEV1";
        infos.push_back(info);

        power_card(false);
    }
    return infos;
}

std::shared_ptr<Chip> ID3ReaderUnit::selectCard(uint8_t idx)
{
    ByteVector ret;
    {
        APDUWrapperGuard guard(this);
        ret = getDefaultReaderCardAdapter()->sendCommand({0x10, idx});
        EXCEPTION_ASSERT_WITH_LOG(
            ret.size() == 0, LibLogicalAccessException,
            "Excepted a 0 length response, got something else");
        power_card(true);
    }
    return nullptr;
}

PCSCReaderUnitType ID3ReaderUnit::getPCSCType() const
{
    return PCSC_RUT_ID3_CL1356;
}

std::shared_ptr<CardProbe> ID3ReaderUnit::createCardProbe()
{
    return std::make_shared<CL1356CardProbe>(this);
}

void ID3ReaderUnit::unfreeze()
{
    APDUWrapperGuard guard(this);
    getDefaultReaderCardAdapter()->sendCommand({0x0C, 0x00});
}

bool ID3ReaderUnit::process_insertion(const std::string &cardType, int maxwait,
                                      const ElapsedTimeCounter &etc)
{
    // The rules for waitInsertion on this reader:
    // 1) If cardType isn't forced, we simply rely on automatic card tracking
    //    and let the reader pick the first card.
    // 2) If cardType is forced, we try to find a card that match this type. For this
    //    we do not rely on automatic card tracking, but rather on the list card
    //    command.
    PCSCReaderUnit::process_insertion(cardType, maxwait, etc);
    if (d_card_type == "UNKNOWN") // automatic
    {
        return true;
    }
    else
    {
        PCSCReaderUnit::connect();

        // Now that we are hopefully connected to something, we try to find
        // the card we meant to use.
        while (etc.elapsed() < static_cast<unsigned int>(maxwait))
        {
            if (select_correct_card())
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    }
    return false;
}

ByteVector ID3ReaderUnit::getAtr(int idx)
{
    APDUWrapperGuard guard(this);

    return getDefaultReaderCardAdapter()->sendCommand(
        {0x12, static_cast<uint8_t>(idx), 0});
}

void ID3ReaderUnit::disconnect()
{
    if (connection_)
    {
        unfreeze();
        power_card(false);
    }
    PCSCReaderUnit::disconnect();
}

void ID3ReaderUnit::power_card(bool power_on)
{
    APDUWrapperGuard guard(this);
    getDefaultReaderCardAdapter()->sendCommand(
        {0x13, static_cast<uint8_t>(power_on), 0x00});
}

bool ID3ReaderUnit::connect(PCSCShareMode share_mode)
{
    // If waitInsertion() has been called before, then we already have
    // established a PCSC connection. In that case, we do nothing, as all
    // is good already.
    if (isConnected())
        return true;

    // However, if we are not connected, we have to do so now. We also have
    // to make sure we pick the correct card.
    bool ret = PCSCReaderUnit::connect(share_mode);
    if (ret)
    {
        // And now we select our card again, based on the forced type.
        // If such a card is not present, too bad.
        return select_correct_card();
    }
    return false;
}

bool ID3ReaderUnit::select_correct_card()
{
    if (d_card_type == "UNKNOWN") // not forced, noop
        return true;

    uint8_t idx = 0;
    for (const auto &card_info : listCards())
    {
        if (card_info.guessed_type_ == d_card_type)
        {
            atr_ = getAtr(idx);
            selectCard(idx);
            power_card(true);
            d_insertedChip = createChip(d_card_type);
            d_insertedChip = adjustChip(d_insertedChip);
            return true;
        }
        idx++;
    }
    return false;
}

std::vector<std::shared_ptr<Chip>> ID3ReaderUnit::getChipList()
{
    std::vector<std::shared_ptr<Chip>> l;

    uint8_t idx = 0;
    for (const auto &info : listCards())
    {
        auto chip = createChip(info.guessed_type_);
        selectCard(idx);
        power_card(true);
        chip = adjustChip(chip);
        l.push_back(chip);
        idx++;
    }
    chips_ = l;
    return l;
}

std::shared_ptr<Chip> ID3ReaderUnit::selectCard(std::shared_ptr<Chip> c)
{
    auto itr = std::find(chips_.begin(), chips_.end(), c);
    if (itr != chips_.end())
    {
        auto idx = std::distance(chips_.begin(), itr);
        selectCard(idx);
        return c;
    }
    return nullptr;
}

// Below is code for APDU wrapping.
// Two nested classes are used in order to provide a clean
// API to wrap command.

ID3ReaderUnit::APDUWrapperGuard::APDUWrapperGuard(ID3ReaderUnit *reader)
    : reader_(reader)
{
    assert(reader);
    adapter_               = reader_->getDefaultReaderCardAdapter();
    auto temporary_adapter = std::make_shared<Adapter>();
    temporary_adapter->setDataTransport(adapter_->getDataTransport());
    reader_->setDefaultReaderCardAdapter(temporary_adapter);
}

ID3ReaderUnit::APDUWrapperGuard::~APDUWrapperGuard()
{
    reader_->setDefaultReaderCardAdapter(adapter_);
}

std::vector<unsigned char> ID3ReaderUnit::APDUWrapperGuard::Adapter::adaptCommand(
    const std::vector<unsigned char> &command)
{
    assert(command.size() <= 255);
    auto tmp = ByteVector{0xFF, 0x9F};
    tmp.insert(tmp.end(), command.begin(), command.end());
    return tmp;
}

std::vector<unsigned char> ID3ReaderUnit::APDUWrapperGuard::Adapter::adaptAnswer(
    const std::vector<unsigned char> &res)
{
    EXCEPTION_ASSERT_WITH_LOG(res.size() >= 2, LibLogicalAccessException,
                              "Response is too short.");
    // Check for PCSC status code first.
    if (res[res.size() - 2] != 0x90 && res[res.size() - 1] != 0x00)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "ID3ReaderUnit Proprietary adapter had PCSC error code.");
    return ByteVector(res.begin(), res.end() - 2);
}

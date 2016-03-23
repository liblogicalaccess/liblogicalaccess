#include "id3readerunit.hpp"
#include "../atrparser.hpp"
#include "../pcscdatatransport.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"
#include "cardprobes/cl1356cardprobe.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include <array>
#include <cassert>
#include <iostream>
#include <string>

using namespace logicalaccess;

ID3ReaderUnit::ID3ReaderUnit(const std::string &name)
    : PCSCReaderUnit(name)
{
}

std::vector<CL1356PlusUtils::Info> ID3ReaderUnit::listCards()
{
    APDUWrapperGuard guard(this);

    auto counts = CL1356PlusUtils::parse_list_card(
        getDefaultReaderCardAdapter()->sendCommand({0x0E, 0, 0}));

    std::vector<CL1356PlusUtils::Info> infos;
    for (int i = 0; i < counts.present_; ++i)
    {
        auto info = CL1356PlusUtils::parse_get_card_info(
            getDefaultReaderCardAdapter()->sendCommand(
                {0x11, static_cast<uint8_t>(i), 0}));


        // We might as well extract ATR.
        info.atr_ = getDefaultReaderCardAdapter()->sendCommand(
            {0x12, static_cast<uint8_t>(i), 0});
        info.guessed_type_ =
            ATRParser::guessCardType(info.atr_, PCSC_RUT_ID3_CL1356);
        infos.push_back(info);
    }
    return infos;
}

ChipInformation ID3ReaderUnit::extract_card_info(ByteVector::iterator &itr,
                                                 const ByteVector::iterator &end)
{
    auto check_enough_bytes = [&](int n) {
        EXCEPTION_ASSERT_WITH_LOG(
            std::distance(itr, end) >= n, LibLogicalAccessException,
            "Parsing listCard response failed. Not enough bytes.");
    };
    // Check we have the byte giving us the length of the UID.
    check_enough_bytes(1);
    int uid_size = *(itr++);
    check_enough_bytes(uid_size);

    ChipInformation chip_info;
    chip_info.identifier_ = ByteVector(itr, itr + uid_size);
    itr += uid_size;

    // 3 bytes for baud rate information. We don't really care.
    check_enough_bytes(3);
    itr += 3;
    return chip_info;
}

void ID3ReaderUnit::toggleCardOperation(bool e)
{
    APDUWrapperGuard guard(this);
    if (e)
        getDefaultReaderCardAdapter()->sendCommand({0x01});
    else
        getDefaultReaderCardAdapter()->sendCommand({0x02});
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
    //    auto tmp =
    //        ByteVector{0xFF, 0x9F, 0x00, 0x00,
    //        static_cast<uint8_t>(command.size())};
    // tmp.insert(tmp.end(), command.begin(), command.end());

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

#include "id3readerunit.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"
#include "../pcscdatatransport.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include <cassert>
#include <array>
#include <iostream>
#include <string>

using namespace logicalaccess;

ID3ReaderUnit::ID3ReaderUnit(const std::string &name)
    : PCSCReaderUnit(name)
{
}

std::vector<ChipInformation> ID3ReaderUnit::listCards()
{
    APDUWrapperGuard guard(this);

    auto ret = getDefaultReaderCardAdapter()->sendCommand({0x09});
    EXCEPTION_ASSERT_WITH_LOG(ret.size() >= 1, LibLogicalAccessException,
                              "listCards response is too short");
    // Check ID3 status byte.
    // todo improve error handling. more generic.
    EXCEPTION_ASSERT_WITH_LOG(ret[0] == 0, LibLogicalAccessException, "Failure");
    EXCEPTION_ASSERT_WITH_LOG(ret.size() >= 2, LibLogicalAccessException,
                              "listCards response is too short");
    int nb_cards = ret[1];

    std::vector<ChipInformation> chips_info;
    auto iterator_start = ret.begin() + 2;
    auto iterator_end = ret.end();
    for (int i = 0; i < nb_cards; ++i)
    {
        chips_info.push_back(extract_card_info(iterator_start, iterator_end));
    }
    return chips_info;
}

ChipInformation ID3ReaderUnit::extract_card_info(ByteVector::iterator &itr,
                                                 const ByteVector::iterator &end)
{
    auto check_enough_bytes = [&](int n)
    {
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
        ret = getDefaultReaderCardAdapter()->sendCommand({0x0A, idx});
        EXCEPTION_ASSERT_WITH_LOG(ret.size() >= 1, LibLogicalAccessException,
                                  "selectCard response is too short");
        EXCEPTION_ASSERT_WITH_LOG(ret[0] == 0, LibLogicalAccessException, "Failure");
    }

    // Now the hard part is detecting what type of card we selected, creating
    // the proper Chip object, along with its command object, etc.
    std::array<SCARD_READERSTATE, 1> readers;
    SCARD_READERSTATE &reader = readers[0];
    std::memset(&reader, 0, sizeof(reader));
    reader.dwCurrentState = SCARD_STATE_UNAWARE;
    reader.dwEventState   = SCARD_STATE_UNAWARE;
    reader.szReader = strdup(getConnectedName().c_str()); // Memory leak? Where?!?!?!
    assert(reader.szReader);
    assert(getPCSCReaderProvider()->getContext());

    std::cout << "CURRENT CSN: " << BufferHelper::getHex(getCardSerialNumber())
              << std::endl;
    // toggleCardOperation(false);
    // toggleCardOperation(true);

    auto r = SCardGetStatusChange(getPCSCReaderProvider()->getContext(), INFINITE,
                                  &readers[0], 1);
    if (SCARD_S_SUCCESS == r)
    {
        if (SCARD_STATE_PRESENT & reader.dwEventState)
        {
            std::cout << "We have a card. ATR IS "
                      << BufferHelper::getHex(
                             ByteVector(std::begin(reader.rgbAtr),
                                        std::begin(reader.rgbAtr) + reader.cbAtr))
                      << std::endl;
        }
    }
    else
    {
        PCSCDataTransport::CheckCardError(r);
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
    auto tmp =
        ByteVector{0xFF, 0x9F, 0x00, 0x00, static_cast<uint8_t>(command.size())};
    tmp.insert(tmp.end(), command.begin(), command.end());

    return tmp;
}

std::vector<unsigned char> ID3ReaderUnit::APDUWrapperGuard::Adapter::adaptAnswer(
    const std::vector<unsigned char> &res)
{
    EXCEPTION_ASSERT_WITH_LOG(res.size() > 2, LibLogicalAccessException,
                              "Response is too short.");
    // Check for PCSC status code first.
    if (res[res.size() - 2] != 0x90 && res[res.size() - 1] != 0x00)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "ID3ReaderUnit Proprietary adapter had PCSC error code.");
    return ByteVector(res.begin(), res.end() - 2);
}

//
// Created by xaqq on 5/29/15.
//

#include "logicalaccess/tests/tools/utils.hpp"
#include <iostream>
#include <sstream>

#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"

#include "pluginsreaderproviders/pcsc/pcscreaderunit.hpp"
#include "logicalaccess/tests/tools/macros.hpp"

std::ostream &operator<<(std::ostream &o, std::vector<uint8_t> &v)
{
    std::stringstream tmp;
    tmp << std::hex;
    for (auto itr = v.begin(); itr != v.end();)
    {
        tmp << (int) *itr;
        if (++itr != v.end())
            tmp << ", ";
    }
    o << tmp.str();
    return o;
}

std::tuple<ReaderProviderPtr, ReaderUnitPtr, ChipPtr> pcsc_test_init()
{
    // Reader configuration object to store reader provider and reader unit selection.
    std::shared_ptr<logicalaccess::ReaderConfiguration> readerConfig(
            new logicalaccess::ReaderConfiguration());

    // Set PCSC ReaderProvider by calling the Library Manager which will load the function from the corresponding plug-in
    auto provider = logicalaccess::LibraryManager::getInstance()->getReaderProvider("PCSC");
    LLA_ASSERT(provider, "Cannot get PCSC provider");
    readerConfig->setReaderProvider(provider);

    auto readerUnit = readerConfig->getReaderProvider()->createReaderUnit();

    LLA_ASSERT(readerUnit, "Cannot create reader unit");

    LLA_ASSERT(readerUnit->connectToReader(), "Cannot connect to reader");
    PRINT_TIME("Connected to reader");
    PRINT_TIME("StartWaitInsertation");

    LLA_ASSERT(readerUnit->waitInsertion(15000), "waitInsertion failed");
    PRINT_TIME("EndWaitInsertation");

    LLA_ASSERT(readerUnit->connect(), "Failed to connect");
    PRINT_TIME("Connected");

    auto chip = readerUnit->getSingleChip();
    LLA_ASSERT(chip, "getSingleChip() returned NULL");
    PRINT_TIME("GetSingleChip");

    return std::make_tuple(provider, readerUnit, chip);
}

void pcsc_test_shutdown(ReaderUnitPtr readerUnit)
{
    readerUnit->disconnect();
    PRINT_TIME("StartWaitRemoval");
    LLA_ASSERT(readerUnit->waitRemoval(15000), "failed to waitRemoval");
    PRINT_TIME("EndWaitRemoval");
    readerUnit->disconnectFromReader();
}

void prologue()
{
    char *wait_mstime = getenv("LLA_TEST_WAIT");
    if (wait_mstime)
    {
        int duration = std::atoi(wait_mstime);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    }
}

std::string get_os_name()
{
#ifdef  _WIN64
  return "Win64";
#elif _WIN32
    return "Win32";
#elif __APPLE__ || __MACH__
    return "Mac OSX";
#elif __linux__
    return "Linux";
#else
    static_assert(0);
#endif
}

std::string pcsc_reader_unit_name(ReaderUnitPtr ru)
{
    using namespace logicalaccess;
    PCSCReaderUnitPtr pcsc_reader = std::dynamic_pointer_cast<PCSCReaderUnit>(ru);
    LLA_ASSERT(pcsc_reader, "Reader is not PCSC");

    switch (pcsc_reader->getPCSCType())
    {
        case PCSC_RUT_OMNIKEY_XX21:
            return "OKXX21";
        case PCSC_RUT_OMNIKEY_XX25:
            return "OKXX25";
        case PCSC_RUT_OMNIKEY_XX27:
            return "OKXX27";
        default:
            LLA_ASSERT(0, "PCSC Type not handled");
    }
    return "I AM DEAD";
}

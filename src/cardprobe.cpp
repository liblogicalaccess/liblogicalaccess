#include <logicalaccess/cardprobe.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <assert.h>
#include <chrono>
#include <thread>

using namespace logicalaccess;

CardProbe::CardProbe(ReaderUnit *ru)
    : reader_unit_(ru)
{
}

std::string CardProbe::guessCardType()
{
    if (maybe_mifare_classic())
        return "Mifare1K";
    if (is_desfire_ev1())
        return "DESFireEV1";
    if (is_desfire())
        return "DESFire";

    return "NOT_FOUND";
}
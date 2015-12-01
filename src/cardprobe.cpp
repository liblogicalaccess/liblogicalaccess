#include <assert.h>
#include <chrono>
#include <thread>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/cardprobe.hpp"
#include "../plugins/pluginscards/desfire/desfirecommands.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "../plugins/pluginscards/mifare/mifarecommands.hpp"
#include "../plugins/pluginscards/mifare/mifareprofile.hpp"
#include "../plugins/pluginsreaderproviders/pcsc/pcscreaderunit.hpp"

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
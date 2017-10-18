#include "../../../plugins/pluginscards/mifare/mifareaccessinfo.hpp"
#include "lla-tests/macros.hpp"
#include <array>

int main(int, char **)
{
    using namespace logicalaccess;

    MifareAccessInfo::SectorAccessBits sab;
    ByteVector array = { {1} };
    LLA_ASSERT(sab.fromArray(array) == false,
               "Passed validation but shouldn't have.");

    ByteVector array2 = { {0xff, 0x07, 0x80} };
    LLA_ASSERT(sab.fromArray(array2) == true,
               "Failed validation");
    return 0;
}

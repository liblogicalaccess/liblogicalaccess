#include "../../../plugins/pluginscards/mifare/mifareaccessinfo.hpp"
#include "lla-tests/macros.hpp"
#include <array>

int main(int, char **)
{
    using namespace logicalaccess;

    MifareAccessInfo::SectorAccessBits sab;
	std::array<uint8_t, 3> array = { {1} };
    LLA_ASSERT(sab.fromArray(&array[0], 3) == false,
               "Passed validation but shouldn't have.");

	std::array<uint8_t, 3> array2 = { {0xff, 0x07, 0x80} };
    LLA_ASSERT(sab.fromArray(&array2[0], 3) == true,
               "Failed validation");
    return 0;
}

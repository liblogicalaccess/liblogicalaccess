#include "../../../plugins/pluginscards/mifare/mifareaccessinfo.hpp"
#include "lla-tests/macros.hpp"
#include <array>

int main(int, char **)
{
    using namespace logicalaccess;

    MifareAccessInfo::SectorAccessBits sab;
	std::vector<uint8_t> tmp(3);

    std::array<uint8_t, 3> array = {1};
	std::copy_n(array.begin(), 3, tmp.begin());
    LLA_ASSERT(sab.fromArray(tmp) == false,
               "Passed validation but shouldn't have.");

    std::array<uint8_t, 3> array2 = {0xff, 0x07, 0x80};
	std::copy_n(array2.begin(), 3, tmp.begin());
    LLA_ASSERT(sab.fromArray(tmp) == true,
               "Failed validation");
    return 0;
}

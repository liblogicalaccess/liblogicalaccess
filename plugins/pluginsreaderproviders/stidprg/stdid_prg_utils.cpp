#include "stid_prg_utils.hpp"
#include <bitset>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/utils.hpp>

using namespace logicalaccess;

static bool get_nth_bit(const ByteVector &v, int n)
{
    auto byte = n / 8;
    auto bit  = n % 8;

    return ((v[byte] << bit) & 0x80) != 0;
}

ByteVector STidPRGUtils::prox_configuration_bytes(const Format &fmt)
{
    auto length = fmt.getDataLength();
    ByteVector data = fmt.getLinearData();

    ByteVector out = {0x00, 0x10, 0x70, 0x60}; // Block 0 always this.
    out.push_back(0x1D);                       // Preamble, always there.
    std::bitset<44> tmp;

    // 2 pattern.
    // < 37 bits and >= 37 bits.
    if (length < 37)
    {
        tmp[6]           = true; // HID bit
        auto padding     = 37 - length;
        tmp[6 + padding] = true; // Start bit
        for (int i                = 0; i < static_cast<int>(length); ++i)
            tmp[6 + padding + 1 + i] = get_nth_bit(data, i);
    }
    else if (length == 37)
    {
        // No HID or start bit
        for (int i     = 0; i < 37; ++i)
            tmp[7 + i] = get_nth_bit(data, i);
    }
    else
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Format not handled.");
    }

    auto manchester = bitsetToVector(
        ManchesterEncoder::encode(tmp, ManchesterEncoder::Type::G_E_THOMAS));
    out.insert(out.end(), manchester.begin(), manchester.end());
    return out;
}

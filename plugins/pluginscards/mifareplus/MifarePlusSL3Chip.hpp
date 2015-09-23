#pragma once

#include "../mifare/mifarechip.hpp"

namespace logicalaccess
{
    class MifarePlusSL3Chip : public MifarePlusChip
    {
    public:
        MifarePlusSL3Chip(int is_2k) :
                Chip(is_2k ? "MifarePlus_SL3_2K" : "MifarePlus_SL3_4K")
        {

        }

        int getSecurityLevel() const override { return 3;};
    };
}

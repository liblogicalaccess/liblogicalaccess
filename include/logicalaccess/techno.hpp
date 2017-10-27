#pragma once

// Define the various technology

#include <bitset>
#include <cstdint>

namespace logicalaccess
{
namespace Techno
{
typedef enum : uint32_t {
    ISO_14443_A         = 1,
    ISO_14443_B         = 2,
    ISO_15693           = 4,
    ICLASS_14443_B      = 8,
    FELICA              = 16,
    PROX                = 32,
    OK5427_CONFIG_CARDS = 64,
    ICLASS_15693        = 128, // 5321
    ICODE1              = 256, // 5321
    STM14443B           = 512  // 5321
} CardTechnologies;
}
using TechnoBitset = uint32_t;
}

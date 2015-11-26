#pragma once

#include <cstdint>

/**
 * Plateform independant utils.
 */

namespace logicalaccess
{
	uint32_t lla_htonl(uint32_t in);
	uint16_t lla_htons(uint16_t in);
}
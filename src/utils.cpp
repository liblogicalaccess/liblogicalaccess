#include "logicalaccess/utils.hpp"
#include <boost/asio.hpp>

namespace logicalaccess
{
	uint32_t lla_htonl(uint32_t in)
	{
		return htonl(in);
	}

	uint16_t lla_htons(uint16_t in)
	{
		return htons(in);
	}
}

#include "logicalaccess/utils.hpp"
#include <boost/asio.hpp>

using namespace std::chrono;

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

uint32_t lla_ntohl(uint32_t in)
{
    return ntohl(in);
}

uint16_t lla_ntohs(uint16_t in)
{
    return ntohs(in);
}

ElapsedTimeCounter::ElapsedTimeCounter()
{
    creation_ = steady_clock::now();
}

size_t ElapsedTimeCounter::elapsed() const
{
    auto diff = steady_clock::now() - creation_;
    return static_cast<size_t>(duration_cast<milliseconds>(diff).count());
}
}
#pragma once

#include <cstdint>
#include <ctime>
#include "logicalaccess/msliblogicalaccess.h"

class LIBLOGICALACCESS_API MyClock
{
private:
    std::size_t clock_start_;

public:
    MyClock()
    {
        clock_start_ = std::clock();
    }

    std::size_t operator()() const
    {
        return clock_start_;
    }
};

LIBLOGICALACCESS_API MyClock &get_global_clock();

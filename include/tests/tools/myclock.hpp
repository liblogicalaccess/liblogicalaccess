#pragma once

#include <cstdint>
#include <ctime>

class MyClock
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

extern MyClock __global_clock;

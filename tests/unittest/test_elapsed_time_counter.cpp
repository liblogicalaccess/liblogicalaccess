#include "logicalaccess/utils.hpp"
#include <gtest/gtest.h>
#include <thread>

using namespace logicalaccess;

TEST(test_elapsed_time_counter, simple)
{
    ElapsedTimeCounter counter;
    // We give 20 ms CPU scheduler delay just in case
    ASSERT_LE(counter.elapsed(), 20);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_LE(counter.elapsed(), 530);
    ASSERT_GE(counter.elapsed(), 500);
}

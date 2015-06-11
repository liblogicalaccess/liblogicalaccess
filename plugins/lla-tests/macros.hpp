#pragma once

#include <iostream>
#include <csignal>
#include "myclock.hpp"
#include "subtest_tracker.hpp"

#define PRINT_TIME(msg)                                                                                  \
            std::cout << ((std::clock() - get_global_clock()()) / static_cast<double>(CLOCKS_PER_SEC)) <<    \
            '\t' << msg << std::endl

#define PRINT_ERROR_IMPL(expr, msg) std::cerr << "Assertion failed: " << (expr) << ". " << (msg) << std::endl

#define LLA_ASSERT(cond, msg)                   \
  do {                                          \
  if (!(cond)) {                                \
  PRINT_ERROR_IMPL((#cond), (msg));             \
  raise(SIGABRT);                               \
  }                                             \
  }                                             \
  while (0)

/**
 * When a sub-test passed this macro will log it.
 * This is useful because some sub-tests are disabled depending on reader/OS
 */
#define LLA_SUBTEST_PASSED(name) SubTestTracker::instance().add_passed((name));

/**
 * Let the system that a potential subtest exists.
 */
#define LLA_SUBTEST_REGISTER(name) SubTestTracker::instance().register_test((name));

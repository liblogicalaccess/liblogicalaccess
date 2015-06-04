//
// Created by xaqq on 6/3/15.
//

#ifndef LLA_TESTS_SUBTEST_TRACKER_H
#define LLA_TESTS_SUBTEST_TRACKER_H

#include <list>
#include <string>
#include "logicalaccess/msliblogicalaccess.h"

/**
 * Track the passed subtests to print an overview when the test program
 * is done.
 *
 * This is implemented as a singleton.
 */
class LIBLOGICALACCESS_API SubTestTracker
{
    std::list<std::string> tests_;

    /**
     * All possible subtests.
     */
    std::list<std::string> all_tests_;

public:
    static SubTestTracker &instance();

    /**
     * Flush to std::cout on destruction.
     */
    ~SubTestTracker();

    void add_passed(const std::string &subtest_name);

    void register_test(const std::string &subtest_name);

    friend std::ostream &operator<<(std::ostream &os, const SubTestTracker &stt);
};


#endif //LLA_TESTS_SUBTEST_TRACKER_H

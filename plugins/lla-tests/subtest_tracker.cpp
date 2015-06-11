//
// Created by xaqq on 6/3/15.
//

#include <iostream>
#include <algorithm>
#include "subtest_tracker.hpp"
#include "macros.hpp"
#include "utils.hpp"

void SubTestTracker::add_passed(const std::string &subtest_name)
{
    tests_.push_back(subtest_name);
    tests_.sort();
    PRINT_TIME("Subtest [" << subtest_name << "] marked as passed.");
}

std::ostream &operator<<(std::ostream &os, const SubTestTracker &stt)
{
    os << stt.tests_.size() << "/" << stt.all_tests_.size() << " subtests passed. OS: " << get_os_name();
    for (const auto& test : stt.tests_)
    {
        os << "\n\t" << test;
    }

    os << "\nSubtest that didn't run: ";

    std::list<std::string> not_run;
    std::set_difference(stt.all_tests_.begin(), stt.all_tests_.end(),
                        stt.tests_.begin(), stt.tests_.end(), std::inserter(not_run, not_run.begin()));
    for (const auto& test : not_run)
    {
        os << "\n\t" << test;
    }

    return os;
}

SubTestTracker &SubTestTracker::instance()
{
    static SubTestTracker instance_;

    return instance_;
}

SubTestTracker::~SubTestTracker()
{
    std::cout << *this << std::endl;
}

void SubTestTracker::register_test(const std::string &subtest_name)
{
    all_tests_.push_back(subtest_name);
    all_tests_.sort();
    PRINT_TIME("Subtest [" << subtest_name << "] registered");
}

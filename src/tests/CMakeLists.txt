cmake_minimum_required(VERSION 3.0)
project(lla_tests)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DUNIX")
#find_package(Boost 1.41.0 REQUIRED system)

enable_testing()

add_subdirectory(tools)
add_subdirectory(pcsc-tests)

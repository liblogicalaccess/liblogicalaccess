cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME lla-tests)
set(SOURCE_FILES utils.cpp myclock.cpp subtest_tracker.cpp)

add_library(${TARGET_NAME} ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/plugins)

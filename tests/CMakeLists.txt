cmake_minimum_required(VERSION 2.8.12)

## Register some include directories for tests.
## Variable number of arguments.
## Call this with no argument to reset the global additional include properties.
function(lla_test_register_include)
  set_property(GLOBAL PROPERTY LLA_TESTS_ADDITIONAL_INCLUDES_PROPERTY "")
  foreach(f ${ARGN})
    set_property(GLOBAL APPEND PROPERTY LLA_TESTS_ADDITIONAL_INCLUDES_PROPERTY ${f})
  endforeach()
endfunction()

## Create a test binary from the SOURCE_FILNAME parameter.
## This function expect a single source test (WITHOUT the .cpp extension)
## This code is duplicated in lla-private and lla-libnfc
##
## TEST_SUITE_NAME will generaly be the name of the card being tested. The value
## of this variable is prefixed to SOURCE_FILENAME
function(lla_create_test TEST_SUITE_NAME SOURCE_FILENAME)
## module we link against
set(TARGET_NAME ${TEST_SUITE_NAME}_${SOURCE_FILENAME})
add_executable(${TARGET_NAME} ${SOURCE_FILENAME}.cpp)
#set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-W -Wall -g -std=c++11")
target_link_libraries(${TARGET_NAME}
 lla-tests
 ${LIBLOGICALACCESS_LIBRARY}
 logicalaccess-cryptolib
 ${Boost_LIBRARIES}
)
get_property(LLA_TESTS_ADDITIONAL_INCLUDES GLOBAL PROPERTY LLA_TESTS_ADDITIONAL_INCLUDES_PROPERTY)

target_include_directories(${TARGET_NAME} PRIVATE
${CMAKE_SOURCE_DIR}/plugins
${LLA_TESTS_ADDITIONAL_INCLUDES}
)
endfunction()

add_subdirectory(pcsc-tests)
add_subdirectory(other)

if (LLA_BUILD_UNITTESTS)
    add_subdirectory(unittest)
endif()

cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME test_mifare_ultralightc)
set(SOURCE_FILES main.cpp)
add_executable(${TARGET_NAME} ${SOURCE_FILES})

target_include_directories(${TARGET_NAME} PUBLIC /usr/include/PCSC/)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/plugins)

target_link_libraries(${TARGET_NAME}
 lla-tests
 ${LIBLOGICALACCESS_LIBRARY}
 pcscreaders
 mifareultralightcards
 logicalaccess-cryptolib
 ${Boost_LIBRARIES}
 )

add_test(NAME ${TARGET_NAME}
        COMMAND $<TARGET_FILE:${TARGET_NAME}>)

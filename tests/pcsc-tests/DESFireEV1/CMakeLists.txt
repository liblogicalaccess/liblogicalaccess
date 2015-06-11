cmake_minimum_required(VERSION 2.8.12)
project(test_desfire_ev1)

set(TARGET_NAME test_desfire_ev1)
set(SOURCE_FILES main.cpp)
add_executable(${TARGET_NAME} ${SOURCE_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
${CMAKE_SOURCE_DIR}/plugins/pluginscards/desfire/
${CMAKE_SOURCE_DIR}/plugins/pluginscards/samav2/
${CMAKE_SOURCE_DIR}/plugins/pluginscards/iso7816/
${CMAKE_SOURCE_DIR}/plugins/readers/iso7816/commands
${CMAKE_SOURCE_DIR}/plugins
)

target_link_libraries(${TARGET_NAME}
 lla-tests
 ${LIBLOGICALACCESS_LIBRARY}
 pcscreaders
 desfirecards
 iso7816readers
 logicalaccess-cryptolib
 ${Boost_LIBRARIES}
 )

add_test(NAME ${TARGET_NAME}
        COMMAND $<TARGET_FILE:${TARGET_NAME}>)

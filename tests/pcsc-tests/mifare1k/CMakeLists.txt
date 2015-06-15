cmake_minimum_required(VERSION 2.8.12)

set(TARGET_NAME test_mifare_1k)
set(SOURCE_FILES main.cpp)
add_executable(${TARGET_NAME} ${SOURCE_FILES})

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/plugins)

target_link_libraries(${TARGET_NAME}
 lla-tests
 ${LIBLOGICALACCESS_LIBRARY}
 pcscreaders
 mifarecards
 logicalaccess-cryptolib
 ${Boost_LIBRARIES}
 )

add_test(NAME ${TARGET_NAME}
        COMMAND $<TARGET_FILE:${TARGET_NAME}>)

## Access Storage
set(TARGET_NAME test_mifare_access_storage)
set(SOURCE_FILES test_storage_service.cpp)
add_executable(${TARGET_NAME} ${SOURCE_FILES})

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/plugins)

target_link_libraries(${TARGET_NAME}
 lla-tests
 ${LIBLOGICALACCESS_LIBRARY}
 pcscreaders
 mifarecards
 logicalaccess-cryptolib
 ${Boost_LIBRARIES}
 )

add_test(NAME ${TARGET_NAME}
        COMMAND $<TARGET_FILE:${TARGET_NAME}>)

## Iteration
set(TARGET_NAME test_mifare_iteration)
set(SOURCE_FILES test_iterate_memory.cpp)
add_executable(${TARGET_NAME} ${SOURCE_FILES})

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/plugins)

target_link_libraries(${TARGET_NAME}
 lla-tests
 ${LIBLOGICALACCESS_LIBRARY}
 pcscreaders
 mifarecards
 logicalaccess-cryptolib
 ${Boost_LIBRARIES}
 )

add_test(NAME ${TARGET_NAME}
        COMMAND $<TARGET_FILE:${TARGET_NAME}>)
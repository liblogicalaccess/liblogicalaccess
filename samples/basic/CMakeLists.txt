cmake_minimum_required(VERSION 2.8)

project(basic CXX)

find_package(LibLogicalAccess NO_MODULE REQUIRED)
include(${LIBLOGICALACCESS_USE_FILE})

add_definitions(${LIBLOGICALACCESS_USE_FILE})

SET(SOURCE basic.cpp)

add_executable(
    basic
    ${SOURCE}
)

target_link_libraries(
        basic
        ${LIBLOGICALACCESS_LIBRARIES}
)

install ( TARGETS basic
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib${LIB_SUFFIX}
          ARCHIVE DESTINATION lib${LIB_SUFFIX}
        )
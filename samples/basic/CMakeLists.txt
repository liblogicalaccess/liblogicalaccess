cmake_minimum_required(VERSION 2.8)

project(basic CXX)

find_package(LibLogicalAccess NO_MODULE REQUIRED)
include(${LIBLOGICALACCESS_USE_FILE})

include_directories(/usr/local/include)

SET(SOURCE basic.cpp)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -Wall -Wno-unused-parameter")
endif()


add_executable(
    basic
    ${SOURCE}
)

target_link_libraries(
    basic
    pcscreaders
    logicalaccess
    ${LIBLOGICALACCESS_LIBRARIES}
)

install ( TARGETS basic
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib${LIB_SUFFIX}
          ARCHIVE DESTINATION lib${LIB_SUFFIX}
        )
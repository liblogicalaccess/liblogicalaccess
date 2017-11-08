cmake_minimum_required(VERSION 3.1)

project(basic CXX)

find_package(LibLogicalAccess NO_MODULE REQUIRED)
include(${LIBLOGICALACCESS_USE_FILE})

SET(SOURCE basic.cpp)

if (UNIX)
	include_directories(/usr/local/include)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -Wall -Wno-unused-parameter")
endif()


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
cmake_minimum_required(VERSION 2.6)
project(logicalaccess-cryptolib)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-DDEBUG -D_DEBUG)
endif()

if(UNIX)
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
  add_definitions(-DUNIX -DLOGICALACCESS_LOGS)
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  add_definitions(-DWIN32 -D_WIN32 -DLOGICALACCESS_LOGS -D_USRDLL -DS_WNT -D_CRT_SECURE_NO_WARNINGS -DLIBLOGICALACCESS_EXPORTS)
endif()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS include/*.h)
file(GLOB_RECURSE HDPPRS include/*.hpp)

add_library(
	logicalaccess-cryptolib
	STATIC

	${SRCS}
	${HDRS}
        ${HDPPRS}
  )
cmake_minimum_required(VERSION 2.6)

project(basictest)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

include_directories("../../include")

if(UNIX)
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
  add_definitions(-DUNIX)
endif()

SET(SOURCE basic.cpp)

add_executable(
    basic
    ${SOURCE}
)

if(UNIX)
  link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../packages/lib" "${CMAKE_CURRENT_SOURCE_DIR}/../../native/Plugins/Cryptolib")
else()
  link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../bin/${VC_FOLDER_NAME}")
endif()

remove_definitions(-DLIBLOGICALACCESS_EXPORTS -DLIBLOGICALACCESS_EXPORTS_DLL)

set(LIB_OPT)
if(UNIX)

set(LIB_ENDNAME)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(LIB_ENDNAME ".debug")
endif()

  set(LIB_OPT
    ssl
    boost_filesystem
    boost_date_time
    logicalaccess-cryptolib
    )
endif()

target_link_libraries(
        basic
        liblogicalaccess
	${LIB_OPT}
)

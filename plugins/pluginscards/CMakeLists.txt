cmake_minimum_required(VERSION 2.6)

project(pluginscards-liblogicalaccess)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG -D_DEBUG)
	set(VC_FOLDER_NAME "Debug")
else()
	set(VC_FOLDER_NAME "Release")
endif()

if(UNIX)
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -g")
  add_definitions(-DUNIX -DLOGICALACCESS_LOGS)
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  
  if (TARGET_ARCH STREQUAL "Win32")
    add_definitions(-DWIN32 -D_WIN32)
  else()
	add_definitions(-DWIN64 -D_WIN64)
  endif()
  add_definitions(-DLOGICALACCESS_LOGS -D_USRDLL -DS_WNT -D_CRT_SECURE_NO_WARNINGS -DLIBLOGICALACCESS_EXPORTS)
endif()

if(UNIX)
	SET(LIBRARY_OUTPUT_PATH ./bin)
else()
	SET(LIBRARY_OUTPUT_PATH ./bin/${TARGET_ARCH})
endif()

if (UNIX)
  INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../../packages/include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../../include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../cryptolib/include")
  LINK_DIRECTORIES(${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/../../packages/lib"
					"${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
					"${CMAKE_CURRENT_SOURCE_DIR}/../cryptolib/bin")
else()
  INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../../packages/include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../../include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../cryptolib/include")
  LINK_DIRECTORIES(${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/../../packages/lib/${TARGET_ARCH}"
					"${CMAKE_CURRENT_SOURCE_DIR}/../cryptolib/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}"
					"${CMAKE_CURRENT_SOURCE_DIR}/../../bin/${TARGET_ARCH}/${VC_FOLDER_NAME}")
endif()

add_subdirectory(twic)
add_subdirectory(desfire)
add_subdirectory(em4102)
add_subdirectory(em4135)
add_subdirectory(generictag)
add_subdirectory(icode1)
add_subdirectory(icode2)
add_subdirectory(indala)
add_subdirectory(infineonmyd)
add_subdirectory(iso7816)
add_subdirectory(iso15693)
add_subdirectory(legicprime)
add_subdirectory(mifare)
add_subdirectory(mifareplus)
add_subdirectory(mifareultralight)
add_subdirectory(prox)
add_subdirectory(proxlite)
add_subdirectory(smartframe)
add_subdirectory(stmlri)
add_subdirectory(tagit)
